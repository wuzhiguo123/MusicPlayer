#include "tts.h"
#include "sherpa-onnx/c-api/c-api.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

const SherpaOnnxOfflineTts *tts;
int32_t target_rate = 0;
int InitSherpaTts()
{
    SherpaOnnxOfflineTtsConfig config;
	memset(&config, 0, sizeof(config));

	config.rule_fsts = "/root/model/vits-icefall-zh-aishell3/date.fst,/root/model/vits-icefall-zh-aishell3/new_heteronym.fst,/root/model/vits-icefall-zh-aishell3/number.fst,/root/model/vits-icefall-zh-aishell3/phone.fst";

	config.model.num_threads = 4;
	config.model.provider = "cpu";

	config.model.vits.model = "/root/model/vits-icefall-zh-aishell3/model.onnx";
	config.model.vits.lexicon = "/root/model/vits-icefall-zh-aishell3/lexicon.txt";
	config.model.vits.tokens = "/root/model/vits-icefall-zh-aishell3/tokens.txt";

	//创建TTS实例
	tts = SherpaOnnxCreateOfflineTts(&config);
	if (NULL == tts)
	{
		printf("创建TTS实例失败\n");
		return -1;
	}

	//获取大模型输出的采样频率
	target_rate = SherpaOnnxOfflineTtsSampleRate(tts);
	printf("模型采样频率 %u\n", target_rate);

	return 0;
}