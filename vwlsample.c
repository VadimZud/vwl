#include <stdlib.h>
#include <string.h>

#include "vwlsample.h"

#define nelems(buf) (sizeof((buf)) / sizeof((buf)[0]))

/*Биты для поля flags структуры sampinfo*/
#define LE 0x01		/*Установлен, если формат имеет little-endian порядок байт*/
#define SIGNED 0x02	/*Установлен, если формат знаковый*/

static struct sampinfo_t{
	size_t size;	/*Число байт на сэмпл*/
	char flags;	
} sampinfo[] = {
	{1, 0x00},			/*VWL_U8*/
	{1, SIGNED},		/*VWL_S8*/
	{2, LE},			/*VWL_U16LE*/
	{2, SIGNED | LE},	/*VWL_S16LE*/
	{2, 0x00},			/*VWL_U16BE*/
	{2, SIGNED},		/*VWL_S16BE*/
	{3, LE},			/*VWL_U24LE*/
	{3, SIGNED | LE},	/*VWL_S24LE*/
	{3, 0x00},			/*VWL_U24BE*/
	{3, SIGNED},		/*VWL_S24BE*/
	{4, LE},			/*VWL_U32LE*/
	{4, SIGNED | LE},	/*VWL_S32LE*/
	{4, 0x00},			/*VWL_U32BE*/
	{4, SIGNED},		/*VWL_S32BE*/
};

/*Число определённых форматов сэмпла, т.е. число элементов в vwl_sampinfo*/
static int nsamp = nelems(sampinfo);

int
vwl_issamp(code) {
	return ((code < nsamp) && ((code >= 0));
}

size_t
vwl_sampsize(code) {
	if (!vwl_issamp(code)) {
		return 0;
	}
	return sampinfo[code].size;
}

int
vwl_isle(code) {
	if (!vwl_issamp(code)) {
		return 0;
	}
	return (sampinfo[code].flags & LE);
}

int
vwl_issigned(code) {
	if (!vwl_issamp(code)) {
		return 0;
	}
	return (sampinfo[code].flags & SIGNED);
}

/*Меняет порядок байт в буфере buf размера size*/
static void
invertbuf(void *buf, size_t size) {
	int i, j;
	char temp, *p = buf;
	
	for (i = 0, j = size - 1; i < j; i++, j--) {
		temp = p[i];
		p[i] = p[j];
		p[j] = temp;
	}
}

int
vwl_convsamp(void *srcsamp, int srcfmt, void *destsamp, int destfmt) {
	if (!vwl_issamp(srcfmt) || !vwl_issamp(destfmt)) {
		return 1;
	}
	
	size_t srcsize = vwl_sampsize(srcfmt);
	size_t destsize = vwl_sampsize(destfmt);
	char src[srcsize]; /*Мы будем использовать копию исходных данных, чтобы не портить данные вызывающего модуля и на случай, когда srcsamp и destsamp пересекаются*/
	char *dest = destsamp;
	int i;
	
	memcpy(src, srcsamp, srcsize);
	
	/*Дальнейший код работает с big-endian порядком, поэтому, если необходимо, меняем порядок байт*/
	if (vwl_isle(srcfmt)) {
		invertbuf(src, srcsize);
	}	
	/*Копируем, начиная со старших байтов. Это позволяет "масштабировать" значения сэмплов под новый диапазон, если исходный и целевой размеры сэмплов не совпадают*/
	for (i = 0; (i < srcsize) && (i < destsize); i++) {
		dest[i] = src[i];
	}
	for (i; i < destsize; i++) {
		dest[i] = 0x00;
	}
	if (vwl_issigned(srcfmt) != vwl_issigned(destfmt)) {
		dest[0] += 0x80; /*порядок big-endian, поэтому dest[0] - старший байт*/
	}
	if (vwl_isle(srcfmt)) {
		invertbuf(dest, destsize);
	}
	
	return 0;
}
