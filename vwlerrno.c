#include <stdio.h>
#include <stdlib.h>

#include "vwldef.h"
#include "vwlhandle.h"
#include "vwlerrno.h"

#define nelems(buf) (sizeof((buf)) / sizeof((buf)[0]))

static char *errmess[] = {
	"Success",					/*0*/
	"Open file error",			/*1 = OPENFERR*/
	"Invalid file format",		/*2 = INVFFMT*/
	"Read file error",			/*3 = FREADERR*/
	"Memory allocation error",	/*4 = ALLOCERR*/
	"Stream is not seekable",	/*5 = NOSEEKABLE*/
	"Unknown error"				/*6 = UNKNOWNERR*/
};

/*Число определённых ошибок, т.е. число элементов в vwl_errmess*/
static int nerr = nelems(errmess);

int
vwl_iserrcode(int code) {
	return ((code < nerr) && (code >= 0));
}

char *
vwl_strerror(vwl_handle *handle) {
	int errcode;
	
	if (handle == NULL) {
		errcode = vwl_getgloberrno();
	} else {
		errcode = vwl_hgeterrno(handle);
	}
	
	if (vwl_iserrcode(errcode) {
		return errmess[errcode];
	}
	return "Undefined error";
}

void
vwl_perror(vwl_handle *handle, const char *s) {
	if (s != NULL) {
		fprintf(stderr, "%s: ", s);
	}
	fprintf(stderr, "%s\n", vwl_strerror(handle));
}
