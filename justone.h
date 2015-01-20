#ifndef __JUSTONE_H__
#define	__JUSTONE_H__

#ifdef DEBUG
#include <iostream>
#endif

#include "singleton.h"

template <class T>
class One : public T, public Singleton<One<T> >
{
public:
    One();
    ~One();
};

template <class T>
class Two : public T, public Singleton<Two<T> >
{
public:
    Two();
    ~Two();
};

/* -------------------------------------------------------------------------- */

template <class T>
inline One<T>::One()
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

template <class T>
inline One<T>::~One()
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

template <class T>
inline Two<T>::Two()
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

template <class T>
inline Two<T>::~Two()
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

#endif	/* __JUSTONE_H__ */

