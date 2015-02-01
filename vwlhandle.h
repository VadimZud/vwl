/*Интерфейс объекта-ручки для библиотеки vwl*/

#ifndef _VWLHANDLE_H
#define _VWLHANDLE_H 1
/*Все функции семейства get возвращают запрашиваемое значение.
Все функции семейства set возвращают 0 в случае успеха и 1 в случае ошибки.*/
/*Объект handle можно устанавливать в состояние ошибки. При работе с объектом, находящимся в состоянии ошибки, функции семейства set (кроме vwl_seterrno()) не изменяют соответствующих им значений и завершаются с ошибкой.*/

/*Объект-ручка*/
typedef _vwl_handle vwl_handle;

/*Структура используется как аргумент для vwl_setfields() и возвращается функцией vwl_getfields()*/
struct vwl_hfields{
	FILE *file;				/*Файл, связанный с обработчиком*/	
	unsigned int nchans;	/*Число каналов*/
	unsigned int rate;		/*Частота дискретизации*/
	int filefmt;			/*Фактический формат сэмплов в файле*/
	int usrfmt;				/*Выходной формат сэмплов, в котором они предоставляются вызывающему модулю*/
	unsigned int datapos;	/*Положение начала аудиоданных в файле*/
	unsigned int nsamp;		/*Размер аудиоданных в сэмплах*/
	unsigned int offset;	/*Смещение относительно начала аудиоданных в сэмплах*/
	int errno;				/*Код состояния ошибки*/
};

/*Возвращает объект-ручку.
В случае ошибки возвращает NULL*/
vwl_handle *vwl_create();

/*В случае успеха возвращает 0, в противном случае 1*/
int vwl_delete(vwl_handle *handle);

/*Все поля объекта, собранные в структуре vwl_hfields*/
struct vwl_hfields vwl_getfields(vwl_handle *handle);
int vwl_setfields(vwl_handle *handle, struct vwl_hfields fields);

/*Связаный с обработчиком файл*/
FILE *vwl_getfile(vwl_handle *handle);
int vwl_setfile(vwl_handle *handle, FILE *file);

/*Число каналов*/
unsigned int vwl_getnchans(vwl_handle *handle);
int vwl_setnchans(vwl_handle *handle, unsigned int nchans);

/*Частота дискретизации.*/
unsigned int vwl_getrate(vwl_handle *handle);
int vwl_setrate(vwl_handle *handle, unsigned int rate);

/*Фактический формат сэмплов в файле (определённые форматы см. в vwlsample.h).*/
int vwl_getfilefmt(vwl_handle *handle);
int vwl_setfilefmt(vwl_handle *handle, int fmt);

/*Выходной формат сэмплов, в котором они предоставляются вызывающему модулю.*/
int vwl_getusrfmt(vwl_handle *handle);
int vwl_setusrfmt(vwl_handle *handle, int fmt);

/*Положение начала аудиоданных в файле.*/
unsigned int vwl_getdatapos(vwl_handle *handle);
int vwl_setdatapos(vwl_handle *handle, unsigned int datapos);

/*Размер аудиоданных в сэмплах.*/
unsigned int vwl_getnsamp(vwl_handle *handle);
int vwl_setnsamp(vwl_handle *handle, unsigned int nsamp);

/*Смещение относительно начала аудиоданных в сэмплах.*/
unsigned int vwl_getoffset(vwl_handle *handle);
int vwl_setoffset(vwl_handle *handle, unsigned int offset);

/*Код состояния ошибки (определённые коды ошибок см. в vwlerrno.h).*/
int vwl_geterrno(vwl_handle *handle);
int vwl_seterrno(vwl_handle *handle, int code);

#endif
