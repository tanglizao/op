#ifndef __TASK_H__
#define	__TASK_H__

#include "callback.h"

class FunTask
{
private:
    fun_cb _cb;
    void * _arg;

public:
    FunTask(fun_cb cb = NULL, void * arg = NULL);

public:
    void operator()();
    bool empty();
};

class RunTask
{
private:
    run_cb _cb;
    void * _arg;

public:
    RunTask(run_cb cb = NULL, void * arg = NULL);

public:
    void operator()(Runnable * r);
    bool empty();
};

/* -------------------------------------------------------------------------- */

inline FunTask::FunTask(fun_cb cb, void * arg) : _cb(cb), _arg(arg)
{
}

inline void FunTask::operator()()
{
    if (NULL != _cb) {
        _cb(_arg);
    }
}

inline bool FunTask::empty()
{
    return ((NULL == _cb) && (NULL == _arg));
}

inline RunTask::RunTask(run_cb cb, void * arg) : _cb(cb), _arg(arg)
{
}

inline void RunTask::operator()(Runnable * r)
{
    if (NULL != _cb) {
        _cb(r, _arg);
    }
}

inline bool RunTask::empty()
{
    return ((NULL == _cb) && (NULL == _arg));
}

#endif	/* __TASK_H__ */
