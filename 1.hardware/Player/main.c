#include "select/select.h"
#include "musiclink/link.h"
#include "player.h"
#include "net/net.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86_64-linux-gnu/sys/select.h>
#include "device.h"
#include "main.h"

extern MusicNode* music_head;
extern int g_maxfd;
int g_asrfd;
int g_ttsfd;
extern int g_device_mode;
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
void InitFifo()
{
    g_asrfd = open("/root/fifo/asr_fifo",O_RDONLY);
    g_ttsfd = open("/root/fifo/tts_fifo",O_WRONLY);
    if(g_asrfd < 0 || g_ttsfd < 0)
    {
        fprintf(stderr, "Open ERROR");
        exit(-1);
    }

    FD_SET(g_asrfd, &READSET);
    g_maxfd = (g_maxfd < g_asrfd) ? g_asrfd : g_maxfd;

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
    InitButton();
    InitFifo();


    SetVolume(DEFAULT_VOLUME);

    //初始化网络
    int ret = InitSocket();
    if(ret < 0)
    {
        g_device_mode = OFFLINE_MODE;
        printf("OFFLINE MODE\n");
        ChangeOfflineMode();
    }
    if(ret == 0)
    {
        printf("ONLINE MODE\n");
        GetMusicName("其他");
    }
    //获取音乐文件名
    ShowMenu();
    MySelect();
    CheckMusicList();
 
    

    while(1)
    {}
}