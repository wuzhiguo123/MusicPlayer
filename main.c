#include "select/select.h"
#include "musiclink/link.h"
#include "player.h"
#include "net/net.h"
#include <signal.h>
#include <stdlib.h>
#include "device.h"
#include "main.h"
extern MusicNode* music_head;
void CheckMusicList()
{

    MusicNode* cur = music_head;
    printf("_________________________________MUSIC_HEAD:%p\n",music_head);
    int count = 0;
    while(cur)
    {
        printf("%s\n",cur->music_name);
        cur = cur->next;
        count++;
    }
    printf("一共%d首歌\n",count);

}
void ShowMenu()
{
    system("clear");
    printf("\t1.开始播放\t2.结束播放\n");
    printf("\t3.暂停播放\t4.继续播放 \n");
    printf("\t5.下一首\t6.上一首\n");
    printf("\t7.增加音量\t8.减少音量\n");
    printf("\t9.单曲循环\t10.顺序播放\n");

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
    InitSem();

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
    ShowMenu();
    GetMusicName("其他");
    MySelect();



    CheckMusicList();
 

    while(1)
    {}
}