#pragma once
#pragma warning( disable : 4996)
//----------------------说明--------------------

//数据必须保证是8000,16000,32000频率单声道的PCM数据

//----------------------------------------------
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "common/WebRtcMoudle/signal_processing_library.h"
#include "common/WebRtcMoudle/noise_suppression_x.h"
#include "common/WebRtcMoudle/noise_suppression.h"
#include "common/WebRtcMoudle/gain_control.h"
enum FUNCSTYLE
{
	ILLEGAL_SAMPLE_RATE,
	SAMPLE_RATE_32K,
	SAMPLE_RATE_32K_X,
	SAMPLE_RATE_NONE_32K,
	SAMPLE_RATE_NONE_32K_X
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
	FUNCSTYLE m_fsStyle;
	NsxHandle *pNSX_inst;
	NsHandle  *pNS_inst;
	short *shOutData;//
	short *shInL, *shInH, *shOutL, *shOutH;


	int  filter_state1[6], filter_state12[6];
	int  Synthesis_state1[6], Synthesis_state12[6];

};

