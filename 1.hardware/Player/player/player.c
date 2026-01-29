#include "player.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <x86_64-linux-gnu/sys/select.h>
#include "link.h"
#include "net.h"
#include "device.h"
#include "select.h"
int g_shm_id = 0;
int g_start_flag = 0;
int g_suspend_flag = 0;
int g_device_mode = 0;
int g_sem_id = 0;
const char* path = "/tmp";
const int proj_id = 0x66;
const int SEMKEY = 1000;
extern MusicNode* music_head;
extern int g_asrfd;
extern fd_set READSET;
extern int g_maxfd;
extern int g_ttsfd;
char buffer[256] = {0};


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
    cur_music.parent_pid = getpid();
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

void SetShm(Shm* s)
{
    void* addr = shmat(g_shm_id,NULL,0);
    if(addr == (void*)-1)
    {
        perror("SetShm Error");
    }
    memcpy(addr,s,sizeof(*s));
    ShmDetach(addr);
}

//进程间同步
//二值信号量充当进程间的互斥锁
int InitSem()
{
    g_sem_id = semget(SEMKEY, 1, IPC_CREAT | IPC_EXCL | 0x666);
    if(g_sem_id == -1)
    {
        perror("semget()");
        return -1;
    }

    union semun s;//.h文件里面自己构建，内核不提供
    s.val = 1;//初始值设置为1，表明一开始是允许进入临界区的，=0的时候就不能进入临界区了
    if(semctl(g_sem_id, 0, SETVAL, s) == -1)
    {
        perror("semctl()");
        return -1;
    }
    printf("Init Sem Success\n");
    return 0;
}   
void SemP()
{
    struct sembuf s;
    s.sem_num = 0;
    s.sem_op = -1;
    s.sem_flg = SEM_UNDO;//异常中断时退出释放掉锁
    if(semop(g_sem_id,&s, 1) == -1)
    {
        perror("SemP()");
    }
}
void SemV()
{
    struct sembuf s;
    s.sem_num = 0;
    s.sem_op = 1;
    s.sem_flg = SEM_UNDO;//异常中断时退出释放掉锁
    if(semop(g_sem_id,&s, 1) == -1)
    {
        perror("SemV()");
    }
}


//播放相关

int FindNextMusic(int mode,char* cur,char* next)
{
    if(cur == NULL || next == NULL)
        return -1;
    MusicNode* p = music_head;
    while(p)
    {
        if(strstr(p->music_name,cur))
            break;
        p = p->next;
    }
    if(p->next == NULL)
    {
        return -1;
    }
    
    strcpy(next,p->next->music_name);
    printf("当前播放的为:%s   下一首播放的可取名称为:%s\n",cur,next);
    return 0;
}

int FindPrevMusic(int mode,char* cur,char* prev)
{
    printf("开始寻找上一首\n");
    if(cur == NULL || prev == NULL)
        return -1;

    MusicNode* p = music_head;
    if(strstr(p->music_name ,cur))
    {
        //先写死，链表到头节点了，就先一直放头节点的
        return 0;
    }
    while (p) {
        if(strstr(p->next->music_name,cur))
        {
            printf("找到了上一首：%s\n",p->music_name);
            break;
        }
        p = p->next;
    }
    strcpy(prev, p->music_name);
    printf("上一首的歌名为:%s\n",p->music_name);
    return 1;
}

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
        else if(grand_pid == 0)//孙进程
        {
            SemP();
            Shm cur_info;
            GetShm(&cur_info);
            if(strlen(music_name) == 0)
            {   if(FindNextMusic(cur_info.cur_mode,cur_info.cur_music,music_name)==-1)
                    {
                        //给父子进程发送信号
                        //父进程：重新请求一个新的歌曲链表
                        //子进程修改g_start_flag，防止再进入播放环节
                        printf("##############一个播放列表结束############\n");
                        kill(cur_info.parent_pid,SIGUSR1);
                        kill(cur_info.child_pid,SIGUSR1);
                        exit(0);
                    }
            }
          
            cur_info.child_pid = getppid();
            cur_info.grand_pid = getpid();
            if(g_device_mode == ONLINE_MODE)
            {
                const char* p = music_name;
                while(*p != '/')
                {
                    p++;
                }
                strncpy(cur_info.cur_signer, music_name,p-music_name);
                strcpy(cur_info.cur_music, p+1);
            }
            SetShm(&cur_info);
            SemV();

            char music_path[128] = {0};
            if(g_device_mode == ONLINE_MODE)
                strcpy(music_path,ONLINE_URL);
            strcat(music_path, music_name);
            char* arg[7] = {0};
            arg[0] = "mplayer";
            arg[1] = music_path;
            printf("##################################################musicpath:%s\n",music_path);
            arg[2] = "-slave";
            arg[3] = "-quiet";
            arg[4] = "-input";
            arg[5] = "file=/root/fifo/cmd_fifo";
            arg[6] = NULL;
            if(execv("/usr/bin/mplayer",arg) == -1)
            {
                fprintf(stderr, "[ERROR]EXECV ERROR\n");
                kill(cur_info.child_pid, SIGUSR1);
                exit(-1);
            }

        }
        else {//子进程
            memset(music_name, 0, strlen(music_name));
            int status;
            // Shm cur_info;
            // GetShm(&cur_info);
            wait(&status);
        }

    }
}


