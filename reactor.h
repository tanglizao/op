#ifndef __REACTOR_H__
#define	__REACTOR_H__

#include <assert.h>
#include <event.h>
#include "noncopyable.h"

class Reactor : public NonCopyable
{
private:
    event_base * _evbase;

public:
    Reactor();
    ~Reactor();

public:
    void run();
    void stop();

public:
    event_base * evbase();
};

/* -------------------------------------------------------------------------- */

inline Reactor::Reactor()
{
    _evbase = event_base_new();
    assert(NULL != _evbase);
}

inline Reactor::~Reactor()
{
    stop();
    event_base_free(_evbase);
}

inline void Reactor::run()
{
    int ret = event_base_dispatch(_evbase);
    assert(0 == ret);
}

inline void Reactor::stop()
{
    event_base_loopbreak(_evbase);
}

inline event_base * Reactor::evbase()
{
    return _evbase;
}

#endif	/* __REACTOR_H__ */
