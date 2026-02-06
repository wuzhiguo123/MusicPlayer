#include "server.hpp"
#include <arpa/inet.h>
#include <bits/types/struct_timeval.h>
#include <cstddef>
#include <cstring>
#include <dirent.h>
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <list>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <sys/types.h>
#include <jsoncpp/json/json.h>
#include "debug.hpp"
#include "player.hpp"

struct event* time_event;
struct timeval* t;
Server::Server()
{
    //保存服务器的网络地址信息
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(Port);
    server_addr.sin_family = AF_INET;

    //初始化LIBEVENT
    m_base = event_base_new();

    //初始化

    //初始化数据库
    m_database = new DataBase;
    if(!m_database->DatabaseInitTable())
    {
        std::cout << "数据库初始化失败" << std::endl;
        exit(-1);
    }

    //初始化链表
    m_player = new Player;

}

Server::~Server()
{
    if(m_database)
        free(m_database);
    if(time_event)
        free(time_event);
    if(t)
        free(t);
}

struct event_base* Server::GetBase()
{
    return m_base;
}

void Server::TimerEvent()
{
    time_event = (struct event*)malloc(sizeof(struct event));
    t = (struct timeval*)malloc(sizeof(struct timeval));
    if(event_assign(time_event, m_base, -1, EV_PERSIST, TimeoutCb, m_player)==-1)
    {
        std::cout << "evet_assign" <<std::endl;
        return;
    }

    evutil_timerclear(t);
    t->tv_sec = 1;
    event_add(time_event, t);
}
void Server::TimeoutCb(evutil_socket_t fd, short s, void * arg)
{
    Player* p = (Player*)arg;
    p->TraverseList();
}

void Server::Listen(const char* ip, int16_t port)
{
    //建立网络通信事件
    struct evconnlistener* listener = evconnlistener_new_bind(m_base,
                                                             ListenCb, //只能传入静态成员函数，因为普通成员函数会隐式的传入this，但libevent是c库，会导致函数签名不一致
                                                             this, 
                                                             LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,//断连自动释放|复用Ip
                                                             3, 
                                                             (struct sockaddr*)&server_addr, 
                                                             sizeof(server_addr));
    if(listener == nullptr)
    {
        std::cout << "evconnlistener_new_bind()" <<std::endl;
        return;
    }                                                             
    //建立定时器事件                                                             
    TimerEvent();

    //开启监听（循环）
    event_base_dispatch(m_base);
    //释放对象
    evconnlistener_free(listener);
    event_base_free(m_base);
}

void Server::ListenCb(struct evconnlistener * l, int fd, struct sockaddr * client_info, int socklen, void * arg)
{
    Server* s = (Server*) arg;
    struct sockaddr_in* client_addr = (struct sockaddr_in*)client_info;
    char* client_ip = inet_ntoa(client_addr->sin_addr);
    int16_t client_port = client_addr->sin_port;
    std::cout << "[CLIENT INFO]IP:" << client_ip << "PORT:" << client_port <<std::endl;

    struct bufferevent* bev = bufferevent_socket_new(s->GetBase(),fd,BEV_OPT_CLOSE_ON_FREE);
    if(bev == nullptr)
    {
        std::cout << "bufferevent_socket_new error" <<std::endl;
        return;
    }
    bufferevent_setcb(bev, ReadCb, nullptr, EventCb, s);
    bufferevent_enable(bev, EV_READ);
}