void ChildQuitProcess(int sig)
{
    FILE* test = fopen("./test.txt",O_WRONLY);
    fprintf(test,"子进程退出\n");
    fclose(test);
    g_start_flag = 0;//让子进程退出循环，结束子进程
}

void UpdateMusic(int sig)
{
    g_start_flag = 0;//把父进程的标志位也设置为0
    //回收子进程
    SemP();
    Shm cur_info;
    GetShm(&cur_info);
    SemV();
    int status;
    waitpid(cur_info.child_pid, &status,0);
    //清空链表
    ClearMusicList();
    //请求新的数据
    GetMusicName(cur_info.cur_signer);

    StartPlay();

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
        signal(SIGUSR1,ChildQuitProcess);
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

    printf("STARTPLAY!!!!!!!!!!!!!!!!!!\n,g_start_pay = %d",g_start_flag);
    if(g_suspend_flag == 1)
    {
        ContinuePlay();
        return;
    }
    if(g_start_flag == 1)
        return;
    char music_name[128] = {0};
    signal(SIGUSR1, UpdateMusic);
    // printf("##############################MUSICHEAD_NAME:::%s\n",music_head->music_name);
    strcpy(music_name, music_head->music_name);
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!MUSIC_HEAD:%p\n",music_head);
    // printf("##############################MUSICHEAD_NAME:::%s\n",music_name);
    g_start_flag = 1;
    // printf("音乐名称:%s",music_name);
    PlayMusic(music_name);
}

void WriteFifo(const char* cmd)
{
    int fd = open("/root/fifo/cmd_fifo",O_WRONLY);
    if(fd == -1)
    {
        perror("[Open Fifo Error]");
        return;
    }

    write(fd, cmd, strlen(cmd));
    close(fd);
    return;

}

void StopPlay()
{
    if(g_start_flag == 0)
        return;
    Shm cur_info;
    GetShm(&cur_info);
    //通知子进程退出播放
    kill(cur_info.child_pid, SIGUSR1);
    g_start_flag = 0;
    g_suspend_flag = 0;
    //结束mplayer
    WriteFifo("quit\n");
    //回收子进程资源
    int status;
    waitpid(cur_info.child_pid, &status,0);
}


void SuspendPlay()
{
    if(g_start_flag == 0 || g_suspend_flag == 1)
    {
        printf("未执行任何操作\n");
        return;
    }

    WriteFifo("pause\n");
    g_suspend_flag = 1;
    printf("暂停播放\n");
}


void ContinuePlay()
{
     if(g_suspend_flag == 0)
     {
        printf("未执行任何操作\n");
        return;
     }

    WriteFifo("pause\n");
    g_suspend_flag = 0;
    printf("继续播放\n");
}


void NextPlay()
{
    
    if(g_start_flag == 0)
        return;
    SemP();
    Shm cur_info;
    GetShm(&cur_info);
    char next[128] = {0};
    if(cur_info.cur_mode == CIRCLE)
    {
        strcpy(next, cur_info.cur_signer);
        strcat(next, "/");
        strcat(next, cur_info.cur_music);
        printf("循环播放：%s\n",next);
    }
    else if(FindNextMusic(cur_info.cur_mode, cur_info.cur_music,next) == -1)//当前链表的5首已经都放完了
    {
        StopPlay();
        ClearMusicList();
        GetMusicName(cur_info.cur_signer);
        StartPlay();
        return;
    }
    //当前链表的5首还没有被放完
    //此时孙进程已经被execv()替换为mplayer了
    const char* p = next;

    while(p)
    {
        if(*p == '/')
            break;
        p++;
    }
    strncpy(cur_info.cur_signer,next,p-next);
    strcpy(cur_info.cur_music,p+1);
    SetShm(&cur_info);
    SemV();
    char cmd[256];
    char music_path[128] = {0};
    strcpy(music_path,ONLINE_URL);
    strcat(music_path,next);
    sprintf(cmd,"loadfile %s\n",music_path);
    WriteFifo(cmd);
    g_suspend_flag = 0;
}

void PrevPlay()
{
    if(g_start_flag == 0)
        return;
    SemP();
    Shm cur_info;
    GetShm(&cur_info);
    char prev[128] = {0};
    if(cur_info.cur_mode == CIRCLE)
    {
        strcpy(prev, cur_info.cur_signer);
        strcat(prev, "/");
        strcat(prev, cur_info.cur_music);
        printf("循环播放：%s\n",prev);
    }
    else if(FindPrevMusic(cur_info.cur_mode,cur_info.cur_music,prev) == 0)
    {
        //已经被写死了，到头节点先循环播放第一首
       strcpy(prev,music_head->music_name);
    }
    const char* p = prev;

    while(p)
    {
        if(*p == '/')
            break;
        p++;
    }
    strncpy(cur_info.cur_signer,prev,p-prev);
    strcpy(cur_info.cur_music,p+1);
    SetShm(&cur_info);
    SemV();
    char cmd[256];
    char music_path[128] = {0};
    strcpy(music_path,ONLINE_URL);
    strcat(music_path,prev);
    sprintf(cmd,"loadfile %s\n",music_path);
    WriteFifo(cmd);
    g_suspend_flag = 0;

}
 

