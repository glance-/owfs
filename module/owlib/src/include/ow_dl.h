/*
$Id$
    OW -- One-Wire filesystem
    version 0.4 7/2/2003

    All the error and statistics counters are declared here,
   including handling macros
*/ 

#ifndef OW_DL_H
#define OW_DL_H

#include <config.h>
#include "owfs_config.h"

#ifdef HAVE_DLOPEN
#include <dlfcn.h>
typedef void * DLHANDLE;
#elif OW_CYGWIN
#include <windows.h>
typedef HMODULE DLHANDLE;
#endif

extern DLHANDLE libdnssd;

DLHANDLE DL_open(const char *pathname, int mode);
void *DL_sym(DLHANDLE handle, const char *name);
int DL_close(DLHANDLE handle);
char *DL_error(void);

#endif
