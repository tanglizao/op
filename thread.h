#ifndef __THREAD_H__
#define	__THREAD_H__

#ifdef DEBUG
#include <iostream>
#endif

#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include "task.h"

class Thread : public Runnable
{
public:

    enum State
    {
        Ready = 0,
        Running = 1,
        Finished = 2
    };

protected:
    RunTask _routine_task;
    RunTask _enter_task;
    RunTask _exit_task;
    bool _detached;
    pthread_t _tid;
    State _state;

public:
    explicit Thread(const RunTask & routine_task = RunTask()
                    , const RunTask & enter_task = RunTask()
                    , const RunTask & exit_task = RunTask()
                    , bool detach = false);
    virtual ~Thread();

public:
    void start();
    void join();
    void detach();
    pthread_t tid();
    State state();
    void state(State state);

private:
    void on_enter();
    void on_exit();

private:
    static void * routine(void * arg);

public:
    static void yield();

public:
    virtual void run();
    virtual void stop();
};

/* -------------------------------------------------------------------------- */

inline Thread::Thread(const RunTask & routine_task
                      , const RunTask & enter_task
                      , const RunTask & exit_task
                      , bool detach)
: _routine_task(routine_task), _enter_task(enter_task), _exit_task(exit_task)
, _detached(detach), _tid(0), _state(Ready)
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

inline Thread::~Thread()
{
    join();
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

inline void Thread::start()
{
    if (Running != state()) {
        _state = Running;
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        if (_detached) {
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        }

        int ret = pthread_create(&_tid, &attr, routine, this);
        pthread_attr_destroy(&attr);
        assert(0 == ret);
    }
}

inline void Thread::join()
{
    if ((Running == state()) && (!_detached)) {
        int ret = pthread_join(_tid, NULL);
        assert(0 == ret);
        _state = Finished;
    }
}

inline void Thread::detach()
{
    if (!_detached) {
        if (0 != _tid) {
            int ret = pthread_detach(_tid);
            _detached = true;
            assert(0 == ret);
        }
    }
}

inline void Thread::stop()
{
    if (Running == state()) {
        int ret = pthread_kill(_tid, SIGKILL);
        _state = Finished;
        assert(0 == ret);
    }
}

inline pthread_t Thread::tid()
{
    return _tid;
}

inline Thread::State Thread::state()
{
    return _state;
}

inline void Thread::state(Thread::State state)
{
    _state = state;
}

inline void Thread::on_enter()
{
    _enter_task(this);
}

inline void Thread::on_exit()
{
    _exit_task(this);
}

void * Thread::routine(void * arg)
{
    Thread * thr = (Thread *) arg;
#ifdef DEBUG
    std::cout << "Thread " << thr->_tid << " running!" << std::endl;
#endif

    thr->on_enter();
    thr->run();
    thr->on_exit();
    thr->state(Finished);

#ifdef DEBUG
    std::cout << "Thread " << thr->_tid << " exit!" << std::endl;
#endif
    pthread_detach(pthread_self());
    return NULL;
}

inline void Thread::yield()
{
    int ret = pthread_yield();
    assert(0 == ret);
}

inline void Thread::run()
{
    _routine_task(this);
}

#endif	/* __THREAD_H__ */