void UpVolume()
{
    int valume;
    GetVolume(&valume);
    if(valume >= 90)
    {
        valume = 100;
    }
    else if(valume == 100)
    {
        printf("音量已经是最大了\n");
    }
    else if(valume < 90)
    {
        valume += 10;
        printf("当前音量为:%d\n",valume);
        SetVolume(valume);
    }
}
void DownVolume()
{
    int valume;
    GetVolume(&valume);
    if(valume <= 10)
    {
        valume = 0;
    }
    else if(valume == 0)
    {
        printf("音量已经是最小了\n");
    }
    else if(valume > 10)
    {
        valume -= 10;
        printf("当前音量为:%d\n",valume);
        SetVolume(valume);
    }
}

void CirclePlay()
{
    Shm cur_info;
    GetShm(&cur_info);
    cur_info.cur_mode  = 2;
    SetShm(&cur_info);
}
void SequencePlay()
{
    Shm cur_info;
    GetShm(&cur_info);
    cur_info.cur_mode  = 1;
    SetShm(&cur_info);
}

void ReplyKeyWords()
{
    char buffer[128] = "老大我在";
    write(g_ttsfd,buffer , strlen(buffer));
}

void TtsStop()
{
    FILE* fp = popen("pgrep tts", "r");
    if(fp == NULL)
    {
        fprintf(stderr,"ponen() error!\n");
    }
    char buff[64] = {0};
    fgets(buff, sizeof(buff), fp);
    pid_t tts_pid = atoi(buff);
    if(strlen(buff))
    {
        kill(tts_pid, SIGUSR1);
    }
}

void ProcessAsrSignal(char* signal)
{
    if(signal == NULL || strlen(signal) == 0)
        return;
    if(strstr(signal, "触发关键词"))
    {
        SuspendPlay();
        TtsStop();
        ReplyKeyWords();
    }
    else if(strstr(signal, "听歌") || strstr(signal,"开始"))
    {
        StartPlay();
    }
    else if(strstr(signal, "停止") || strstr(signal,"不想听") || strstr(signal,"退出") || strstr(signal,"关闭"))
    {
        StopPlay();
    }
    else if (strstr(signal, "暂停")|| strstr(signal,"休息") || strstr(signal,"电话")||strstr(signal,"急事")) 
    {
        SuspendPlay();
    }
    else if(strstr(signal,"继续") || strstr(signal,"恢复"))
    {
        ContinuePlay();
    }
    else if((strstr(signal,"下") || strstr(signal,"换"))&& strstr(signal, "首"))
    {
        NextPlay();
    }
    else if((strstr(signal,"上") || strstr(signal,"刚")) && strstr(signal,"首"))
    {
        PrevPlay();
    }
    else if(strstr(signal,"音") && strstr(signal,"调大"))
    {
        UpVolume();
    }
    else if(strstr(signal,"音") &&  strstr(signal,"调小"))
    {
        DownVolume();
    }
    else if(strstr(signal,"循环"))
    {
        CirclePlay();
    }
    else if (strstr(signal,"顺序")) {
        SequencePlay();
    }
    else if(strstr(signal,"周杰伦"))
    {
        StopPlay();
        ClearMusicList();
        GetMusicName("周杰伦");
        StartPlay();
    }
    else if(strstr(signal,"陈奕迅"))
    {
        StopPlay();
        ClearMusicList();
        GetMusicName("陈奕迅");
        StartPlay();
    }
    else if(strstr(signal,"许嵩"))
    {
        StopPlay();
        ClearMusicList();
        GetMusicName("许嵩");
        StartPlay();
    }
    else {
        char cmd[1024] = {0};
        sprintf(cmd, "/root/MusicPlayer/1.hardware/Chatmodel/build/qianwen %s", signal);
        system(cmd);
    }
    memset(buffer, 0, sizeof(buffer));
}

void ReadAsrFifo()
{
    int ret = read(g_asrfd, buffer, sizeof(buffer));
    if(ret == -1)
    {
        fprintf(stderr,"READ ERROR");
        return;
    }
    else if(ret == 0)
    {
        close(g_asrfd);
        FD_CLR(g_asrfd, &READSET);
        g_maxfd = (g_maxfd == g_asrfd ? g_maxfd-1 : g_maxfd);
        printf("语言模型已关闭，将关闭语音控制功能\n");
        return;
    }
    printf("-------->%s\n",buffer);
    ProcessAsrSignal(buffer);
}