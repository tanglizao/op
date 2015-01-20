#ifndef __THREADPOOL_H__
#define	__THREADPOOL_H__

#include "thread.h"
#include "queue.h"
#include "task.h"

class ThreadPool : private NonCopyable
{
private:

    enum State
    {
        Stopped,
        Running,
        Stopping
    };

private:
    uint _thread_num;
    State _state;
    typedef std::vector<Thread *> ThreadList;
    ThreadList _threads;
    WaitQueue<RunTask *> _tasks;

public:
    explicit ThreadPool(uint thread_num
                        , const RunTask & enter_task = RunTask()
                        , const RunTask & exit_task = RunTask()
                        , bool start = true);
    ~ThreadPool();

private:
    RunTask * task();

public:
    void start();
    void stop();
    bool schedule(run_cb cb, void * arg);
    bool schedule(const RunTask & task);
    bool running();
    bool stopped();
    size_t backlog();

private:
    static void routine(Runnable * r, void * arg);
};

/* -------------------------------------------------------------------------- */

inline ThreadPool::ThreadPool(uint thread_num
                              , const RunTask & enter_task
                              , const RunTask & exit_task
                              , bool start)
: _thread_num(thread_num ? thread_num : 1), _state(Stopped)
{
    while (_threads.size() < _thread_num) {
        _threads.push_back(new Thread(RunTask(routine, this), enter_task, exit_task));
    }

    if (start) {
        this->start();
    }
}

inline ThreadPool::~ThreadPool()
{
    stop();

    for (ThreadList::iterator it = _threads.begin(); it != _threads.end(); ++it) {
        delete *it;
    }

    assert(_tasks.empty());
    while (!_tasks.empty()) {
        delete _tasks.pop();
    }
}

inline RunTask * ThreadPool::task()
{
    return _tasks.pop();
}

inline void ThreadPool::start()
{
    if (Running != _state) {
        _state = Running;
        for (ThreadList::iterator it = _threads.begin(); it != _threads.end(); ++it) {
            (*it)->start();
        }
    }
}

inline void ThreadPool::stop()
{
    if (_state != Running) {
        return;
    }

    _state = Stopping;

    for (ThreadList::iterator it = _threads.begin(); it != _threads.end(); ++it) {
        _tasks.push(NULL);
    }

    for (ThreadList::iterator it = _threads.begin(); it != _threads.end(); ++it) {
        (*it)->join();
    }

    _state = Stopped;
}

inline bool ThreadPool::schedule(run_cb cb, void * arg)
{
    return schedule(RunTask(cb, arg));
}

inline bool ThreadPool::schedule(const RunTask & task)
{
    if (Running == _state) {
        _tasks.push(new RunTask(task));
        return true;
    }

    return false;
}

inline bool ThreadPool::running()
{
    return Running == _state;
}

inline bool ThreadPool::stopped()
{
    return Stopped == _state;
}

inline size_t ThreadPool::backlog()
{
    return _tasks.size();
}

void ThreadPool::routine(Runnable * r, void * arg)
{
    ThreadPool * thrpool = (ThreadPool *) arg;
    assert(thrpool);

    RunTask * task = NULL;
    while (NULL != (task = thrpool->task())) {
        (*task)(r);
        delete task;
    }
}

#endif	/* __THREADPOOL_H__ */
