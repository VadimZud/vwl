/*Содержит общие определения для библиотеки vwl*/

#ifndef _VWLDEF_H
#define _VWLDEF_H 1

/*Форматы сэмплов*/
enum vwl_sampfmt {
	VWL_U8,		/*Беззнаковый 8 бит*/
	VWL_S8,		/*Знаковый 8 бит*/	
	VWL_U16LE,	/*Беззнаковый 16 бит little-endian*/
	VWL_S16LE,	/*Знаковый 16 бит little-endian*/
	VWL_U16BE,	/*Беззнаковый 16 бит big-endian*/
	VWL_S16BE,	/*Знаковый 16 бит big-endian*/
	VWL_U24LE,	/*Беззнаковый 24 бита little-endian*/
	VWL_S24LE,	/*Знаковый 24 бита little-endian*/
	VWL_U24BE,	/*Беззнаковый 24 бита big-endian*/
	VWL_S24BE,	/*Знаковый 24 бита big-endian*/
	VWL_U32LE,	/*Беззнаковый 32 бита little-endian*/
	VWL_S32LE,	/*Знаковый 32 бита little-endian*/
	VWL_U32BE,	/*Беззнаковый 32 бита big-endian*/
	VWL_S32BE	/*Знаковый 32 бита big-endian*/
};

/*Коды ошибок*/
enum vwl_errcode {
	/*0 - отсутствие ошибок*/
	OPENFERR = 1,	/*Ошибка открытия файла*/
	INVFFMT,		/*Неверный формат файла*/
	FREADERR,		/*Ошибка чтения из файла*/
	ALLOCERR,		/*Ошибка при выделении памяти*/
	NOTSEEKABLE,	/*В потоке, связанном с обработчиком, нельзя менять позицию ввода-вывода*/
	UNKNOWNERR/		/*Неизвестная ошибка*/
};
#endif
