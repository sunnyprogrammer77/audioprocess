/*---------------------
说明:数据块进行增益
1数据块需要单通道16位数据
2数据块的采样率可以有8KHZ, 16KHZ, 32KHZ 
3数据块的长度尽量保持如下倍数，否则有残留数据无法处理
8KHZ:160
16KHZ:320
32KHZ:640
*---------------------*/
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "signal_processing_library.h"
#include "gain_control.h"
enum AgcMode { AgcModeAdaptiveAnalog = 1,           //带有模拟音量调节的功能
	AgcModeAdaptiveDigital,              //可变增益agc，但是不调节系统音量
	AgcModeFixedDigital                //固定增益的agc
};
class CAudioGain
{
public:
	CAudioGain();
	~CAudioGain();
public:
//--------------------------------数据块增益--------------------------------------------//
	bool InitAudioGain(DWORD dwSample, AgcMode agcMode, int nGainStr);
	void SetGainStr(int gainstr);
	void SetAgcMode(AgcMode agcmode);
	bool AudioFileGain(char* pData, size_t tSize);
private:
	bool AudioFileGainNo32K(char *pdata, size_t tSize);
	bool AudioFileGain32K(char *pData, size_t sLength);
private:
	//-----------------公用数据-------------------//
	int m_nGainStr;//增益强度
	AgcMode m_agcMode;//三种模式
	DWORD m_dwSamples;//采样率
	void *agcHandle;
	//-----------------分频参数设置------------------//
	int  filter_state1[6], filter_state12[6];//分频需要
	int  Synthesis_state1[6], Synthesis_state12[6];
	short* shOutL, *shOutH;//
	short* shInL, *shInH ;//
   //-----------------非32KHZ输出数据时使用
	short *pOutData;
	//下面的数据暂时没有发现有什么用
	int micLevelIn;
	int micLevelOut;
	int inMicLevel;
	int outMicLevel;
};

