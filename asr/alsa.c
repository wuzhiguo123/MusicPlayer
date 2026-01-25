#include "alsa.h"
#include <stdio.h>
snd_pcm_t *pcmp;
snd_pcm_uframes_t frams_per_buffer = 1024;
unsigned int sample_rate = RATE;
int InitAlsa(){
    	int ret;

	// 1.打开PCM设备
	ret = snd_pcm_open(&pcmp, RECORD_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
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
	unsigned int actual_rate = sample_rate; 
	snd_pcm_hw_params_set_rate_near(pcmp, params, &actual_rate, 0);
	if (actual_rate != RATE)
	{
		printf("实际采样频率 %u\n", actual_rate);
		sample_rate = actual_rate;
	}

	// 7.设置缓冲区的周期大小：1024帧
	snd_pcm_hw_params_set_period_size_near(pcmp, params, &frams_per_buffer, 0);

	// 8.应用参数
	snd_pcm_hw_params(pcmp, params);

	// 9.准备设备，随时开始工作
	snd_pcm_prepare(pcmp);

	return 0;
}