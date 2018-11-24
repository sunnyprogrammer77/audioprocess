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
//-----------------------数据块增益-------------------------------------------------//
bool CAudioGain::AudioFileGain(char* pData, size_t tSize)
{
	if (m_dwSamples == 32000)
		return AudioFileGain32K(pData, tSize);
	if (m_dwSamples == 16000 || m_dwSamples == 8000)
		return AudioFileGainNo32K(pData, tSize);   
	return false;
}

//----------------------------非32KHZ数据处理--------------------------------------//
bool CAudioGain::AudioFileGainNo32K(char *pdata, size_t tSize)
{
	
	bool status = true;
    int frameSize = m_dwSamples / 100;
	size_t len = frameSize*sizeof(short);
	inMicLevel = micLevelOut;
	outMicLevel = 0;
	int16_t echo = 0;//增益放大是否考虑回声影响
	for (size_t i = 0; i < tSize; i += len)
	{
		if (tSize - i >= len)
		{
			inMicLevel = micLevelOut;
			outMicLevel = 0;
			uint8_t saturationWarning; //是否有溢出发生，增益放大以后的最大值超过了65536
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

//------------------------32k采样处理32K采样率需要采用滤波分频-----------------//
bool CAudioGain::AudioFileGain32K(char *pData, size_t sLength)
{

	bool status = true;
	int frameSize = m_dwSamples / 100;//10ms采样数
	size_t len = frameSize*sizeof(short);//处理的字节数
	inMicLevel = micLevelOut;
	outMicLevel = 0;
	int16_t echo = 0;//增益放大是否考虑回声影响
	for (size_t index = 0; index < sLength; index += len)
	{
		if (sLength - index >= len)
		{
			//首先需要使用滤波函数将音频数据分高低频，以高频和低频的方式传入降噪函数内部
			WebRtcSpl_AnalysisQMF((short*)(pData + index), 320, shInL, shInH, filter_state1, filter_state12);
			uint8_t saturationWarning; //是否有溢出发生，增益放大以后的最大值超过了65536
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

//----------------------------部分数据初始化------------------------------------//
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
	agcConfig.compressionGaindB = m_nGainStr;//在kAgcModeFixedDigital模式下，越大声音越大;
	agcConfig.limiterEnable = 1;//默认设置1就行，是一个开关;
	agcConfig.targetLevelDbfs = 3;//0表示full scale，越小声音越大 ，越大声音越小，（笔者测试发现最大只能设置到30;
	WebRtcAgc_set_config(agcHandle, agcConfig);

	//分频数据初始化
	memset(filter_state1, 0, sizeof(filter_state1));
	memset(filter_state12, 0, sizeof(filter_state12));
	memset(Synthesis_state1, 0, sizeof(Synthesis_state1));
	memset(Synthesis_state12, 0, sizeof(Synthesis_state12));
	shOutL = new short[160],shOutH = new short[160];
	shInL = new short[160], shInH = new short[160];

	//非32KHZ数据
	pOutData = new short[m_dwSamples / 100];//10ms采样数据

	//暂时不知道这数据有什么用
	micLevelIn = 0;
	micLevelOut = 50;
	inMicLevel = 0;
	outMicLevel = 0;

	return true;
}

//---------------------------修改增益强度,但是意义不大暂时保留-------------------------//
void CAudioGain::SetGainStr(int gainstr)
{
	m_nGainStr = gainstr;
}

//--------------------------修改增益模式, 但是意义不大暂时保留----------------------//
void CAudioGain::SetAgcMode(AgcMode agcmode)
{
	m_agcMode = agcmode;
}
