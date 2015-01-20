#ifndef __EVENTTHREAD_H__
#define	__EVENTTHREAD_H__

#ifdef DEBUG
#include <iostream>
#endif

#include <event.h>
#include "thread.h"

class EventThread : public Thread
{
private:
    event _notify;
    event_base * _evbase;
    int _notify_recv_fd;
    int _notify_send_fd;
    size_t _backlog;
    Mutex _mutex;

public:
    EventThread(const RunTask & enter_task = RunTask()
                , const RunTask & exit_task = RunTask()
                , uint buffer = 1024);
    virtual ~EventThread();

public:
    event_base * evbase();
    size_t backlog();
    bool defer(run_cb cb, void * arg);
    bool defer(const RunTask & task);

public:
    static void notify_cb(evutil_socket_t fd, short which, void * arg);

public:
    virtual void run();
    virtual void stop();
};

/* -------------------------------------------------------------------------- */

inline EventThread::EventThread(const RunTask & enter_task
                                , const RunTask & exit_task, uint buffer)
: Thread(NULL, enter_task, exit_task), _backlog(0)
{
    int fds[2] = {0};
    int ret = evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    assert(0 == ret);
    evutil_make_socket_nonblocking(fds[0]);
    evutil_make_socket_nonblocking(fds[1]);
    _notify_recv_fd = fds[0];
    _notify_send_fd = fds[1];

    uint rnum = buffer * sizeof (RunTask);
    ret = setsockopt(_notify_send_fd, SOL_SOCKET, SO_RCVBUF, &rnum, sizeof (uint));
    assert(0 == ret);

    _evbase = event_base_new();
    assert(NULL != _evbase);
    ret = event_assign(&_notify, _evbase, _notify_recv_fd, EV_READ | EV_PERSIST, notify_cb, this);
    assert(0 == ret);
    ret = event_add(&_notify, 0);
    assert(0 == ret);
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

inline EventThread::~EventThread()
{
    if (_evbase) {
        event_base_free(_evbase);
    }
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

inline event_base * EventThread::evbase()
{
    return _evbase;
}

inline size_t EventThread::backlog()
{
    MLOCK lock(_mutex);
    return _backlog;
}

inline bool EventThread::defer(run_cb cb, void * arg)
{
    return defer(RunTask(cb, arg));
}

bool EventThread::defer(const RunTask & task)
{
    MLOCK lock(_mutex);
    ssize_t ret = send(_notify_send_fd, &task, sizeof (task), 0);
    if (ret <= 0) {
        return false;
    }

    ++_backlog;
    return true;
}

void EventThread::notify_cb(evutil_socket_t fd, short /*which*/, void * arg)
{
    EventThread * evthr = (EventThread *) arg;
    assert(evthr);

    RunTask task;
    ssize_t ret = recv(fd, &task, sizeof (task), 0);
    if (ret <= 0) {
        if (EAGAIN != errno) {
            return evthr->stop();
        }
    }
    else {
        if (task.empty()) {
            event_base_loopbreak(evthr->_evbase);
#ifdef DEBUG
            std::cout << __PRETTY_FUNCTION__ << "evthread loop break" << std::endl;
#endif
        }
        else {
            task(evthr);
        }

        MLOCK lock(evthr->_mutex);
        --evthr->_backlog;
    }
}

inline void EventThread::run()
{
    assert(NULL != _evbase);
    int ret = event_base_dispatch(_evbase);
    assert(0 == ret);
}

inline void EventThread::stop()
{
    bool ret = defer(NULL, NULL);
    assert(ret);
    join();
}

#endif	/* __EVENTTHREAD_H__ */
