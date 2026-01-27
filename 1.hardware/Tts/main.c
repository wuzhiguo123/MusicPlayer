#include "tts.h"
#include "alsa.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
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

    int tts_fd = open("/root/fifo/tts_fifo", O_RDONLY);
    while (1) {
        char buffer[128] = {0};
        ssize_t r = read(tts_fd, buffer,sizeof(buffer));
        if(r < 0)
        {
            fprintf(stderr,"READ ERROR");
        }
        SherpaOnnxOfflineTtsGenerateWithCallback(tts, buffer, 
								1, 1.0, PlayCallBack);
    }
    return 0;
}