#include "select.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdio.h>
#include "player.h"
#include "net.h"
#include "device.h"
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>
#include "main.h"

extern fd_set READSET;
extern pthread_t tid;
extern int g_maxfd;
extern int g_sockfd;
extern int g_button_fd;
void InitSelect()
{
    FD_ZERO(&READSET);
    FD_SET(0, &READSET);
    printf("SELECT INIT SUCCESS!\n");
}

void ReadStdIO()
{
    char key;
    scanf("%c",&key);
    printf("[SELECT] READ STDIO:%c",key);
    switch(key)
    {   
        case '1':
            StartPlay();
            break;
        case '2':
            StopPlay();
            break;
        case '3':
            SuspendPlay();
            break;
        case '4':
            ContinuePlay();
            break;
        case '5':
            NextPlay();
            break;
        case '6':
            PrevPlay();
            break;
        case '7':
            UpVolume();
            break;
        case '8':
            DownVolume();
            break;
        case 'c':
            CirclePlay();
            break;
        case 's':
            SequencePlay();
            break;
    }
    fflush(stdin);
}

void MySelect()
{

    printf("开启监听\n");
    fd_set TMPSET;
    while(1)
    {
        TMPSET = READSET;
        int ret = select(g_maxfd + 1, &TMPSET, NULL, NULL, NULL);
        if(ret == -1 && errno == EINTR)//如果监听是被信号signal打断的不算是错误
        {
            continue;
        }
        else if(ret == -1 && errno != EINTR)//如果监听不是被信号打断，打印错误信息
        {
            perror("SELECT");
            continue;
        }

        if(FD_ISSET(0, &TMPSET))//终端输入
        {
            ReadStdIO();
        }
        else if(FD_ISSET(g_sockfd, &TMPSET))//网络
        { 
            ReadSocket();
        }

        else if(FD_ISSET(g_button_fd,&TMPSET))
        {
            ReadButton();
        }
    }

}