#ifndef __PLAYER__H
#define __PLAYER__H

#include <sched.h>
#include <sys/shm.h>
#define SHMSIZE 4096
#define SEQUENCE 1
#define CIRCLE   2



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

#endif