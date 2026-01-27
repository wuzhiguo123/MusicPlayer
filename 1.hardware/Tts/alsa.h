#ifndef _ALSA_H
#define __ALSA_H
#include "alsa/asoundlib.h"
#define PLACKBACK_DEVICE "hw:0,0"

int InitAlsa();
int32_t PlayCallBack(const float *samples, int32_t n);//samples:数据;n:数据帧数


#endif