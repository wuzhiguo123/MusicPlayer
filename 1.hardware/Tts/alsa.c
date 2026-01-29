#include "alsa.h"
#include "alsa/asoundlib.h"
#include <alloca.h>
#include <asm-generic/errno-base.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
snd_pcm_t *pcmp;
extern int32_t target_rate;
extern int play_flag;
int InitAlsa(){
   int ret;

	// 1.打开PCM设备
	ret = snd_pcm_open(&pcmp, PLACKBACK_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
	if (ret != 0)
	{
		fprintf(stderr, "snd_pcm_open error");
		return -1;
	}

	snd_pcm_hw_params_t *params;
	// 2.初始化硬件参数结构体（申请内存） 宏函数
	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcmp, params);

	// 3.设置访问模式：多声道交错存储
	snd_pcm_hw_params_set_access(pcmp, params, SND_PCM_ACCESS_RW_INTERLEAVED);

	// 4.设置采样格式：S16_LE
	ret = snd_pcm_hw_params_set_format(pcmp, params, SND_PCM_FORMAT_S16_LE);
	if (ret != 0)
	{
		fprintf(stderr, "不支持S16_LE\n");
		return -1;
	}

	// 5.设置通道：单声道
	snd_pcm_hw_params_set_channels(pcmp, params, 1);

	// 6.设置采样频率：44100Hz   44098  44106
	unsigned int actual_rate = target_rate;
	snd_pcm_hw_params_set_rate_near(pcmp, params, &actual_rate, 0);


	printf("播放频率为%u\n",actual_rate);

	// 7.设置缓冲区大小
    snd_pcm_uframes_t frames_buffer = 16384;
	snd_pcm_hw_params_set_buffer_size_near(pcmp, params, &frames_buffer);

	// 8.应用参数
	snd_pcm_hw_params(pcmp, params);

	// 9.准备设备，随时开始工作
	snd_pcm_prepare(pcmp);

	return 0;
}

int32_t PlayCallBack(const float *samples, int32_t n)//samples:数据;n:数据帧数
{
	printf("playcallback\n");
	if(!play_flag)
		return 0;
    int16_t* play_buffer = malloc(sizeof(int16_t)*n);
    for(int i = 0; i< n;i++)
    {
        float tmp = samples[i];
        if(tmp < -1.0)
            tmp = -1.0;
        if(tmp > 1.0)
            tmp = 1.0;
        play_buffer[i] = (int16_t)(tmp*32767);
    }

    int ret = snd_pcm_writei(pcmp, play_buffer, n);
    if(ret == -EPIPE)
    {
		snd_pcm_prepare(pcmp);
		ret = snd_pcm_writei(pcmp, play_buffer, n);
		if (ret < 0)
		{
			printf("缓冲区欠载，重写失败\n");
			return 0;
		}
    }

    if(ret < 0)
    {
        fprintf(stderr,"写入失败\n");
        return 0;
    }
	free(play_buffer);
    return play_flag;
}
