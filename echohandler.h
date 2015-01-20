#ifndef __ECHOHANDLER_H__
#define	__ECHOHANDLER_H__

#include <assert.h>
#include <string>
#include "eventhandler.h"
#include "connection.h"

class EchoHandler : public EventHandler
{
public:
    virtual void on_accepted(Connection * conn);
    virtual void on_read(Connection * conn);
    virtual void on_write(Connection * conn);
    virtual void on_closed(Connection * conn);
    virtual void on_event(short what, Connection * conn);
};

void EchoHandler::on_accepted(Connection * conn)
{
    assert(NULL != conn);
#ifdef DEBUG
    printf("accept client %p\n", conn);
#endif
    std::string welcome = "Welcome to echo server, enter 'quit' to exit!\n";
    conn->write(welcome.c_str(), welcome.length());
}

void EchoHandler::on_read(Connection * conn)
{
    static char quit[] = "quit";

    assert(NULL != conn);
    char buffer[1024] = {0};
    size_t read = conn->read(buffer, sizeof (buffer));
#ifdef DEBUG
    printf("read %d from client %p\n", read, conn);
#endif
    if (0 < read) {
        if (0 == strncasecmp(quit, buffer, sizeof (quit) - 1)) {
            conn->disconnect();
        }
        else {
            conn->write(buffer, read);
        }
    }
}

void EchoHandler::on_write(Connection * conn)
{
    assert(NULL != conn);
#ifdef DEBUG
    printf("write to client %p end\n", conn);
#endif
}

void EchoHandler::on_closed(Connection * conn)
{
    assert(NULL != conn);
#ifdef DEBUG
    printf("close client %p\n", conn);
#endif
    delete conn;
}

void EchoHandler::on_event(short what, Connection * conn)
{
    assert(NULL != conn);
#ifdef DEBUG
    printf("event %d from client %p\n", what, conn);
#endif
    conn->disconnect();
}

#endif	/* __ECHOHANDLER_H__ */
