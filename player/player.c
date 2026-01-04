#include "player.h"
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/shm.h>

int g_shm_id = 0;
int g_start_flag = 0;
int g_suspend_flag = 0;
const char* path = "/tmp";
const int proj_id = 0x66;
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


