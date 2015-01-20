#ifndef __QUEUE_H__
#define	__QUEUE_H__

#include <queue>
#include "mutex.h"
#include "condition.h"

template <class T>
class Queue
{
public:
    typedef T value_type;
    typedef typename std::deque<T>::size_type size_type;
    typedef typename std::deque<T>::const_reference const_reference;

private:
    Mutex _mutex;
    std::deque<value_type> _queue;

public:
    size_type size();
    bool empty();
    value_type pop();
    void push(const_reference item);
};

template<class T>
class WaitQueue
{
public:
    typedef T value_type;
    typedef typename std::deque<T>::size_type size_type;
    typedef typename std::deque<T>::const_reference const_reference;

private:
    Mutex _mutex;
    Condition _cond;
    std::deque<value_type> _queue;

public:
    size_type size();
    bool empty();
    value_type pop();
    void push(const_reference item);
};

/* -------------------------------------------------------------------------- */

template <class T>
inline typename Queue<T>::size_type Queue<T>::size()
{
    MutexLock lock(_mutex);
    return _queue.size();
}

template <class T>
inline bool Queue<T>::empty()
{
    MutexLock lock(_mutex);
    return _queue.empty();
}

template <class T>
inline typename Queue<T>::value_type Queue<T>::pop()
{
    MutexLock lock(_mutex);
    if (_queue.empty()) {
        throw std::exception();
    }
    value_type item(_queue.front());
    _queue.pop_front();
    return item;
}

template <class T>
inline void Queue<T>::push(typename Queue<T>::const_reference item)
{
    MutexLock lock(_mutex);
    _queue.push_back(item);
}

template <class T>
inline typename WaitQueue<T>::size_type WaitQueue<T>::size()
{
    MutexLock lock(_mutex);
    return _queue.size();
}

template <class T>
inline bool WaitQueue<T>::empty()
{
    MutexLock lock(_mutex);
    return _queue.empty();
}

template <class T>
inline typename WaitQueue<T>::value_type WaitQueue<T>::pop()
{
    MutexLock lock(_mutex);
    while (_queue.empty()) {
        _cond.wait(_mutex);
    }

    value_type item = _queue.front();
    _queue.pop_front();

    return item;
}

template <class T>
inline void WaitQueue<T>::push(typename WaitQueue<T>::const_reference item)
{
    MutexLock lock(_mutex);
    _queue.push_back(item);
    _cond.signal();
}

#endif	/* __QUEUE_H__ */
