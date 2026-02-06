#include "device.h"
#include "player.h"
#include <sys/select.h>
#include <linux/input.h>
#include <signal.h>

int g_button_fd;
extern int g_maxfd;
extern fd_set READSET;
extern int g_start_flag;
extern int g_suspend_flag;
struct timeval old,new;
BUTTON_STATE state = STATE_IDLE;
struct itimerval tv;
int SetVolume(long volume)
{
    int err;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;

    // 打开混音器
    if ((err = snd_mixer_open(&handle, 0)) < 0) {
        fprintf(stderr, "Mixer %s open error: %s\n", CARD_NAME, snd_strerror(err));
        return err;
    }

    // 连接到声卡
    if ((err = snd_mixer_attach(handle, CARD_NAME)) < 0) {
        fprintf(stderr, "Mixer attach %s error: %s\n", CARD_NAME, snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }

    // 注册混音器
    if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        fprintf(stderr, "Mixer register error: %s\n", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }

    // 加载混音器元素
    if ((err = snd_mixer_load(handle)) < 0) {
        fprintf(stderr, "Mixer load error: %s\n", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }

    // 创建混音器元素标识符
    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, SELE_NAME);

    // 获取混音器元素
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
    if (!elem) {
        fprintf(stderr, "Unable to find simple control '%s',%i\n", SELE_NAME, 0);
        snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return -1;
    }

    // 设置音量为50%
    long minv, maxv;
    snd_mixer_selem_get_playback_volume_range(elem, &minv, &maxv);

    printf("Volume set to %ld%%\n",volume);

    volume =  minv + volume * (maxv - minv) / 100;
    if ((err = snd_mixer_selem_set_playback_volume_all(elem, volume)) < 0) {
        fprintf(stderr, "Error setting volume: %s\n", snd_strerror(err));
        printf("设置失败！\n");
    }  
    // 清理
    snd_mixer_selem_id_free(sid);
    snd_mixer_close(handle);
    return 0;
}



int GetVolume(int* value)
{
    int err;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;

    // 打开混音器
    if ((err = snd_mixer_open(&handle, 0)) < 0) {
        fprintf(stderr, "Mixer %s open error: %s\n", CARD_NAME, snd_strerror(err));
        return err;
    }

    // 连接到声卡
    if ((err = snd_mixer_attach(handle, CARD_NAME)) < 0) {
        fprintf(stderr, "Mixer attach %s error: %s\n", CARD_NAME, snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }

    // 注册混音器
    if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        fprintf(stderr, "Mixer register error: %s\n", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }

    // 加载混音器元素
    if ((err = snd_mixer_load(handle)) < 0) {
        fprintf(stderr, "Mixer load error: %s\n", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }

    // 创建混音器元素标识符
    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, SELE_NAME);

    // 获取混音器元素
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
    if (!elem) {
        fprintf(stderr, "Unable to find simple control '%s',%i\n", SELE_NAME, 0);
        snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return -1;
    }
    long volume;
 
    if ((err = snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT,&volume)) < 0) {
        fprintf(stderr, "Error setting volume: %s\n", snd_strerror(err));
    } 

    long minv, maxv;
    snd_mixer_selem_get_playback_volume_range(elem, &minv, &maxv);

    *value = (int)(((volume - minv) * 100 + (maxv - minv)/2) / (maxv - minv));
    // 清理
    snd_mixer_selem_id_free(sid);
    snd_mixer_close(handle);
    return 0;

}

void InitButton()
{
    g_button_fd = open("/dev/input/event1", O_RDONLY);
    if(g_button_fd  == -1)
    {
        perror("OPEN ERROR");
        return;
    }
    FD_SET(g_button_fd, &READSET);

    g_maxfd = (g_maxfd < g_button_fd) ? g_button_fd : g_maxfd;

    

}

void Handle(int sig)
{
    printf("短按\n");
    if(g_start_flag == 0)
    {
        StartPlay();
    }
    else if(g_start_flag == 1 && g_suspend_flag == 0)
    {
        SuspendPlay();
    }
    else if(g_start_flag == 1 && g_suspend_flag == 1)
    {
        ContinuePlay();
    }
    state = STATE_IDLE;
}

void ReadButton()
{
    struct input_event ev;
    int ret = read(g_button_fd, &ev, sizeof(ev));
    signal(SIGALRM, Handle);
    if(ret == -1)
    {
        perror("READ ERROR");
        return;
    }

    if(ev.type != EV_KEY)
        return;

    if(ev.value == 1)
    {
        if(state == STATE_IDLE)
        {
            gettimeofday(&old, NULL);
            state = STATE_FIRST_PRESS;
        }
        else if(state == STATE_FIRST_RELEASE)
        {
            printf("双击\n");
            NextPlay();
            //多长时间触发一次定时器信号
            state = STATE_IDLE;
            tv.it_value.tv_sec = 0;
            tv.it_value.tv_usec = 0;

            //多长时间重复启动一次定时器
            tv.it_interval.tv_sec = 0;
            tv.it_interval.tv_usec = 0;
            setitimer(ITIMER_REAL, &tv, NULL);
        }

    }
    else if(ev.value == 0)
    {
        if(state == STATE_FIRST_PRESS)
        {
            gettimeofday(&new,NULL);
            if((new.tv_sec-old.tv_sec) * 1000 + (new.tv_usec-old.tv_usec)/1000 > 300)
            {
                printf("长按\n");
                PrevPlay();
                state = STATE_IDLE;
            }
            else {
                state = STATE_FIRST_RELEASE;
                tv.it_value.tv_sec = 0;
                tv.it_value.tv_usec = 300 * 1000;

                tv.it_interval.tv_sec = 0;
                tv.it_interval.tv_usec = 0;

                setitimer(ITIMER_REAL, &tv, NULL);
            }
        }
    }
}