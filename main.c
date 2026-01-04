#include "select/select.h"
#include "musiclink/link.h"
#include "player.h"
#include "net/net.h"
#include <stdlib.h>
int main()
{
    system("bash init.sh");
    //初始化监听集合
    InitSelect();
    //初始化歌曲链表
    if(!InitMusicLink())
    {
        printf("InitMusicLink SUCCESS\n");
    }
    //初始化共享内存
    InitShm();

    //初始化网络
    int ret = InitSocket();
    if(ret < 0)
    {
        printf("OFFLINE MODE\n");
    }
    if(ret == 0)
    {
        printf("ONLINE MODE\n");
    }

    while(1)
    {}
}