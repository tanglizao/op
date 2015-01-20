#ifndef __ACCEPTOR_H__
#define	__ACCEPTOR_H__

#include <string>
#include <event.h>
#include <event2/listener.h>
#include "noncopyable.h"
#include "eventhandler.h"
#include "eventthreadpool.h"
#include "connection.h"

class Acceptor : private NonCopyable
{
private:
    std::string _ip;
    ushort _port;
    EventThread * _reactor;
    EventHandler * _handler;
    evconnlistener * _listener;
    int _backlog;
    EventThreadPool * _workers;

public:
    Acceptor(const char * ip, ushort port
             , EventThread * reactor, EventHandler * handler
             , int backlog = 1024);
    ~Acceptor();

public:
    EventThread * reactor();
    EventHandler * handler();
    bool start(uint worker_num = 0);
    void stop();

private:
    static void accept_cb(Runnable * r, void * arg);
    static void accept_cb(evconnlistener * lsnr, evutil_socket_t sock
                          , sockaddr * sa, int sa_len, void * arg);
};

class AcceptParam
{
public:
    evutil_socket_t _sock;
    EventHandler * _handler;

public:
    AcceptParam(evutil_socket_t sock, EventHandler * handler);
};

AcceptParam::AcceptParam(int sock, EventHandler* handler)
: _sock(sock), _handler(handler)
{
}

/* -------------------------------------------------------------------------- */

Acceptor::Acceptor(const char * ip, ushort port
                   , EventThread * reactor, EventHandler * handler
                   , int backlog)
: _ip((NULL == ip) ? std::string() : std::string(ip)), _port(port)
, _reactor(reactor), _handler(handler)
, _listener(NULL), _backlog(backlog), _workers(NULL)
{
}

Acceptor::~Acceptor()
{
    stop();
}

inline EventThread * Acceptor::reactor()
{
    return _reactor;
}

inline EventHandler * Acceptor::handler()
{
    return _handler;
}

bool Acceptor::start(uint worker_num)
{
    if (worker_num) {
        _workers = new EventThreadPool(worker_num);
    }

    in_addr sa = {0};
    if (!_ip.empty()) {
        evutil_inet_pton(AF_INET, _ip.c_str(), &sa);
    }
    sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_addr = sa;
    sin.sin_port = htons(_port);

    _listener = evconnlistener_new_bind(_reactor->evbase(), accept_cb, this
                                        , LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE
                                        , _backlog, (struct sockaddr *) &sin, sizeof (sin));
    return (NULL != _listener);
}

void Acceptor::stop()
{
    if (NULL != _listener) {
        evconnlistener_free(_listener);
        _listener = NULL;
    }

    if (NULL != _workers) {
        delete _workers;
        _workers = NULL;
    }
}

void Acceptor::accept_cb(evconnlistener * lsnr, evutil_socket_t sock
                         , sockaddr * sa, int sa_len, void * arg)
{
    if(-1 == sock) {
#ifdef DEBUG
        fprintf(stderr, "err:accept %d\n", errno);
#endif
        return;
    }

    Acceptor * acceptor = (Acceptor *) arg;
    if (NULL != acceptor->_workers) {
        acceptor->_workers->schedule(accept_cb, new AcceptParam(sock, acceptor->handler()));
    }
    else {
        Connection * conn = new Connection(acceptor->reactor(), acceptor->handler());
        if (!conn->on_accepted(sock)) {
            delete conn;
        }
    }
}

void Acceptor::accept_cb(Runnable * r, void * arg)
{
    assert(NULL != r);
    assert(NULL != arg);

    AcceptParam * ap = (AcceptParam *)arg;

    Connection * conn = new Connection((EventThread *)r, ap->_handler);
    if (!conn->on_accepted(ap->_sock)) {
        delete conn;
    }

    delete ap;
}

#endif	/* __ACCEPTOR_H__ */
