#ifndef __EVENTTHREADPOOL_H__
#define	__EVENTTHREADPOOL_H__

#include "eventthread.h"

class EventThreadPool : private NonCopyable
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
    typedef std::vector<EventThread *> ThreadList;
    ThreadList _threads;

public:
    explicit EventThreadPool(uint thread_num
                             , const RunTask & enter_task = RunTask()
                             , const RunTask & exit_task = RunTask()
                             , bool start = true, uint buffer = 1024);
    ~EventThreadPool();

public:
    void start();
    void stop();
    bool schedule(run_cb cb, void * arg);
    bool schedule(const RunTask & task);
    bool running();
    bool stopped();
    size_t backlog();
};

/* -------------------------------------------------------------------------- */

inline EventThreadPool::EventThreadPool(uint thread_num
                                        , const RunTask & enter_task
                                        , const RunTask & exit_task
                                        , bool start, uint buffer)
: _thread_num(thread_num ? thread_num : 1), _state(Stopped)
{
    while (_threads.size() < _thread_num) {
        _threads.push_back(new EventThread(enter_task, exit_task, buffer));
    }

    if (start) {
        this->start();
    }
}

inline EventThreadPool::~EventThreadPool()
{
    stop();

    for (ThreadList::iterator it = _threads.begin(); it != _threads.end(); ++it) {
        delete *it;
    }
}

inline void EventThreadPool::start()
{
    if (Running != _state) {
        _state = Running;
        for (ThreadList::iterator it = _threads.begin(); it != _threads.end(); ++it) {
            (*it)->start();
        }
    }
}

inline void EventThreadPool::stop()
{
    if (_state != Running) {
        return;
    }

    _state = Stopping;

    for (ThreadList::iterator it = _threads.begin(); it != _threads.end(); ++it) {
        (*it)->stop();
    }

    _state = Stopped;
}

inline bool EventThreadPool::schedule(run_cb cb, void * arg)
{
    return schedule(RunTask(cb, arg));
}

inline bool EventThreadPool::schedule(const RunTask & task)
{
    if (Running == _state) {
        EventThread * min_thr = NULL;

        for (ThreadList::iterator it = _threads.begin(); it != _threads.end(); ++it) {
            int thr_backlog = (*it)->backlog();
            int min_backlog = 0;
            if (NULL != min_thr) {
                min_backlog = min_thr->backlog();
            }

            if(NULL == min_thr) {
                min_thr = (*it);
            }
            else if (0 == thr_backlog) {
                min_thr = (*it);
            }
            else if(thr_backlog < min_backlog) {
                min_thr = (*it);
            }

            if(0 == min_thr->backlog()) {
                break;
            }
        }
        
        return min_thr->defer(task);
    }

    return false;
}

inline bool EventThreadPool::running()
{
    return Running == _state;
}

inline bool EventThreadPool::stopped()
{
    return Stopped == _state;
}

inline size_t EventThreadPool::backlog()
{
    size_t backlog = 0;
    for (ThreadList::iterator it = _threads.begin(); it != _threads.end(); ++it) {
        backlog += (*it)->backlog();
    }
    return backlog;
}

#endif	/* __EVENTTHREADPOOL_H__ */
