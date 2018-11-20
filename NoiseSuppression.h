#pragma once
#pragma warning( disable : 4996)
//----------------------˵��--------------------

//���ݱ��뱣֤��8000,16000,32000Ƶ�ʵ�������PCM����

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
//-------------------------------------------���ݿ鴦��----------------------------//
	
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
	int m_nMode;// mode:4��ģʽ�ֱ��Ӧ��0 / 1 / 2 / 3����ֵԽ�ߣ�Ч��Խ���ԣ�������ʧ����һ��2�Ϻ�
	bool m_bStyle;//���㻹�Ǹ��㽵��
	FUNCSTYLE m_fsStyle;
	NsxHandle *pNSX_inst;
	NsHandle  *pNS_inst;
	short *shOutData;//
	short *shInL, *shInH, *shOutL, *shOutH;


	int  filter_state1[6], filter_state12[6];
	int  Synthesis_state1[6], Synthesis_state12[6];

};

