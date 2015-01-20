#ifndef __SINGLETON_H__
#define	__SINGLETON_H__

#ifdef DEBUG
#include <iostream>
#endif

#include "noncopyable.h"

template <class T>
class Singleton : private NonCopyable
{
protected:
    Singleton();
    ~Singleton();

public:
    static T & instance();
};

/* -------------------------------------------------------------------------- */

template <class T>
inline Singleton<T>::Singleton()
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

template <class T>
inline Singleton<T>::~Singleton()
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

template <class T>
T & Singleton<T>::instance()
{
    static T _instance;
    return _instance;
}

#endif	/* __SINGLETON_H__ */
