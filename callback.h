#ifndef __CALLBACK_H__
#define	__CALLBACK_H__

#include "runnable.h"

typedef void (*fun_cb)(void * arg);
typedef void (*run_cb)(Runnable * r, void * arg);

#endif	/* __CALLBACK_H__ */
