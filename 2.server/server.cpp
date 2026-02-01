#include "server.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <event2/event.h>
#include <event2/listener.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
Server::Server()
{
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr = inet_addr(Ip);
    server_addr.sin_port = htons(Port);
    server_addr.sin_family = AF_INET;

    m_base = event_base_new();
}

void Server::Listen(const char* ip, int16_t port)
{
    struct evconnlistener* listener = evconnlistener_new_bind(m_base,
                                                             ListenCb, //只能传入静态成员函数，因为普通成员函数会隐式的传入this，但libevent是c库，会导致函数签名不一致
                                                             this, 
                                                             LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,//断连自动释放|复用Ip
                                                             10, 
                                                             (struct sockaddr*)&server_addr, 
                                                             sizeof(server_addr));
    if(listener == nullptr)
    {
        std::cout << "evconnlistener_new_bind()" <<std::endl;
        return;
    }

    //开启监听（循环）
    event_base_dispatch(m_base);

    //释放对象
    evconnlistener_free(listener);
    event_base_free(m_base);
}

void Server::ListenCb(struct evconnlistener * l, int fd, struct sockaddr * client_info, int socklen, void * arg)
{
    struct sockaddr_in* client_addr = (struct sockaddr_in*)client_info;
    char* client_ip = inet_ntoa(client_addr->sin_addr);
    int16_t client_port = client_addr->sin_port;
    std::cout << "[CLIENT INFO]IP:" << client_ip << "PORT:" << client_port <<std::endl;
}