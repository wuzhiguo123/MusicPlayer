#include "player.h"
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "link.h"
int g_shm_id = 0;
int g_start_flag = 0;
int g_suspend_flag = 0;
int g_device_mode = 0;
const char* path = "/tmp";
const int proj_id = 0x66;
extern MusicNode* music_head;

//共享内存相关，用来实现进程间共享歌曲的信息
key_t GetShmkey()
{
    key_t key = ftok(path, proj_id);
    if(key == -1)
    {
        perror("ftok() error");
        exit(-1);
    }
    return key;
}
int GetShmId(key_t key)
{
    g_shm_id  = shmget(key,SHMSIZE,IPC_CREAT|IPC_EXCL|0X666);
    if(g_shm_id  == -1)
    {
        perror("shmget() error");
        exit(-1);
    }
    return g_shm_id;
}
char* ShmAttach()
{
    void* addr = shmat(g_shm_id ,NULL,0);
    if(addr == (void*)-1)
    {
        perror("shmat() error");
        exit(-1);
    }
    return (char*)addr;
}
void ShmDetach(char* addr)
{
    if(shmdt((void*)addr) == -1)
    {
        perror("shmdt() error");
        exit(-1);
    }
}
void ShmDestroy()
{
    if(shmctl(g_shm_id , IPC_RMID, NULL) == -1)
    {
        perror("shmctl() error");
        exit(-1);
    }
}

void InitShm()
{
    Shm cur_music;
    memset(&cur_music, 0, sizeof(cur_music));

    key_t key = GetShmkey();

    g_shm_id  = GetShmId(key);

    char* shm_addr = ShmAttach();

    cur_music.cur_mode = SEQUENCE;
    memcpy((void*)shm_addr,&cur_music,sizeof(cur_music));
    printf("SHARMEMORYINIT SUCCESS\n");
    ShmDetach(shm_addr);
    // ShmDestroy();

}

void GetShm(Shm* s)
{
    memset(s, 0, sizeof(Shm));
    void* addr = shmat(g_shm_id , NULL, 0);
    if(addr == (void*)-1)
    {
        perror("GetSham() error");
        return;
    }

    memcpy(s, addr, sizeof(Shm));

    ShmDetach(addr);
}


//播放相关

void ChildProcess(char* music_name)
{
    while(g_start_flag)
    {
        pid_t grand_pid = fork();
        if(grand_pid == -1)
        {
            perror("GRAND PID");
            return;
        }
        else if(grand_pid == 0)
        {
            char music_path[128] = {0};
            if(g_device_mode == ONLINE_MODE)
                strcpy(music_path,ONLINE_URL);
            strcat(music_path, music_name);
            char* arg[7] = {0};
            arg[0] = "mplayer";
            arg[1] = music_path;
            arg[2] = "-slave";
            arg[3] = "-quiet";
            arg[4] = "-input";
            arg[5] = "file=./cmd_fifo";
            arg[6] = NULL;
            if(execv("/usr/bin/mplayer",arg) == -1)
            {
                fprintf(stderr, "[ERROR]EXECV ERROR\n");
            }

        }
        else {
            int status;
            wait(&status);
        }

    }
}

void PlayMusic(char* music_name)
{
    pid_t child_pid = fork();
    if(child_pid == -1)
    {
        perror("CHILD PID");
        return;
    }
    else if(child_pid == 0)
    {
        ChildProcess(music_name);
        exit(0);
    }
    else {
        // int status;
        // wait(&status);
    }
}
void StartPlay()
{
    if(g_start_flag == 1)
        return;
    char music_name[128] = {0};
    strcpy(music_name, music_head->music_name);
    g_start_flag = 1;
    printf("音乐名称:%s",music_name);
    PlayMusic(music_name);
}



