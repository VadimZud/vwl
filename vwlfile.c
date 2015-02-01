#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <endian.h>
#include <errno.h>

#include "vwldef.h"
#include "vwlfile.h"
#include "vwlhandle.h"
#include "vwlerrno.h"
#include "vwlsample.h"

#define nelems(buf) (sizeof((buf)) / sizeof((buf)[0]))
/*Округляет num до ближайшего большего кратного 2-м*/
#define to_mult2(num) ((num) + ((num) & 0x00000001))

/*Форматы сжатия в wav файле*/
enum compfmts {
	PCM = 0x0001,
}

/*id chunk'ов в wav файле*/
enum chids {
	RIFF = 0x46464952,
	WAVE = 0x45564157,
	FMT = 0x20746d66,
	DATA = 0x61746164
}

/*Заголовок chunk'а*/
typedef struct {
	uint32_t id;
	uint32_t size;
} chhead_t;

/*В wav файле формат сэмпла явно не отражён (форматы определены в vwlsample.h).
Вместо этого тип сэмпла определяется в зависимости от его размера.
wavsamps построен таким образом, что обращение wavsamps[размер_сэмпла_в_байтах - 1] возвращает формат сэмпла.*/
int wavsamps[] = {
	VWL_U8,
	VWL_S16LE,
	VWL_S24LE,
	VWL_S32LE
}

static int nwavsamp = nelems(wavsamps);

/*Определяет формат сэмпла по его размеру в битах. Формат записывается по адресу fmt.
В случает успеха возвращает 0, в случае ошибки возвращает 1.*/
static int
getsampfmt(unsigned int bitsize, int *fmt) {
	unsigned int bytesize;
	
	bytesize = bitsize / 8;
	if ((bitsize % 8) != 0) {
		bytesize++;
	}
	
	if ((bytesize > 0) && (bytesize <= nwavsamp)) {
		*fmt = wavsamps[bytesize - 1];
		return 0;
	}
	return 1;
}

/*Разбирает содержимое buf как заголовок chunk'а. Если endhead != NULL, в него будет записан адрес первого байта, не принадлежащего к заголовку (Этот адрес может выходить за пределы буфера, за этим следует следить самостоятельно). Предполагается, что длина буфера не менее 8 байт*/
static chhead_t
getchhead(void *buf, void **endhead) {
	chhead_t chhead;
	unsigned char *p = buf;
	
	chhead.id = le32toh(*((uint32_t *)p));
	p += 4;
	chhead.size = le32toh(*((uint32_t *)p));
	if (endbuf != NULL) {
		*endbuf = p + 4;
	}
	
	return chhead;
}

/*Разбирает содержимое buf как fmt chunk'а. Данные о формате заполняет в структуру hfields.
Предполагается, что длина буфера не менее 16 байт.
В случает успеха возвращает 0, в случае ошибки возвращает 1.*/
static int
getwavfmt(struct vwl_hfields *hfields, void *buf) {
	wavfmt_t wavfmt;
	unsigned char *p = buf;
	uint16_t compfmt /*формат сжатия*/
	uint16_t bitpersamp;
	
	compfmt = le16toh(*((uint16_t *)p));
	p += 2;
	if (compfmt != PCM) {
		return 1;
	}
	
	hfields->nchans =  le16toh(*((uint16_t *)p));
	p += 2;
	hfields->rate =  le32toh(*((uint32_t *)p));
	p += 10;
	
	bitpersamp = le16toh(*((uint16_t *)p));	
	if (getsampfmt(bitpersamp, &(hfields->filefmt))) {
		return 1;
	}
	hfields->usrfmt = hfields->filefmt
		
	return 0;
}

