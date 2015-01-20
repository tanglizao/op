#ifndef __MUTEX_H__
#define	__MUTEX_H__

#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include "noncopyable.h"

class Mutex : private NonCopyable
{
private:
    pthread_mutex_t _handle;

public:
    Mutex(bool recursive = false);
    ~Mutex();

public:
    void lock();
    bool trylock();
    void unlock();

public:
    pthread_mutex_t * handle();
    const pthread_mutex_t * handle() const;
};

class MutexLock : private NonCopyable
{
private:
    Mutex & _mutex;

public:
    MutexLock(Mutex & m);
    ~MutexLock();
};

class ReadWriteMutex : private NonCopyable
{
private:
    pthread_rwlock_t _handle;

public:
    ReadWriteMutex();
    ~ReadWriteMutex();

public:
    void rdlock();
    bool tryrdlock();
    void wrlock();
    bool trywrlock();
    void unlock();

public:
    pthread_rwlock_t * handle();
    const pthread_rwlock_t * handle() const;
};

class ReadLock : private NonCopyable
{
private:
    ReadWriteMutex & _rw_mutex;

public:
    ReadLock(ReadWriteMutex & m);
    ~ReadLock();
};

class WriteLock : private NonCopyable
{
private:
    ReadWriteMutex & _rw_mutex;

public:
    WriteLock(ReadWriteMutex & m);
    ~WriteLock();
};

#ifndef MLOCK
#define MLOCK MutexLock
#endif

#ifndef RLOCK
#define RLOCK ReadLock
#endif

#ifndef WLOCK
#define WLOCK WriteLock
#endif

/* -------------------------------------------------------------------------- */

inline Mutex::Mutex(bool recursive)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_ERRORCHECK);

    int ret = pthread_mutex_init(&_handle, &attr);
    assert(0 == ret);
}

inline Mutex::~Mutex()
{
    pthread_mutex_destroy(&_handle);
}

inline void Mutex::lock()
{
    int ret = pthread_mutex_lock(&_handle);
    assert(0 == ret);
}

inline bool Mutex::trylock()
{
    int ret = pthread_mutex_trylock(&_handle);
    assert(0 == ret);
    return (0 == ret);
}

inline void Mutex::unlock()
{
    int ret = pthread_mutex_unlock(&_handle);
    assert(0 == ret);
}

inline pthread_mutex_t * Mutex::handle()
{
    return &_handle;
}

inline const pthread_mutex_t * Mutex::handle() const
{
    return &_handle;
}

inline MutexLock::MutexLock(Mutex & m) : _mutex(m)
{
    _mutex.lock();
}

inline MutexLock::~MutexLock()
{
    _mutex.unlock();
}

inline ReadWriteMutex::ReadWriteMutex()
{
    int ret = pthread_rwlock_init(&_handle, NULL);
    assert(0 == ret);
}

inline ReadWriteMutex::~ReadWriteMutex()
{
    int ret = pthread_rwlock_destroy(&_handle);
    assert(0 == ret);
}

inline void ReadWriteMutex::rdlock()
{
    int ret = pthread_rwlock_rdlock(&_handle);
    assert(0 == ret);
}

inline bool ReadWriteMutex::tryrdlock()
{
    int ret = pthread_rwlock_tryrdlock(&_handle);
    assert(0 == ret);
    return (0 == ret);
}

inline void ReadWriteMutex::wrlock()
{
    int ret = pthread_rwlock_wrlock(&_handle);
    assert(0 == ret);
}

inline bool ReadWriteMutex::trywrlock()
{
    int ret = pthread_rwlock_trywrlock(&_handle);
    assert(0 == ret);
    return (0 == ret);
}

inline void ReadWriteMutex::unlock()
{
    int ret = pthread_rwlock_unlock(&_handle);
    assert(0 == ret);
}

pthread_rwlock_t * ReadWriteMutex::handle()
{
    return &_handle;
}

const pthread_rwlock_t * ReadWriteMutex::handle() const
{
    return &_handle;
}

inline ReadLock::ReadLock(ReadWriteMutex & m) : _rw_mutex(m)
{
    _rw_mutex.rdlock();
}

inline ReadLock::~ReadLock()
{
    _rw_mutex.unlock();
}

inline WriteLock::WriteLock(ReadWriteMutex & m) : _rw_mutex(m)
{
    _rw_mutex.wrlock();
}

inline WriteLock::~WriteLock()
{
    _rw_mutex.unlock();
}

#endif	/* __MUTEX_H__ */
