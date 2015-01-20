#ifndef __EVENTHANDLER_H__
#define	__EVENTHANDLER_H__

class Connection;

class EventHandler
{
public:
    virtual void on_accepted(Connection * conn);
    virtual void on_connected(Connection * conn);
    virtual void on_read(Connection * conn);
    virtual void on_write(Connection * conn);
    virtual void on_closed(Connection * conn);
    virtual void on_event(short what, Connection * conn);
};

/* -------------------------------------------------------------------------- */

inline void EventHandler::on_event(short what, Connection * conn)
{
}

inline void EventHandler::on_accepted(Connection * conn)
{
}

inline void EventHandler::on_connected(Connection * conn)
{
}

inline void EventHandler::on_read(Connection * conn)
{
}

inline void EventHandler::on_write(Connection * conn)
{
}

inline void EventHandler::on_closed(Connection * conn)
{
}

#endif	/* __EVENTHANDLER_H__ */
