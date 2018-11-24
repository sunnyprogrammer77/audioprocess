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
	m_cAgc.InitAudioGain(dwSample, agcMode, wAgcStr);//��Ƶ��ǿ
	m_cNs.InitNoiseSuppression(dwSample, wNSMode, wNSStyle);//��Ƶ����
	return true;
}
bool  CAudioProcess::AudioProcessT(char* pData, DWORD dwSize)
{

	if(!m_cNs.NoiseSuppressionProcessT(pData, dwSize))//��Ƶ����
		return false;
	if (!m_cAgc.AudioFileGain(pData, dwSize))//��Ƶ����
		return false;
	return true;
	
}