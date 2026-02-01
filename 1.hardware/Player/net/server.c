#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void SendInfo(struct json_object* snd_obj,int fd)
{
    char buffer[1024] = {0};
    int len = 0;
    //序列化
    const char* info = json_object_to_json_string(snd_obj);
   
    if(info == NULL)
    {
        perror("jason_object_to_json_string() error");
    }
    //简单的头部长度校验协议，前四个字节放入消息的字节长度
    len = strlen(info);
    memcpy(buffer, &len, sizeof(int));
    memcpy(buffer+sizeof(int),info , len);
    printf("[SERVER SEND]%s\n",info);
    if(send(fd,buffer,len+4,0)  < 0)
    {
        perror("send() error");
        return;
    }
}

void* ReciveMusicInfo(void* arg)
{
    int conn_fd = *(int*) arg;
    char buffer[1024] = {0};
    int len = 0;
    size_t size = 0;

    while(1)
    {

        while(1)
        {
            ssize_t r =recv(conn_fd, buffer+size, sizeof(int) - size, 0);
            if(r == (ssize_t)0)
            {
                close(conn_fd);
                printf("断开连接！\n");
                pthread_exit(NULL);
            }
            size = (size_t)r;
            if(size == sizeof(int))
                break;
        }
        size = 0;
        len = *(int*)buffer;
        memset(buffer, 0, sizeof(buffer));
        while(1)
        {
            ssize_t r = recv(conn_fd, buffer+size, len - size, 0); 
            if(r == (ssize_t)0)
            {
                close(conn_fd);
                printf("断开连接！\n");
                pthread_exit(NULL);
            }
            size += (size_t)r;
            if(size == len)
                break;
        }
        size = 0;
        printf("[SERVER REC]%s\n",buffer);

        //把收到的字符串转换成JSON对象
        struct json_object* music_info = json_tokener_parse(buffer);
        struct json_object* val = json_object_object_get(music_info, "cmd");
        if(strcmp("get_music_list", json_object_get_string(val)) == 0)
        {
            //返回音乐数据
            struct json_object* snd_obj = json_object_new_object();
            json_object_object_add(snd_obj, "cmd", json_object_new_string("reply_music_list"));

            struct json_object* music_array = json_object_new_array();
            json_object_array_add(music_array, json_object_new_string("其他/以后的以后.mp3"));
            json_object_array_add(music_array, json_object_new_string("其他/倾国倾城.mp3"));
            json_object_array_add(music_array, json_object_new_string("其他/童话.mp3"));
            json_object_array_add(music_array, json_object_new_string("其他/那些年.mp3"));
            json_object_array_add(music_array, json_object_new_string("其他/一直想着他.mp3"));
            // json_object_array_add(music_array, json_object_new_string("其他/1.mp3"));
            // json_object_array_add(music_array, json_object_new_string("其他/2.mp3"));
            // json_object_array_add(music_array, json_object_new_string("其他/3.mp3"));
            // json_object_array_add(music_array, json_object_new_string("其他/4.mp3"));
            // json_object_array_add(music_array, json_object_new_string("其他/5.mp3"));

            json_object_object_add(snd_obj, "music", music_array);
            SendInfo(snd_obj,conn_fd);
            sleep(1);

        }
        memset(buffer, 0, sizeof(buffer));
    }
}


int main()
{
    int listen_fd = socket(AF_INET,SOCK_STREAM,0);

    if (listen_fd < 0)
    {
        perror("socket() error");
    }

    struct sockaddr_in server_info;
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(8008);
    server_info.sin_addr.s_addr = htonl(INADDR_ANY);


    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    if(bind(listen_fd,(struct sockaddr*)&server_info,sizeof(server_info)) < 0)
    {
        perror("bind() error");
    }

    if(listen(listen_fd,64) < 0)
    {
        perror("listen() error");
    }
    while(1)
    {
        struct sockaddr_in client_addr;
        int len = sizeof(client_addr);
        int conn_fd = accept(listen_fd,(struct sockaddr*)&client_addr ,&len);

    

        if(conn_fd < 0)
        {
            perror("accept() error");
        }
        printf("connect success ,fd = %d\n",conn_fd);
        
        pthread_t id;
        pthread_create(&id, NULL, ReciveMusicInfo, &conn_fd);
        //测试socket播放
        // sleep(3);
        // struct json_object* obj1 = json_object_new_object();
        // json_object_object_add(obj1, "cmd", json_object_new_string("app_start"));
        // SendInfo(obj1,conn_fd);
        // json_object_put(obj1);


        // sleep(5);
        // struct json_object* obj3 = json_object_new_object();
        // json_object_object_add(obj3, "cmd", json_object_new_string("app_suspend"));
        // SendInfo(obj3,conn_fd);
        // json_object_put(obj3);

        // sleep(5);
        // struct json_object* obj4 = json_object_new_object();
        // json_object_object_add(obj4, "cmd", json_object_new_string("app_continue"));
        // SendInfo(obj4,conn_fd);
        // json_object_put(obj4);

        // sleep(5);
        // struct json_object* obj5 = json_object_new_object();
        // json_object_object_add(obj5, "cmd", json_object_new_string("app_next"));
        // SendInfo(obj5,conn_fd);
        // json_object_put(obj5);

        // sleep(5);
        // struct json_object* obj6 = json_object_new_object();
        // json_object_object_add(obj6, "cmd", json_object_new_string("app_prev"));
        // SendInfo(obj6,conn_fd);
        // json_object_put(obj6);

        // sleep(5);
        // struct json_object* obj7 = json_object_new_object();
        // json_object_object_add(obj7, "cmd", json_object_new_string("app_downvolume"));
        // SendInfo(obj7,conn_fd);
        // json_object_put(obj7);

        //  sleep(5);
        // struct json_object* obj8 = json_object_new_object();
        // json_object_object_add(obj8, "cmd", json_object_new_string("app_upvolume"));
        // SendInfo(obj8,conn_fd);
        // json_object_put(obj8);

        // sleep(5);
        // struct json_object* obj2 = json_object_new_object();
        // json_object_object_add(obj2, "cmd", json_object_new_string("app_stop"));
        // SendInfo(obj2,conn_fd);
        // json_object_put(obj2);


        pthread_join(id, NULL);
        printf("断开连接\n");
    }

    return 0;
}