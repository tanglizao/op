#include <iostream>
#include "justone.h"
#include "queue.h"
#include "thread.h"
#include "eventthread.h"
#include "threadpool.h"
#include "eventthreadpool.h"
#include "objmanager.h"
#include "semaphore.h"
#include "log.h"
#include "connection.h"
#include "acceptor.h"
#include "echohandler.h"

using namespace std;
#define dlog(l, args...)	if( One<Log>::instance().level() >= l ) { One<Log>::instance().out(l, args); }
#define dlog1(args...)		dlog(1, args)
#define dlog2(args...)		dlog(2, args)
#define dlog3(args...)		dlog(3, args)
#define dlog4(args...)		dlog(4, args)
#define dlog5(args...)		dlog(5, args)

class test : public EventHandler
{
private:
    size_t _seq;

public:

    void print()
    {
        cout << __PRETTY_FUNCTION__ << " print test " << this << endl;
    }

    void sequence(size_t seq)
    {
        _seq = seq;
    }

    size_t sequence()
    {
        return _seq;
    }
public:

    test()
    {
        cout << __PRETTY_FUNCTION__ << " " << this << endl;
    }

    ~test()
    {
        cout << __PRETTY_FUNCTION__ << " " << this << endl;
    }

public:

    static void run(Runnable * r, void * arg)
    {
        cout << __PRETTY_FUNCTION__ << " " << r << " " << arg << endl;
    }

public:

    virtual void on_accepted(Connection * conn)
    {
        conn->timeout(10, 10);
        dlog1("client %p accepted.\n", conn);
    }

    virtual void on_read(Connection * conn)
    {
        char buffer[1024] = {0};
        size_t read = conn->read(buffer, sizeof (buffer));
        // evbuffer *in = conn->get_input();
        dlog1("Received %zu bytes\n", read);
        dlog1("----- data ----\n");
        dlog1("%.*s\n", read, buffer);

        conn->write(buffer, read);

        evbuffer *out = conn->output();
        dlog1("writing %zu bytes\n", evbuffer_get_length(out));
    }

    virtual void on_write(Connection * conn)
    {
        evbuffer * out = conn->output();
        dlog1("output buffer %zu bytes\n", evbuffer_get_length(out));
    }

    virtual void on_closed(Connection * conn)
    {
        dlog1("client %p closed.\n", conn);
        conn->disconnect();
    }

    virtual void on_event(short what, Connection * conn)
    {
        dlog1("conn %p on event %d\n", conn, what);
        if (what & BEV_EVENT_TIMEOUT) {
            string str = "timeout...\n";
            conn->write(str.c_str(), str.length());
            conn->enable(EV_READ);
            conn->disconnect();
        }
        else if(what & BEV_EVENT_ERROR) {
            conn->disconnect();
        }
    }
};

int main(int argc, char * argv[])
{
    One<test>::instance().print();
    Two<test>::instance().print();

    One<Log>::instance().create("./op_debug_");
    assert(((long *) &(One<test>::instance())) != ((long *) &(Two<test>::instance())));

    Thread thr(RunTask(test::run, &thr));
    thr.start();

    EventThread evthr(RunTask(test::run, &evthr));
    evthr.start();
    evthr.defer(test::run, &evthr);
    evthr.stop();

    ThreadPool thrpool(0, RunTask(test::run, (void *) 0), RunTask(test::run, (void *) 5));
    thrpool.start();
    thrpool.schedule(test::run, (void *) 1);
    thrpool.schedule(RunTask(test::run, (void *) 2));
    thrpool.stop();
    thrpool.start();
    thrpool.schedule(RunTask(test::run, (void *) 3));
    thrpool.schedule(test::run, (void *) 4);
    //    thrpool.stop();

    EventThreadPool evthrpool(2);
    //    evthrpool.start();
    evthrpool.schedule(test::run, (void *) 6);
    evthrpool.schedule(RunTask(test::run, (void *) 7));
    evthrpool.stop();
    evthrpool.start();
    evthrpool.schedule(test::run, (void *) 8);
    evthrpool.schedule(RunTask(test::run, (void *) 9));
    //    evthrpool.stop();

    ObjManager<test> tpool;
    test * pt = tpool.pop();
    pt->print();
    dlog1("test %p seq %d\n", pt, pt->sequence());
    tpool.push(pt);
    pt = tpool.pop();
    pt->print();
    cout << "test " << pt << " seq " << pt->sequence() << endl;
    tpool.push(pt);

    EchoHandler eh;
    Acceptor acceptor(NULL, 2345, &evthr, &eh);
    if (!acceptor.start()) {
        perror("accept on 2345....................................");
    }
    else {
        evthr.run();
    }
    return 0;
}
