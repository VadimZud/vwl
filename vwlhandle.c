#include <stdio.h>
#include <stdlib.h>

#include "vwlhandle.h"
#include "vwlerrno.h"

/*Внутренней представление объекта-ручки для библиотеки vwl*/
typedef struct {
	FILE *file;				/*Файл, связанный с обработчиком*/	
	unsigned int nchans;	/*Число каналов*/
	unsigned int rate;		/*Частота дискретизации*/
	int filefmt;			/*Фактический формат сэмплов в файле*/
	int usrfmt;				/*Выходной формат сэмплов, в котором они предоставляются вызывающему модулю*/
	unsigned int datapos;	/*Положение начала аудиоданных в файле*/
	unsigned int nsamp;		/*Размер аудиоданных в сэмплах*/
	unsigned int offset;	/*Смещение относительно начала аудиоданных в сэмплах*/
	int errno;				/*Код состояния ошибки*/
} _vwl_handle;

_vwl_handle *
vwl_hcreate() {
	_vwl_handle *handle;
	int err = 0;
	
	handle = malloc(sizeof(_vwl_handle));
	if (handle == NULL) {
		return NULL;
	}
	
	err ||= vwl_setfile(handle, NULL);
	err ||= vwl_seterrno(handle, 0);
	if (err) {
		free(handle);
		return NULL;
	}
	return handle;
}

int 
vwl_hdelete(_vwl_handle *handle) {
	free(handle);	
	return 0;
}

struct vwl_hfields
vwl_getfields(_vwl_handle *handle) {
	struct vwl_hfields fields;
	
	fields.file = vwl_getfile(handle);
	fields.nchans = vwl_getnchans(handle);
	fields.rate = vwl_getrate(handle);
	fields.filefmt = vwl_getfilefmt(handle);
	fields.usrfmt = vwl_getusrfmt(handle);
	fields.datapos = vwl_getdatapos(handle);
	fields.nsamp = vwl_getnsamp(handle);
	fields.offset = vwl_getoffset(handle);
	fields.errno = vwl_geterrno(handle);
	
	return fields;
}

int 
vwl_setfields(vwl_handle *handle, struct vwl_hfields fields) {
	int err = 0;
	
	err ||= vwl_setfile(handle, fields.file);
	err ||= vwl_setnchans(handle, fields.nchans);
	err ||= vwl_setrate(handle, fields.rate);
	err ||= vwl_setfilefmt(handle, fields.filefmt);
	err ||= vwl_setusrfmt(handle, fields.usrfmt);
	err ||= vwl_setdatapos(handle, fields.datapos);
	err ||= vwl_setnsamp(handle, fields.nsamp);
	err ||= vwl_setoffset(handle, fields.offset);
	err ||= vwl_seterrno(handle, fields.errno);
	
	return err;
}

FILE *
vwl_getfile(_vwl_handle *handle) {
	return handle->file;
}

int
vwl_setfile(_vwl_handle *handle, FILE *file) {
	if (vwl_geterrno(handle)) {
		return 1;
	}
	handle->file = file;
	return 0;
}

unsigned int 
vwl_getnchans(_vwl_handle *handle) {
	return handle->nchans;
}

int
vwl_setnchans(_vwl_handle *handle, unsigned int nchans) {
	if (vwl_geterrno(handle)) {
		return 1;
	}
	handle->nchans = nchans;
	return 0;
}

unsigned int 
vwl_getrate(_vwl_handle *handle) {
	return handle->rate;
}

int
vwl_setrate(_vwl_handle *handle, unsigned int rate) {
	if (vwl_geterrno(handle)) {
		return 1;
	}
	handle->rate = rate;
	return 0;
}

int
vwl_getfilefmt(_vwl_handle *handle) {
	return handle->filefmt;
}

int
vwl_setfilefmt(_vwl_handle *handle, int fmt) {
	if (!vwl_issamp(fmt) || vwl_geterrno(handle)) {
		return 1;
	}
	handle->filefmt = fmt;
	return 0
}

int
vwl_getusrfmt(_vwl_handle *handle) {
	return handle->usrfmt;
}

int
vwl_setusrfmt(_vwl_handle *handle, int fmt) {
	if (!vwl_issamp(fmt) || vwl_geterrno(handle)) {
		return 1;
	}
	handle->usrfmt = fmt;
	return 0
}

unsigned int 
vwl_getdatapos(_vwl_handle *handle) {
	return handle->datapos;
}

int
vwl_setdatapos(_vwl_handle *handle, unsigned int datapos) {
	if (vwl_geterrno(handle)) {
		return 1;
	}
	handle->datapos = datapos;
	return 0
}

unsigned int 
vwl_getnsamp(_vwl_handle *handle) {
	return handle->nsamp;
}

int
vwl_setnsamp(_vwl_handle *handle, unsigned int nsamp) {
	if (vwl_geterrno(handle)) {
		return 1;
	}
	handle->nsamp = nsamp;
	return 0
}

unsigned int 
vwl_getoffset(_vwl_handle *handle) {
	return handle->offset;
}

int
vwl_setoffset(_vwl_handle *handle, unsigned int offset) {
	if (vwl_geterrno(handle)) {
		return 1;
	}
	handle->offset = offset;
	return 0
}

int
vwl_geterrno(_vwl_handle *handle) {
	return handle->errno;
}

int
vwl_seterrno(_vwl_handle *handle, unsigned int code) {
	int retcode = 0;

	if (!vwl_iserrcode(code)) {
		return 1;
	}
	handle->errno = code;	
	return 0;
}
