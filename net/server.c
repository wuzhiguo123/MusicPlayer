#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void* ReciveMusicInfo(void* arg)
{
    int conn_fd = *(int*) arg;
    char buffer[1024] = {0};
    int len = 0;
    size_t size = 0;

    while(1)
    {
        printf("123123");
        while(1)
        {
            size+=recv(conn_fd, buffer+size, sizeof(int) - size, 0);
            if(size == sizeof(int))
                break;
        }
        size = 0;
        len = *(int*)buffer;
        memset(buffer, 0, sizeof(buffer));
        while(1)
        {
            size += recv(conn_fd, buffer+size, len - size, 0);
            if(size == len)
                break;
        }
        size = 0;
        printf("%s\n",buffer);
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
    server_info.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(listen_fd,(struct sockaddr*)&server_info,sizeof(server_info)) < 0)
    {
        perror("bind() error");
    }

    if(listen(listen_fd,64) < 0)
    {
        perror("listen() error");
    }

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
    while(1)
    {
    }
    close(listen_fd);
    close(conn_fd);


    return 0;
}