#ifndef __OBJMANAGER_H__
#define	__OBJMANAGER_H__

#include <queue>
#include "mutex.h"

template <class T>
class ObjManager
{
public:
    typedef T * value_type;
    typedef typename std::deque<value_type>::size_type size_type;
    typedef typename std::deque<value_type>::const_reference const_reference;

private:
    size_t _total;
    size_t _sequence;
    std::deque<value_type> _queue;
    Mutex _mutex;

public:
    ObjManager();
    ~ObjManager();

public:
    void push(const_reference obj);
    value_type pop();
    void adjust(size_t limit, size_t delta);
    size_t size();
    size_t total();
};

/* -------------------------------------------------------------------------- */

template <class T>
inline ObjManager<T>::ObjManager() : _total(0), _sequence(0)
{
}

template <class T>
inline ObjManager<T>::~ObjManager()
{
    MLOCK lock(_mutex);
    while (!_queue.empty()) {
        value_type obj(_queue.front());
        _queue.pop_front();
        delete obj;
    }
    _total = 0;
}

template <class T>
inline void ObjManager<T>::push(typename ObjManager<T>::const_reference obj)
{
    MLOCK lock(_mutex);
    _queue.push_back(obj);
}

template <class T>
inline typename ObjManager<T>::value_type ObjManager<T>::pop()
{
    value_type obj = NULL;

    MLOCK lock(_mutex);
    if (_queue.empty()) {
        obj = new T();
        ++_total;
    }
    else {
        obj = _queue.front();
        _queue.pop_front();
    }

    obj->sequence(_sequence++);
    return obj;
}

template <class T>
inline void ObjManager<T>::adjust(size_t limit, size_t delta)
{
    MLOCK lock(_mutex);
    while ((_queue.size() > limit) && (delta > 0)) {
        value_type obj(_queue.front());
        _queue.pop_front();
        delete obj;
        --_total;
        --delta;
    }
}

template <class T>
inline size_t ObjManager<T>::size()
{
    MLOCK lock(_mutex);
    return _queue.size();
}

template <class T>
inline size_t ObjManager<T>::total()
{
    MLOCK lock(_mutex);
    return _total;
}

#endif	/* __OBJMANAGER_H__ */
