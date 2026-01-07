#include <arpa/inet.h>
#include <json-c/json_object.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <json-c/json.h>
#include <pthread.h>
#include "json_object_iterator.h"
#include "select.h"
#include <unistd.h>
#include "net.h"
#include "player.h"
#include "main.h"
#include "device.h"
// #include "../select/select.h"
extern fd_set READSET;
int g_sockfd = 0;
int g_maxfd = 0;
extern int g_start_flag;
extern int g_suspend_flag;


void SendMusicInfo(struct json_object* obj)
{
    char buffer[1024] = {0};
    int len = 0;
    //序列化
    const char* info = json_object_to_json_string(obj);
    printf("%s\n",info);
    if(info == NULL)
    {
        perror("jason_object_to_json_string() error");
    }
    //简单的头部长度校验协议，前四个字节放入消息的字节长度
    len = strlen(info);
    memcpy(buffer, &len, sizeof(int));
    memcpy(buffer+sizeof(int),info , len);
    // printf("BUFFER %s\n",buffer);
    if(send(g_sockfd,buffer,len+4,0)  < 0)
    {
        perror("send() error");
        return;
    }
}

void* SendServer(void* arg)
{
    while(1)
    {
        //创建json对象
        struct json_object * obj = json_object_new_object();

        //往json中添加键对值
        json_object_object_add(obj, "cmd", json_object_new_string("info"));

        Shm music_info;
        GetShm(&music_info);

        json_object_object_add(obj, "cur_music", json_object_new_string(music_info.cur_music));
        json_object_object_add(obj, "mode", json_object_new_int(music_info.cur_mode));

        char status[8] = {0};

        //当前状态
        if(g_start_flag == 0)
            strcpy(status, "stop");
        else if(g_start_flag == 1 && g_suspend_flag == 0)
            strcpy(status, "start");
        else if(g_start_flag == 1 && g_suspend_flag == 1)
            strcpy(status,"suspend");

        json_object_object_add(obj, "status", json_object_new_string(status));
        json_object_object_add(obj, "deviceid", json_object_new_string(DEVICEID));
        //获取当前音量
        int value;
        GetVolume(&value);
        json_object_object_add(obj, "volume", json_object_new_int(value));

        //发送给服务器
        SendMusicInfo(obj);
        json_object_put(obj);
        sleep(2);
        
    }

}

int InitSocket()
{
    g_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(g_sockfd < 0)
    {
        perror("client socket() error");
    }

    int connect_cnt = 10;
    int connect_ret = 0;

    struct sockaddr_in server_info;
    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_addr.s_addr = inet_addr(IP);
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(PORT);

    while(connect_cnt--)
    {
        connect_ret = connect(g_sockfd,(struct sockaddr*)&server_info,sizeof(server_info));
        if(connect_ret < 0)
        {
            perror("connect() error");
            sleep(1);
            continue;
        }

        FD_SET(g_sockfd,&READSET);
        g_maxfd = (g_maxfd < g_sockfd) ? g_sockfd : g_maxfd;

        pthread_t id;
        if(pthread_create(&id, NULL, SendServer, 0) != 0)
        {
            perror("pthread_create() error");
            break;
        }
        pthread_detach(id);
        break;
    }
    return connect_ret;
}