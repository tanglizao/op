#ifndef __CONNECTION_H__
#define	__CONNECTION_H__

#include <event.h>
#include "eventhandler.h"
#include "eventthread.h"

class Connection
{
private:

    enum State
    {
        Closed,
        Connecting,
        Connected,
        Closing,
    };
private:
    State _state;
    evutil_socket_t _sock;
    EventThread * _reactor;
    EventHandler * _handler;
    bufferevent * _bev;

public:
    Connection(EventThread * reactor, EventHandler * handler);
    ~Connection();

private:
    void on_connected();
    void close();

public:
    bool connect(sockaddr * sa, int sa_len);
    bool connected();

    size_t read(void * data, size_t len);
    bool write(const void * data, size_t len);

    void disconnect();
    evbuffer * input();
    evbuffer * output();
    void timeout(uint rt = 0, uint wt = 0);
    void enable(short event);

    bool on_accepted(evutil_socket_t sock);

private:
    static void read_cb(bufferevent * buffev, void * arg);
    static void write_cb(bufferevent * buffev, void * arg);
    static void event_cb(bufferevent * buffev, short what, void * arg);
};

/* -------------------------------------------------------------------------- */

inline Connection::Connection(EventThread * reactor, EventHandler * handler)
: _state(Closed), _sock(-1), _reactor(reactor), _handler(handler), _bev(NULL)
{
}

inline Connection::~Connection()
{
    close();
}

void Connection::on_connected()
{
    _sock = bufferevent_getfd(_bev);
    _state = Connected;
    assert(NULL != _bev);
    int ret = bufferevent_enable(_bev, EV_READ | EV_WRITE | EV_PERSIST);
    assert(0 == ret);
    _handler->on_connected(this);
}

bool Connection::connect(sockaddr * sa, int sa_len)
{
    _bev = bufferevent_socket_new(_reactor->evbase(), -1, BEV_OPT_CLOSE_ON_FREE);
    if (NULL == _bev) {
        return false;
    }
    bufferevent_setcb(_bev, read_cb, write_cb, event_cb, this);

    int ret = bufferevent_socket_connect(_bev, sa, sa_len);
    if (0 != ret) {
        bufferevent_free(_bev);
        _bev = NULL;
        return false;
    }

    _state = Connecting;
    return true;
}

inline bool Connection::connected()
{
    return (Connected == _state);
}

size_t Connection::read(void * data, size_t len)
{
    if (!connected()) {
        return 0;
    }

    assert(NULL != _bev);
    return bufferevent_read(_bev, data, len);
}

bool Connection::write(const void * data, size_t len)
{
    if (!connected()) {
        return false;
    }

    int ret = bufferevent_write(_bev, data, len);
    return (0 == ret);
}

void Connection::disconnect()
{
    if (-1 != _sock) {
        ::shutdown(_sock, SHUT_RD);
    }
    _state = Closing;
}

void Connection::close()
{
    if (-1 != _sock) {
        evutil_closesocket(_sock);
        _sock = -1;
    }

    if (NULL != _bev) {
        bufferevent_free(_bev);
        _bev = NULL;
    }

    _state = Closed;
}

inline evbuffer * Connection::input()
{
    return bufferevent_get_input(_bev);
}

inline evbuffer * Connection::output()
{
    return bufferevent_get_output(_bev);
}

void Connection::timeout(uint rt, uint wt)
{
    bufferevent_settimeout(_bev, rt, wt);
}

void Connection::enable(short event)
{
    short ev = bufferevent_get_enabled(_bev) ^ event & event;
    if (0 != ev) {
        int ret = bufferevent_enable(_bev, ev);
        assert(0 == ret);
    }
}

bool Connection::on_accepted(evutil_socket_t sock)
{
    _sock = sock;
    _state = Connected;
    _bev = bufferevent_socket_new(_reactor->evbase(), sock, BEV_OPT_CLOSE_ON_FREE);
    if (_bev == NULL) {
        return false;
    }

    bufferevent_enable(_bev, EV_READ | EV_WRITE | EV_PERSIST);
    bufferevent_setcb(_bev, read_cb, write_cb, event_cb, this);
    _handler->on_accepted(this);
    return true;
}

void Connection::read_cb(bufferevent * bev, void * arg)
{
    Connection * conn = (Connection *) (arg);
    conn->_handler->on_read(conn);
}

void Connection::write_cb(bufferevent * bev, void * arg)
{
    Connection * conn = (Connection *) (arg);
    conn->_handler->on_write(conn);
}

void Connection::event_cb(bufferevent * bev, short what, void * arg)
{
    Connection * conn = (Connection *) (arg);
    if (what & BEV_EVENT_CONNECTED) {
        conn->on_connected();
    }
    else if ((what & BEV_EVENT_READING) && (what & BEV_EVENT_EOF)) {
        conn->_handler->on_closed(conn);
    }
    else {
        conn->_handler->on_event(what, conn);
    }
}

#endif	/* __CONNECTION_H__ */
