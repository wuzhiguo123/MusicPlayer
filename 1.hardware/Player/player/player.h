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
#define ONLINE_URL "http://180.76.142.171/music/"
#define OFFLINE_URL "XXXX"


typedef struct Shm
{
    char cur_music[128];
    char cur_signer[128];
    int cur_mode;
    pid_t parent_pid;
    pid_t child_pid;
    pid_t grand_pid;
}Shm ;

union semun {
	int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET*/
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO(Linux-specific) */
};

void InitShm();
int InitSem();
void ShmDestroy();
void GetShm(Shm* music_info);
void UpdateMusic(int sig);
int FindNextMusic(int mode,char* cur,char* next);
int FindPrevMusic(int mode,char* cur,char* prev);
void StartPlay();
void StopPlay();
void SuspendPlay();
void ContinuePlay();
void NextPlay();
void PrevPlay();
void UpVolume();
void DownVolume();
void CirclePlay();
void SequencePlay();
void ReadAsrFifo();

#endif