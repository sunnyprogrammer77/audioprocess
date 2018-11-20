#pragma once
#include<iostream>
#include<string>
#define RIFF_SIGN_ID 0x46464952ul 
#define WAVE_SIGN_ID 0x45564157ul 
#define FMT__SIGN_ID 0x20746D66ul
#define FACT_SIGN_ID 0x74636166ul 
#define DATA_SIGN_ID 0x61746164ul
enum FILE_STAT{FILE_OPEN_BAD,FILE_OPEN_FAIL,FILE_OPEN_EOF,FILE_OPEN_NORMAL};//文件打开后的状态
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#ifndef WORD
typedef unsigned short WORD;
#endif
#ifndef BYTE
typedef unsigned char BYTE;
#endif
//基本的头文件结构44字节
struct HeaderType
{
	DWORD riff;                 /*RIFF类资源文件头部 4byte 0x46464952 'R' 'I' 'F' 'F'*/
	DWORD file_len;                /*文件长度4byte*/
	DWORD wave;                    /*"WAVE"标志4byte  WAV文件标志 0x45564157 'W','A','V','E'*/
	DWORD fmt;                  /*"fmt"标志4byte 波形格式标志 0x20746D66 'f','m','t',' '*/
	DWORD NI1;                     /*格式块长度4byte*/
	WORD format_type;              /*格式类别(10H为PCM形式的声音数据)2byte*/
	WORD Channels;                 /*Channels 1 = 单声道; 2 = 立体声2byte*/
	DWORD frequency;               /*采样频率4byte*/
	DWORD trans_speed;             /*音频数据传送速率4byte*/
	WORD dataBlock;                /*数据块的调整数（按字节算的）2byte*/
	WORD sample_bits;              /*样本的数据位数(8/16) 2byte*/
	DWORD data;                 /*数据标记符"data" 4byte 数据标志符 0x61746164, 'd','a','t','a' */
	DWORD wav_len; /*语音数据的长度 4byte*/
	char * wav_data;
	HeaderType()
	{
		riff = 0;
		wave = 0;
		file_len = 0;
		fmt = 0;
		NI1 = 0;
		format_type = 0;
		Channels = 0;
		frequency = 0;
		frequency = 0;
		trans_speed = 0;
		dataBlock = 0;
		sample_bits = 0;
		data = 0;
		wav_len = 0;
		wav_data = NULL;
	}
};

//FACT块的检测
struct UNKNOW_BLOCK{
	DWORD ID;  // 未知块 
	DWORD Size; // 未知块长度
};
struct FACT_BLOCK{
	DWORD FactID;   // 可选部分标识 0x74636166 'f','a','c','t' 
	DWORD FactSize; // 可选部分长度
	BYTE  Data[1];  // 可选部分数据 
};
class WavFile
{
public:
	WavFile();
	~WavFile();
public:
	bool OpenWavFile(std::string audio_file);
	char* GetVioceData();
	DWORD GetChannels();
	DWORD GetFrequency();
	DWORD GetNI1();
	DWORD GetSample_bits();
	DWORD GetWav_len();
	bool SavePCMData(std::string file_name);
	bool SavePCMData(const char * file_name);
	size_t GetAudioHeadLen();
	static bool PCMTranceWav(const char* file_name, const char* outfile_name = "test.wav",  WORD channels = 2, DWORD sample_bits = 16, DWORD frequncy = 44100);
	static bool PCMTranceWav(const char* file_name, WORD channels = 2, DWORD sample_bits = 16, DWORD frequncy = 44100, const char* outfile_name = "test.wav");
	void close();
protected:
	static FILE_STAT Check_File_Stata(std::ios& fs);
private:
	bool m_file_open;
	HeaderType HT;
	size_t m_sAudioLen;
};

