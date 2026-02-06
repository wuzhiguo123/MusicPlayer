#include "sherpa-onnx/c-api/c-api.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const SherpaOnnxKeywordSpotter *kws_recognizer = NULL;
const SherpaOnnxOnlineStream   *kws_stream     = NULL;
extern int asr_fd;
extern int target_rate;
int InitSherpaKws()
{
    //配置config
    SherpaOnnxKeywordSpotterConfig config;

  	memset(&config, 0, sizeof(config));
  	config.model_config.transducer.encoder = "/root/model/sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01-mobile/encoder-epoch-12-avg-2-chunk-16-left-64.onnx";

  	config.model_config.transducer.decoder = "/root/model/sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01-mobile/decoder-epoch-12-avg-2-chunk-16-left-64.onnx";

  	config.model_config.transducer.joiner = "/root/model/sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01-mobile/joiner-epoch-12-avg-2-chunk-16-left-64.int8.onnx";

  	config.model_config.tokens = "/root/model/sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01-mobile/tokens.txt";

  	config.model_config.provider = "cpu";
  	config.model_config.num_threads = 4;
  	config.model_config.debug = 0;

  	config.keywords_file = "/root/model/sherpa-onnx-kws-zipformer-wenetspeech-3.3M-2024-01-01-mobile/keywords.txt";

	//创建识别器
	kws_recognizer = SherpaOnnxCreateKeywordSpotter(&config);
  	if (!kws_recognizer) 
	{
    	fprintf(stderr, "SherpaOnnxCreateKeywordSpotter ERROR\n");
    	return -1;
  	}

	//创建音频流
	kws_stream = SherpaOnnxCreateKeywordStream(kws_recognizer);
  	if (!kws_stream) 
	{
    	fprintf(stderr, "SherpaOnnxCreateKeywordStream ERROR\n");
    	return -1;
  	}

	return 0;
}

int SherpaKws(float *float_buffer, int resample_frams)
{
	int ret = 0;

	SherpaOnnxOnlineStreamAcceptWaveform(kws_stream, target_rate, float_buffer, resample_frams);

	while (SherpaOnnxIsKeywordStreamReady(kws_recognizer, kws_stream)) 
	{
    	SherpaOnnxDecodeKeywordStream(kws_recognizer, kws_stream);
    }
    const SherpaOnnxKeywordResult *r;
    r = SherpaOnnxGetKeywordResult(kws_recognizer, kws_stream);
    if (r && r->keyword && strlen(r->keyword) > 0)
    {
        printf("---> %s\n", r->keyword);
        char buffer[16] = "触发关键词";
        if(write(asr_fd,buffer,strlen(buffer)) < 0)
        {
            fprintf(stderr,"WRITE ASR_FD ERROR\n");
        }

        

        // if (device_mode == ONLINE_MODE && 
        // 	!strcmp(r->keyword, "你好小胖"))
        // {
        // 	if (write(asr_fd, r->keyword, strlen(r->keyword)) == -1)
        // 	{
        // 		perror("write fifo");
        // 	}

        // 	ret = 1;
        // }
        // else if (device_mode == OFFLINE_MODE && 
        // 		 strcmp(r->keyword, "你好小胖")) 
        // {  //离线模式，不需要 你好小胖 来唤醒
        // 	if (write(asr_fd, r->keyword, strlen(r->keyword)) == -1)
        // 	{
        // 		perror("write fifo");
        // 	}

        // 	ret = 1;
        // }
        ret = 1;

        SherpaOnnxResetKeywordStream(kws_recognizer, kws_stream);
    }
    SherpaOnnxDestroyKeywordResult(r);

	

	return ret;
}