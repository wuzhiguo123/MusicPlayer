#ifndef SERVER_HPP
#define SERVER_HPP

#include <event2/event.h>
#include <jsoncpp/json/value.h>
#include <stdint.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <list>
#include <string>
#include "database.hpp"
#include "player.hpp"
 
#define Ip "0.0.0.0"
#define Port 8008
#define SEQUENCE 1
#define CIRCLE 2
#define MUSIC_PATH "/var/www/html/music/"



class Server
{
public:
    Server();
    void Listen(const char* ,int16_t );
    static void ListenCb(struct evconnlistener *, int, struct sockaddr *, int, void *);
    struct event_base* GetBase();
    static void ReadCb(struct bufferevent *bev, void *ctx);
    static void EventCb(struct bufferevent *bev, short what, void *ctx);
    void  ServerReadData(struct bufferevent *,char*);
    void SendData(struct bufferevent *,Json::Value&);
    void TimerEvent();
    static void TimeoutCb(evutil_socket_t, short, void *);
    void Login(struct bufferevent *bev,Json::Value value);
    void BindUsr(struct bufferevent *bev,Json::Value value);


    ~Server();
private:
    struct sockaddr_in server_addr;
    struct event_base* m_base;
    DataBase* m_database;
    Player* m_player;
};
#endif