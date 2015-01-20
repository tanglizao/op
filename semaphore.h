#ifndef __SEMAPHORE_H__
#define	__SEMAPHORE_H__

#include </usr/include/semaphore.h>
#include <assert.h>
#include <errno.h>
#include "noncopyable.h"

class Semaphore : private NonCopyable
{
private:
    sem_t _handle;

public:
    Semaphore(uint initial = 0);
    ~Semaphore();

public:
    void wait();
    void post();
};

/* -------------------------------------------------------------------------- */

inline Semaphore::Semaphore(uint initial)
{
    int ret = sem_init(&_handle, 0, initial);
    assert(0 == ret);
}

inline Semaphore::~Semaphore()
{
    int ret = sem_destroy(&_handle);
    assert(0 == ret);
}

inline void Semaphore::wait()
{
    while((-1 == sem_wait(&_handle)) && (EINTR == errno)) {
    }
}

inline void Semaphore::post()
{
    int ret = sem_post(&_handle);
    assert(0 == ret);
}

#endif	/* __SEMAPHORE_H__ */
