#include "alsa/asoundlib.h"
#include "tts.h"
#include "alsa.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sherpa-onnx/c-api/c-api.h"
const SherpaOnnxOfflineTts *tts;
int play_flag = 1;
int tts_fd = -1;
extern snd_pcm_t *pcmp;
int running = 1;
int speaker_id = 1;


void CleanUp()
{
    running=0;
    if(tts)
        SherpaOnnxDestroyOfflineTts(tts);
    if(pcmp)
        snd_pcm_close(pcmp);
    if(tts_fd > 0)
        close(tts_fd);
    exit(0);
}

void HandleQuit()
{
    printf("TTS退出\n");
    CleanUp();
}
void HandleStop()
{

    play_flag = 0;
    snd_pcm_drop(pcmp);
}
void HandleChangeVoice()
{
    speaker_id = (speaker_id+1)%175;
}
int main()
{
    signal(SIGINT, HandleQuit);
    signal(SIGUSR1, HandleStop);
    signal(SIGUSR2, HandleChangeVoice);
    if(InitSherpaTts()== 0)
        printf("初始化TTS成功\n");
    else 
    {
        printf("初始化TTS失败\n");
        CleanUp();
        return -1;
    }

    if(InitAlsa() == 0)
      printf("初始化Alsa成功\n");
    else 
    {
        printf("初始化Alsa失败\n");
        CleanUp();
        return -1;
    }

    tts_fd = open("/root/fifo/tts_fifo", O_RDONLY);
    if(tts_fd < 0)
    {
        printf("打开管道失败\n");
        CleanUp();
        return -1;
    }
    char buffer[1024] = {0};
    while (running) {
        ssize_t r = read(tts_fd, buffer,sizeof(char)*128);
        if(r < 0)
        {
            fprintf(stderr,"READ ERROR");
        }
        else if (0 == r)
		{
			sleep(1);
			continue;
		}
        printf("--->%s\n",buffer);
        play_flag = 1;
        snd_pcm_prepare(pcmp);  // PREPARED状态
        SherpaOnnxOfflineTtsGenerateWithCallback(tts, buffer, 
								speaker_id, 1.0, PlayCallBack);
        	//等待缓冲区数据播放完毕
		snd_pcm_drain(pcmp);    // SETUP状态
        memset(buffer, 0, sizeof(buffer));

	
    }
    return 0;
}