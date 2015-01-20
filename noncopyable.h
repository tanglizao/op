#ifndef __NONCOPYABLE_H__
#define	__NONCOPYABLE_H__

#ifdef DEBUG
#include <iostream>
#endif

class NonCopyable
{
private:
    NonCopyable(const NonCopyable &);
    NonCopyable & operator=(const NonCopyable &);

protected:
    NonCopyable();
    ~NonCopyable();
};

/* -------------------------------------------------------------------------- */

inline NonCopyable::NonCopyable()
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

inline NonCopyable::~NonCopyable()
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

#endif	/* __NONCOPYABLE_H__ */
