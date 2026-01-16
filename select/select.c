#include "select.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdio.h>
#include "player.h"
#include <sys/select.h>

extern fd_set READSET;
extern int g_maxfd;

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

    }
}

void MySelect()
{
    fd_set TMPSET;
    TMPSET = READSET;
    while(1)
    {
        int ret = select(g_maxfd, &TMPSET, NULL, NULL, NULL);
        if(ret == -1 && errno == EINTR)//如果监听是被信号signal打断的不算是错误
        {
            continue;
        }
        else if(ret == -1 && errno != EINTR)//如果监听不是被信号打断，打印错误信息
        {
            perror("SELECT");
            continue;
        }

        if(FD_ISSET(0, &TMPSET))
        {
            ReadStdIO();
        }
    }

}