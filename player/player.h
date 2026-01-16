#ifndef __PLAYER__H
#define __PLAYER__H

#include <sched.h>
#include <sys/shm.h>
#define SHMSIZE 4096
//播放模式
#define SEQUENCE 1
#define CIRCLE   2

#define ONLINE_MODE 1
#define OFFLINE_MODE 2
#define ONLINE_URL "http://47.94.80.54/music/"
#define OFFLINE_URL "XXXX"


typedef struct Shm
{
    char cur_music[128];
    int cur_mode;
    pid_t child_pid;
    pid_t grand_pid;
}Shm ;

void InitShm();
void ShmDestroy();
void GetShm(Shm* music_info);
void StartPlay();

#endif