void Server::ReadCb(struct bufferevent *bev, void *ctx)
{
    Server* s = (Server*)ctx;
    char buffer[1024] = {0};
    s->ServerReadData(bev,buffer);
    Json::Reader reader;
    Json::Value value;
    if(!reader.parse(buffer,value))
    {
        std::cout << "收到的字符串不是一个Json格式" << std::endl;
    }
    if(value["cmd"] == "get_music_list")
    {
        value = s->m_player->GetMusicName(bev,value["singer"].asString());
        s->SendData(bev,value);
    }

    else if (value["cmd"] == "info") {
        s->m_player->DevUpdateList(bev,value,s);
    }

    else if(value["cmd"] == "app_info")
    {
        s->m_player->AppUpdateList(bev,value,s);
    }

    else if(value["cmd"] == "upload_music")
    {
        std::cout << "#################" <<s->m_player->online_list->begin()->a_bev << std::endl;
        s->m_player->UploadMusic(bev,value,s);
    }

    else if(value["cmd"] == "app_start" || value["cmd"] == "app_stop" || value["cmd"] == "app_suspend" || value["cmd"] == "app_continue" || value["cmd"] == "app_prev"
            || value["cmd"] == "app_next" || value["cmd"] == "app_upvolume" || value["cmd"] == "app_downvolume" || value["cmd"] == "app_circle" || value["cmd"] == "app_sequence")
    {
        s->m_player->TransAppCmd(bev,value,s);
    }
    else if(value["cmd"] == "app_start_reply" || value["cmd"] == "app_stop_reply" || value["cmd"] == "app_suspend_reply" || value["cmd"] == "app_continue_reply" || value["cmd"] == "app_prev_reply"
            || value["cmd"] == "app_next_reply" || value["cmd"] == "app_upvolume_reply" || value["cmd"] == "app_downvolume_reply" || value["cmd"] == "app_circle_reply" || value["cmd"] == "app_sequence_reply")
    {
        s->m_player->TransDevReply(bev,value,s);
    }

    else if (value["cmd"] == "app_register")
    {
        s->m_database->RegisterUsr(bev,value,s);
    }
    else if (value["cmd"] == "app_login") 
    {
        s->Login(bev,value);
    }
    else if(value["cmd"] == "app_bind")
    {
        s->BindUsr(bev,value);
    }
    else if(value["cmd"] == "app_offline")
    {
        s->m_player->AppOffline(bev);
    }
    else if(value["cmd"] == "app_capture_cur_musiclist")
    {
        debug("app_capture_cur_musiclist");
        s->m_player->CaptureCurMusicList(bev,value,s);
    }


    // std::cout << value << std::endl;
}
void Server::EventCb(struct bufferevent *bev, short what, void *ctx)
{
    Server* s = (Server*)ctx;
    if(what & BEV_EVENT_EOF)//异常关闭事件
    {
        s->m_player->AppOrDevOffline(bev);
    }

}

void Server::ServerReadData(struct bufferevent *bev,char* buffer)
{
    ssize_t r = 0;
    ssize_t head_len = sizeof(int);
    while(true)
    {
        r += bufferevent_read(bev, buffer+r, sizeof(int)-r);
        if(r <= 0)
        {
            std::cout << "对端关闭连接" << std::endl;
        }
        else if(r == head_len)
            break;
    }

    int buffer_len = *(int*)buffer;
    memset(buffer, 0, sizeof(int));
    r = 0;
    while(true)
    {
        r += bufferevent_read(bev, buffer+r, buffer_len-r);
        if(r <= 0)
        {
            std::cout << "对端关闭连接" << std::endl;
        }
        else if(r == buffer_len)
            break;
    }

    std::cout << "[LEN:]" << head_len << "MSG:" << buffer << std::endl;
}




void Server::SendData(struct bufferevent *bev,Json::Value& val)
{
    char msg[1024] = {0};
    std::string send_str = Json::FastWriter().write(val);
    std::cout << "[SERVER SEND]" << send_str <<std::endl;
    int len = send_str.size();
    memcpy(msg, &len, sizeof(int));
    memcpy(msg+sizeof(int), send_str.c_str(), len);

    if(bufferevent_write(bev, msg, len+sizeof(int)) == -1)
    {
        std::cout << "bufferevent_write() failure" <<std::endl;
    }

}

void Server::Login(struct bufferevent *bev,Json::Value value)
{
    Json::Value reply_value;
    reply_value["cmd"] = "app_login_reply";
    m_database->DatabaseConnect();
    std::string deviceid;
    do{
        if(!m_database->CheckUsrExist(value))//检查用户是否存在
        {
            reply_value["result"] = "not_exist";
            break;
        }
        else 
        {
            if(!m_database->CheckPassword(value))//检查密码是否正确
            {
                reply_value["result"] = "password_error";
                break;
            }
            //登陆成功
            else 
            {
                if(!m_database->CheckBind(value,deviceid))//检查是否和音箱绑定
                {
                    reply_value["result"] = "not_bind";
                }

                else
                {
                    reply_value["result"] = "bind";
                    reply_value["deviceid"] = deviceid;
                }
            }
        }

    }while(0);
    m_database->DatabaseDisconnect();
    SendData(bev, reply_value);
}


void Server::BindUsr(struct bufferevent *bev,Json::Value value)
{
    Json::Value reply_value;
    reply_value["cmd"] = "app_bind_reply";
    m_database->DatabaseConnect();

    if(m_database->AppBindDev(value))
        reply_value["result"] = "success";
    else
        reply_value["result"] = "failure";
    SendData(bev, reply_value);

    m_database->DatabaseDisconnect();
}

