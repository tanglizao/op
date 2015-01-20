#ifndef __RUNNABLE_H__
#define	__RUNNABLE_H__

#include "noncopyable.h"

class Runnable : private NonCopyable
{
public:
    virtual void run() = 0;
};

#endif	/* __RUNNABLE_H__ */

