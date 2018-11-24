/*---------------------
˵��:���ݿ��������
1���ݿ���Ҫ��ͨ��16λ����
2���ݿ�Ĳ����ʿ�����8KHZ, 16KHZ, 32KHZ 
3���ݿ�ĳ��Ⱦ����������±����������в��������޷�����
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
enum AgcMode { AgcModeAdaptiveAnalog = 1,           //����ģ���������ڵĹ���
	AgcModeAdaptiveDigital,              //�ɱ�����agc�����ǲ�����ϵͳ����
	AgcModeFixedDigital                //�̶������agc
};
class CAudioGain
{
public:
	CAudioGain();
	~CAudioGain();
public:
//--------------------------------���ݿ�����--------------------------------------------//
	bool InitAudioGain(DWORD dwSample, AgcMode agcMode, int nGainStr);
	void SetGainStr(int gainstr);
	void SetAgcMode(AgcMode agcmode);
	bool AudioFileGain(char* pData, size_t tSize);
private:
	bool AudioFileGainNo32K(char *pdata, size_t tSize);
	bool AudioFileGain32K(char *pData, size_t sLength);
private:
	//-----------------��������-------------------//
	int m_nGainStr;//����ǿ��
	AgcMode m_agcMode;//����ģʽ
	DWORD m_dwSamples;//������
	void *agcHandle;
	//-----------------��Ƶ��������------------------//
	int  filter_state1[6], filter_state12[6];//��Ƶ��Ҫ
	int  Synthesis_state1[6], Synthesis_state12[6];
	short* shOutL, *shOutH;//
	short* shInL, *shInH ;//
   //-----------------��32KHZ�������ʱʹ��
	short *pOutData;
	//�����������ʱû�з�����ʲô��
	int micLevelIn;
	int micLevelOut;
	int inMicLevel;
	int outMicLevel;
};

