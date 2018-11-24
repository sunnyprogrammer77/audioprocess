#pragma once
#pragma warning( disable : 4996)
/*----------------------˵��--------------------
ͨ�����û���������ݿ���н��봦��
1���ݱ��뱣֤��8000,16000,32000Ƶ�ʵ�����16λ��PCM����
2���ݿ�Ĵ�С������֤�����µı���,�������ֲ��������޷�����
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
enum FUNCSTYLE//���봦����ѡ��
{
	ILLEGAL_SAMPLE_RATE,//�����ʲ��Ϸ�
	SAMPLE_RATE_32K,//32KHZ���㽵��
	SAMPLE_RATE_32K_X,//32KHZ���㽵��
	SAMPLE_RATE_NONE_32K,//��32KHZ���㽵��
	SAMPLE_RATE_NONE_32K_X//��32KHZ���㽵��
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
	FUNCSTYLE m_fsStyle;//�����ʵͨ��ǰ����ж����ô������ݵ����ĸ�����

	//��32KHZ�õĲ���
	NsxHandle *pNSX_inst;
	short *shOutData;
	
	//��Ƶ��Ҫ�õ��ı���
	NsHandle  *pNS_inst;  
	short *shInL, *shInH, *shOutL, *shOutH;
	int  filter_state1[6], filter_state12[6];
	int  Synthesis_state1[6], Synthesis_state12[6];

};

