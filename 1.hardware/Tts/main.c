#include "tts.h"
#include "alsa.h"
#include <stdio.h>
#include "sherpa-onnx/c-api/c-api.h"
const SherpaOnnxOfflineTts *tts;
int main()
{
    if(InitSherpaTts()== 0)
        printf("初始化TTS成功\n");
    else 
    {
        printf("初始化TTS失败\n");
        return -1;
    }

    if(InitAlsa() == 0)
      printf("初始化Alsa成功\n");
    else 
    {
        printf("初始化Alsa失败\n");
        return -1;
    }

    char buffer[128] = "今天天气真的很不错哟\n";
    SherpaOnnxOfflineTtsGenerateWithCallback(tts, buffer, 
								1, 1.0, PlayCallBack);

    return 0;
}