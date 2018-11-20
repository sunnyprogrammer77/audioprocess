#pragma once
/*
2018-11-14
可以对多段数据进行播放
使用close()函数后音频任然会播放,暂无方法
不支持多线程使用
*/
#include <stdio.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <Endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <Audiopolicy.h>
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
#include "../AudioPlayer/NoiseSuppression.h"
#include "../AudioPlayer/AudioGain.h"
//-----------第二种方法所需参数定义---------------------
#define BLOCK_SIZE 6400
#define BLOCK_COUNT 200
static CRITICAL_SECTION waveCriticalSection;//多线程参数暂时不使用
struct MWAVE_FORMAT
{
	WORD m_wFormatTag;
	WORD m_wChannels;        //通道数
	DWORD m_dwSamplesRate;   //采样率
	DWORD m_dwAvgBytesPerSec;  //每秒的音频的 BYTE
	WORD m_wBlockAlign;        //每次采样的 BYTE (16 / 8)*nChannel
	WORD m_wBitsPerSample;     //每次采样的 bit  16*nChannel
};
class CAudioPlayer
{
private:
	CAudioPlayer();
	~CAudioPlayer();
//----------------------------------------------------------------------------------------------------------------
public:
	static CAudioPlayer* Instance();
	bool SetAudioFormat(WORD channel, DWORD sampleRate, WORD bitsPerSample);//设置读取数据块的音频参数
private:
	bool m_bSetAudioFormat;//是否设置音频文件的格式参数
	MWAVE_FORMAT m_wWaveFormat;//保存音频参数
//----------------------------------------------------------------------------------------------------------------
public:
	bool AudioStartPlayer(const char* filename);//通过文件名直接播放音乐	
	//第一种方法对数据块整体读取
	bool AudioStartPalyer(const char* fildata, size_t filesize);//对数据块进行音频播放
private:
	//--------第一种方法所需参数

//---------------------------------------------------------------------------------------------------------------
public:
	//---------第二种方法采用数据缓存方法对大数据快切成小数据块
	static void CALLBACK waveOutProc(HWAVEOUT hWaveOut,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);//回调函数
	bool InitData(WORD channel, DWORD sampleRate, WORD bitsPerSample, size_t  sBlockSize = BLOCK_SIZE, size_t sBlockCount = BLOCK_COUNT);
	void InputData(const char* fileData, size_t size);
	void close();
private:
	WAVEHDR* allocateBlocks(int size, int count);//创建数据缓存
	void freeBlocks(WAVEHDR* blockArray);//清空缓存
	void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);//数据插入
//-------------------------------------------------------------------------------------------------------------
private:
	//--------第二种方法所需参数
	//static CRITICAL_SECTION waveCriticalSection;
	WAVEHDR* waveBlocks;
	volatile int waveFreeBlockCount;
	int waveCurrentBlock;
	size_t m_sBlockSize, m_sBlockCount;
	HWAVEOUT hWaveOut;//很重要的数据
private:
	//-----------降噪增益数据成员
	CNoiseSuppression cns;
	CAudioGain      cap;
private:
	static CAudioPlayer* m_sInstnce;
	

};

