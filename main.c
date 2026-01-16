#include "select/select.h"
#include "musiclink/link.h"
#include "player.h"
#include "net/net.h"
#include <stdlib.h>
#include "device.h"
#include "main.h"
extern MusicNode* music_head;
void CheckMusicList()
{
    printf("%p",music_head);

    MusicNode* cur = music_head;
    while(cur)
    {
        printf("name:%s",cur->music_name);

        cur = cur->next;
    }
    printf("\n");
}
void ShowMenu()
{
    system("clear");
    printf("\t1.开始播放\t2.结束播放\n");
    printf("\t3.暂停播放\t4.结束播放 \n");
    printf("\t5.下一首\t6.上一首\n");
    printf("\t7.增加音量\t8.减少音量\n");
    printf("\t9.单曲循环\t10.结束播放\n");

}
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

    SetVolume(DEFAULT_VOLUME);

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

    //获取音乐文件名
    GetMusicName("其他");

    // CheckMusicList();
    ShowMenu();
    MySelect();

    while(1)
    {}
}