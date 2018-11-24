#include "stdafx.h"
#include "AudioProcess.h"
CAudioProcess::CAudioProcess()
{
}
CAudioProcess::~CAudioProcess()
{
}
bool CAudioProcess::InitAudioData(DWORD dwSample, AgcMode agcMode, WORD wAgcStr, WORD wNSMode, bool wNSStyle)
{
	m_cAgc.InitAudioGain(dwSample, agcMode, wAgcStr);//“Ù∆µ‘ˆ«ø
	m_cNs.InitNoiseSuppression(dwSample, wNSMode, wNSStyle);//“Ù∆µΩµ‘Î
	return true;
}
bool  CAudioProcess::AudioProcessT(char* pData, DWORD dwSize)
{

	if(!m_cNs.NoiseSuppressionProcessT(pData, dwSize))//“Ù∆µΩµ‘Î
		return false;
	if (!m_cAgc.AudioFileGain(pData, dwSize))//“Ù∆µ‘ˆ“Ê
		return false;
	return true;
	
}