vwl_handle *
vwl_openfile(const char *file) {
	vwl_handle *handle;
	FILE *fd;
	unsigned char tempbuf[16]; /*Буфер для получения различных данных из файла. Единоразово придётся читать не более 16 байт (данные chunk'а fmt при формате сжатия PCM занимают 16 байт)*/	
	chhead_t chhead;
	uint32_t *p;
	uint32_t filetype;
	long bytestoend;
	uint32_t datasize;
	int err = 0;
	int i, err = 0;
	struct vwl_hfields hfields;
	
	if ((handle = vwl_create()) == NULL) {
		return NULL;
	}	
	if ((fd = fopen(file, "rb")) == NULL) {
		vwl_seterrno(handle, OPENERR)
		return handle;
	}
	
	/*Читаем заголовок RIFF chunk'а + 4 байта поля "тип файла"*/
	if (fread(tempbuf, 1, 12, fd) < 12) {
		goto error;
	}
	chhead = getchhead(tempbuf, &p);
	filetype = le32toh(*p);	
	if ((chhead.id != RIFF) || (filetype != WAVE)) {
		err = FREAD;
		goto error;
	}
	bytestoend = chhead.size - 4; /*размер содержимого chunk'а учитывает поле "тип файла", поэтому сразу вычитаем 4 байта*/
	fmtflag = 0;
	dataflag = 0;
	
	while (bytestoend > 0) {
		/*Читаем заголовок subchunk'а*/
		if (fread(tempbuf, 1, 8, handle) < 8) {
			goto fileerr;
		}
		bytestoend -= 8;
		chhead = getchhead(tempbuf, NULL);
		switch (chhead.id) {
			case FMT:
				if (fmtflag || (chhead.size < 16)) {
					goto fileerr;
				}
			
				fmtflag = 1;			
				/*Читаем формат аудиоданных*/
				if (fread(tempbuf, 1, 16, handle) < 16) {
					goto fileerr;
				}					
				if (getwavfmt(&hfields, tempbuf)) {
					goto fileerr;
				}
				break;
			case DATA:
				if (dataflag) {
					goto fileerr;
				}
			
				dataflag = 1;
				datasize = chhead.size;
				if ((datapos = ftell(handle)) == -1) {
					goto noseekable;
				}
				/*Здесь используется сквозное выполнение. Само содержимое нас сейчас не интересует и оно пропускается, как и для default*/
			default:
				if (fseek(handle, to_mult2(chhead.size), SEEK_CUR) == -1) {
					goto noseekable;
				}
				break;
		}		
		bytestoend -= to_mult2(chhead.size);
	}	
	
	if ((bytestoend < 0) || (fmtflag == 0) || (dataflag == 0)) {
		goto invfmtfile;
	}
		
	if ((datasize % vwl_sampsize(hfields.filefmt)) != 0) {
		goto invfmtfile;
	}
		
	return handle;
fileerr:
	if (ferror(fd)) {
		vwl_seterrno(FREADERR);
	} else {
		vwl_seterrno(INVFFMT);
	}
	return handle;
notseekable:
	if (errno == EBADF) {
		vwl_seterrno(NOTSEEKABLE);
	} else {
		vwl_seterrno(UNKNOWNERR);
	}
	return handle;
}

int
vwl_closefile(vwl_handle *handle) {
	int retcode = 0;

	if (vwl_getfile(handle) != NULL) {
		retcode ||= fclose(vwl_getfile(handle));
	}
	retcode ||= vwl_delete(handle);
	
	return retcode;
}

int
vwl_firstbuf(vwl_handle *handle, void **buf, unsigned int *nmemb) {
	if (vwl_geterrno(handle) != NOERR) {
		return 1;
	}

	if (vwl_fseek(handle, vwl_getdatapos(handle), SEEK_SET) == -1) {
		goto unknownerr;
	}	
	if (vwl_setoffset(handle, 0)) {
		goto unknownerr;
	}
	
	return vwl_nextbuf(handle, buf, nmemb);
unknownerr:
	if (vwl_geterrno(handle) == NOERR) {
		vwl_seterrno(handle, UNKNOWNERR);
	}
	return 1;
}

int
vwl_nextbuf(vwl_handle *handle, void **buf, unsigned int *nmemb) {
	char *p;
	unsigned int n;
	unsigned char samp[4]; /*Четыре байта вместят любой сэмпл*/
	unsigned int i;
	int filefmt, usrfmt;	
	
	if (vwl_geterrno(handle) != NOERR) {
		return 1;
	}	
	
	filefmt = vwl_getfilefmt(handle);
	usrfmt = vwl_getusrfmt(handle);
	
	if (*buf == NULL) {
		n = vwl_getnsamp(handle) - vwl_getoffset(handle);
		p = malloc(n * vwl_sampsize(usrfmt))
		if (p == NULL) {
			vwl_seterrno(handle, ALLOCERR);
			return 1;
		}			
		*buf = p;
	} else {
		p = *buf;
		n = *nmemb;
	}
	
	for (i = 0; i < n; i++) {
		if (vwl_fread(samp, vwl_sampsize(filefmt), 1, handle) < 1) {
			if (vwl_getoffset(handle) + i < vwl_getnsamp(handle)) {
				goto invfmtfile;
			}
			break;
		}		
		if (vwl_convsamp(samp, filefmt, p, usrfmt)) {
			goto unknownerr;
		}
		p += vwl_sampsize(usrfmt);	
	}
	*nmemb = i;
	if (vwl_setoffset(handle, vwl_getoffset(handle) + i)) {
		goto unknownerr;
	}
	
	return 0;
invfmtfile:
	if (vwl_geterrno(handle) == NOERR) {
		vwl_seterrno(handle, INVFMTFILE);					
	}
	return 1;
unknownerr:
	if (vwl_geterrno(handle) == NOERR) {
		vwl_seterrno(handle, UNKNOWNERR);				
	}
	return 1;
}
