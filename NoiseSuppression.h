#pragma once
#pragma warning( disable : 4996)
/*----------------------说明--------------------
通过对用户输入的数据块进行降噪处理
1数据必须保证是8000,16000,32000频率单声道16位的PCM数据
2数据块的大小尽量保证是如下的倍数,否则会出现残留数据无法处理
8KHZ:160
16KHZ:320
32KHZ:640
//--------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "signal_processing_library.h"
#include "noise_suppression_x.h"
#include "noise_suppression.h"
enum FUNCSTYLE//降噪处理函数选择
{
	ILLEGAL_SAMPLE_RATE,//采样率不合法
	SAMPLE_RATE_32K,//32KHZ浮点降噪
	SAMPLE_RATE_32K_X,//32KHZ定点降噪
	SAMPLE_RATE_NONE_32K,//非32KHZ浮点降噪
	SAMPLE_RATE_NONE_32K_X//非32KHZ定点降噪
};
class CNoiseSuppression
{
public:
	CNoiseSuppression();
	~CNoiseSuppression();
//-------------------------------------------数据块处理----------------------------//
	
public:
	bool NoiseSuppressionProcessT(char *pData, size_t tSize);
	bool InitNoiseSuppression(DWORD dwSample, int nMode, bool bStyle = true);
private:
	bool NoiseSuppressionProcessX32K(char *pData, size_t tSize);
	bool NoiseSuppressionProcess32K(char *pData, size_t tSize);
	bool NoiseSuppressionProcessNoX32K(char *pData, size_t tSize);
	bool NoiseSuppressionProcessNo32K(char *pData, size_t tSize);
	void SetStyle();
private:
	DWORD m_dwSample;
	int m_nMode;// mode:4种模式分别对应：0 / 1 / 2 / 3，数值越高，效果越明显，但会损失音质一般2较好
	bool m_bStyle;//定点还是浮点降噪
	FUNCSTYLE m_fsStyle;//这个其实通过前面的判断设置处理数据调用哪个函数

	//非32KHZ用的参数
	NsxHandle *pNSX_inst;
	short *shOutData;
	
	//分频需要用到的变量
	NsHandle  *pNS_inst;  
	short *shInL, *shInH, *shOutL, *shOutH;
	int  filter_state1[6], filter_state12[6];
	int  Synthesis_state1[6], Synthesis_state12[6];

};

