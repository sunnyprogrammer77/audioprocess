#include "stdafx.h"
#include "AudioGain.h"
#pragma warning( disable : 4996)
CAudioGain::CAudioGain()
{
	m_nGainStr = 20;
	m_agcMode = AgcModeFixedDigital;
	agcHandle = NULL;
}
CAudioGain::~CAudioGain()
{
	free(shOutL);
	free(shOutH);
	free(shInL);
	free(shInH);
	free(pOutData);
	shOutL = NULL;
	shOutH = NULL;
	shInL = NULL;
	shInH = NULL;
	pOutData = NULL;
	WebRtcAgc_Free(agcHandle);
}
//-----------------------���ݿ�����-------------------------------------------------//
bool CAudioGain::AudioFileGain(char* pData, size_t tSize)
{
	if (m_dwSamples == 32000)
		return AudioFileGain32K(pData, tSize);
	if (m_dwSamples == 16000 || m_dwSamples == 8000)
		return AudioFileGainNo32K(pData, tSize);   
	return false;
}

//----------------------------��32KHZ���ݴ���--------------------------------------//
bool CAudioGain::AudioFileGainNo32K(char *pdata, size_t tSize)
{
	
	bool status = true;
    int frameSize = m_dwSamples / 100;
	size_t len = frameSize*sizeof(short);
	inMicLevel = micLevelOut;
	outMicLevel = 0;
	int16_t echo = 0;//����Ŵ��Ƿ��ǻ���Ӱ��
	for (size_t i = 0; i < tSize; i += len)
	{
		if (tSize - i >= len)
		{
			inMicLevel = micLevelOut;
			outMicLevel = 0;
			uint8_t saturationWarning; //�Ƿ����������������Ŵ��Ժ�����ֵ������65536
			int nAgcRet = WebRtcAgc_Process(agcHandle, (short*)(pdata + i), NULL, frameSize, pOutData, NULL, inMicLevel, &outMicLevel, echo, &saturationWarning);
			if (nAgcRet != 0)
			{
				status = false;
				break;
			}
			memcpy(pdata+i, pOutData, len);
			micLevelIn = outMicLevel;
		}
	}
	return status;
}

//------------------------32k��������32K��������Ҫ�����˲���Ƶ-----------------//
bool CAudioGain::AudioFileGain32K(char *pData, size_t sLength)
{

	bool status = true;
	int frameSize = m_dwSamples / 100;//10ms������
	size_t len = frameSize*sizeof(short);//������ֽ���
	inMicLevel = micLevelOut;
	outMicLevel = 0;
	int16_t echo = 0;//����Ŵ��Ƿ��ǻ���Ӱ��
	for (size_t index = 0; index < sLength; index += len)
	{
		if (sLength - index >= len)
		{
			//������Ҫʹ���˲���������Ƶ���ݷָߵ�Ƶ���Ը�Ƶ�͵�Ƶ�ķ�ʽ���뽵�뺯���ڲ�
			WebRtcSpl_AnalysisQMF((short*)(pData + index), 320, shInL, shInH, filter_state1, filter_state12);
			uint8_t saturationWarning; //�Ƿ����������������Ŵ��Ժ�����ֵ������65536
			int nAgcRet = WebRtcAgc_Process(agcHandle, shInL, shOutH, frameSize, shOutL, shOutH, inMicLevel, &outMicLevel, echo, &saturationWarning);
			if (nAgcRet != 0)
			{
				status = false;
				break;
			}
			micLevelIn = outMicLevel;

			WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, (short *)(pData + index), Synthesis_state1, Synthesis_state12);
		}
	}
	return status;
}

//----------------------------�������ݳ�ʼ��------------------------------------//
bool CAudioGain::InitAudioGain(DWORD dwSample, AgcMode agcMode, int nGainStr)
{

	m_nGainStr = nGainStr;
	m_agcMode = agcMode;
	m_dwSamples = dwSample;

	WebRtcAgc_Create(&agcHandle);
	int minLevel = 0;
	int maxLevel = 255;
	WebRtcAgc_Init(agcHandle, minLevel, maxLevel, m_agcMode, m_dwSamples);

	WebRtcAgc_config_t agcConfig;
	agcConfig.compressionGaindB = m_nGainStr;//��kAgcModeFixedDigitalģʽ�£�Խ������Խ��;
	agcConfig.limiterEnable = 1;//Ĭ������1���У���һ������;
	agcConfig.targetLevelDbfs = 3;//0��ʾfull scale��ԽС����Խ�� ��Խ������ԽС�������߲��Է������ֻ�����õ�30;
	WebRtcAgc_set_config(agcHandle, agcConfig);

	//��Ƶ���ݳ�ʼ��
	memset(filter_state1, 0, sizeof(filter_state1));
	memset(filter_state12, 0, sizeof(filter_state12));
	memset(Synthesis_state1, 0, sizeof(Synthesis_state1));
	memset(Synthesis_state12, 0, sizeof(Synthesis_state12));
	shOutL = new short[160],shOutH = new short[160];
	shInL = new short[160], shInH = new short[160];

	//��32KHZ����
	pOutData = new short[m_dwSamples / 100];//10ms��������

	//��ʱ��֪����������ʲô��
	micLevelIn = 0;
	micLevelOut = 50;
	inMicLevel = 0;
	outMicLevel = 0;

	return true;
}

//---------------------------�޸�����ǿ��,�������岻����ʱ����-------------------------//
void CAudioGain::SetGainStr(int gainstr)
{
	m_nGainStr = gainstr;
}

//--------------------------�޸�����ģʽ, �������岻����ʱ����----------------------//
void CAudioGain::SetAgcMode(AgcMode agcmode)
{
	m_agcMode = agcmode;
}
