#ifndef __CONDITION_H__
#define	__CONDITION_H__

#include <sys/time.h>
#include "noncopyable.h"
#include "mutex.h"

class Condition : private NonCopyable
{
private:
    pthread_cond_t _handle;

public:
    Condition();
    ~Condition();

public:
    void wait(Mutex & m);
    bool wait(Mutex &m, uint ms);
    void signal();
    void broadcast();
};

/* -------------------------------------------------------------------------- */

inline Condition::Condition()
{
    int ret = pthread_cond_init(&_handle, NULL);
    assert(0 == ret);
}

inline Condition::~Condition()
{
    int ret = pthread_cond_destroy(&_handle);
    assert(0 == ret);
}

inline void Condition::wait(Mutex& m)
{
    int ret = pthread_cond_wait(&_handle, m.handle());
    assert(0 == ret);
}

inline bool Condition::wait(Mutex& m, uint ms)
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);

    timespec ts = {0};
    ts.tv_nsec = ((ms%1000) * 1000 + tv.tv_usec) * 1000;
    ts.tv_sec = (ms/1000) + (ts.tv_nsec/1000000000) + tv.tv_sec;
    ts.tv_nsec = ts.tv_nsec % 1000000000;

    int ret = pthread_cond_timedwait(&_handle, m.handle(), &ts);
    assert(0 == ret);
    return (0 == ret);
}

inline void Condition::signal()
{
    int ret = pthread_cond_signal(&_handle);
    assert(0 == ret);
}

inline void Condition::broadcast()
{
    int ret = pthread_cond_broadcast(&_handle);
    assert(0 == ret);
}

#endif	/* __CONDITION_H__ */

