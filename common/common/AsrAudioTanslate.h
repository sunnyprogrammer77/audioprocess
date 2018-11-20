#pragma once
/*
/实现语言翻译
/文件必须是采样率8000HZ或者16000HZ, 单声道, 采样位数16位
*/
#include "common/AccountInfo.h"
#include "common/CommonTool.h"
#include "common/FileReader.h"
#include "hci_sys.h"
#include "hci_asr.h"
#include <fstream>
#include <sstream>
#include <string>
#if defined(WIN32) || defined(WIN64)
#include <Windows.h>
#endif
#ifdef __LINUX__
#include <stdio.h>
#include <unistd.h>
#endif

#define MAX_AUDIOBYTE 300000;
class AsrAudioTanslate
{
public:
	AsrAudioTanslate();
	~AsrAudioTanslate();
public:
	bool InitAccountInfo();
	bool HciInit();
	bool HciAsrInit();
	bool StartRecog(const char* audiofile,size_t buffer = 0, bool fileflag = true, const char* outfile = "test.txt");
	void CloseAsr();
	void CloseHci();
	void SetInterceptLen(size_t len);
	void SetAcountInfoTyp(const char* Type);
protected:
	//识别
	bool Recog(const string &cap_key, const string& recog_config, const char* audiofile, const char * file_name, size_t buffer);
	//实时识别
	bool RealtimeRecog(const string &cap_key, const string& recog_config, const char* audio_file, const char* outfile, size_t buffer);
private:
	AccountInfo *account_info;
	const char* audio_file;
	std::string asr_init_config;
	size_t m_InterceptLen;
	const char* m_AcountInfoType;
};

