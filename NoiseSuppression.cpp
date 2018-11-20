#include "NoiseSuppression.h"
CNoiseSuppression::CNoiseSuppression()
{

}
CNoiseSuppression::~CNoiseSuppression()
{
	free(shOutData);
	free(shInL);
	free(shOutL);
	free(shInH);
	free(shOutH);
	WebRtcNs_Free(pNS_inst);
	WebRtcNsx_Free(pNSX_inst);
}
////////---------------------------------------------------------------------------------------------------------------------------------
bool CNoiseSuppression::NoiseSuppressionProcessT(char *pData, size_t tSize)//bStyle:true代表定点降噪，false代表浮点降噪
{
	switch (m_fsStyle)
	{
	case SAMPLE_RATE_32K:
		NoiseSuppressionProcess32K(pData, tSize);
		break;
	case SAMPLE_RATE_32K_X:
		NoiseSuppressionProcessX32K(pData, tSize);
		break;
	case SAMPLE_RATE_NONE_32K:
		NoiseSuppressionProcessNo32K(pData, tSize);
		break;
	case SAMPLE_RATE_NONE_32K_X:
		NoiseSuppressionProcessNoX32K(pData, tSize);
		break;
	default:
		return false;
	}
	return true;
}
bool CNoiseSuppression::NoiseSuppressionProcess32K(char *pData, size_t tSize)
{
	bool bStatus = true;
	int frameSize = m_dwSample / 100;
	int len = frameSize*sizeof(short);
	for (int index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{
			WebRtcSpl_AnalysisQMF((short *)(pData + index), 320, shInL, shInH, filter_state1, filter_state12);
			//将需要降噪的数据以高频和低频传入对应接口，同时需要注意返回数据也是分高频和低频
			if (0 == WebRtcNs_Process(pNS_inst, shInL, shInH, shOutL, shOutH))
			{
				short shBufferOut[320];
				//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据写入文件
				WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, (short*)(pData + index), Synthesis_state1, Synthesis_state12);
			}
		}
	}
	return bStatus;
}
bool CNoiseSuppression::NoiseSuppressionProcessX32K(char *pData, size_t tSize)
{
	bool bStatus = true;
	int frameSize = m_dwSample / 100;
	int len = frameSize*sizeof(short);
	for (int index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{		
			WebRtcSpl_AnalysisQMF((short *)(pData+index), 320, shInL, shInH, filter_state1, filter_state12);
			//将需要降噪的数据以高频和低频传入对应接口，同时需要注意返回数据也是分高频和低频
			if (0 == WebRtcNsx_Process(pNSX_inst, shInL, shInH, shOutL, shOutH))
			{
				short shBufferOut[320];
				//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据写入文件
				WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, (short*)(pData+index), Synthesis_state1, Synthesis_state12);
			}
		}
	}
	return bStatus;
}



bool CNoiseSuppression::NoiseSuppressionProcessNoX32K(char *pData, size_t tSize)
{
	bool bStatus = true;
	int frameSize = m_dwSample / 100;
	int len = frameSize*sizeof(short);
	for (int index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{

			if (0 == WebRtcNsx_Process(pNSX_inst, (short *)(pData + index), NULL, shOutData, NULL))
			{
				//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据写入文件

				memcpy((short *)(pData + index), shOutData, len);

			}
		}
	}
	return bStatus;
}
bool CNoiseSuppression::NoiseSuppressionProcessNo32K(char *pData, size_t tSize)
{
	bool bStatus = true;
	int frameSize = m_dwSample / 100;
	int len = frameSize*sizeof(short);
	for (int index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{

			if (0 == WebRtcNs_Process(pNS_inst, (short *)(pData + index), NULL, shOutData, NULL))
			{
				//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据写入文件

				memcpy((short *)(pData + index), shOutData, len);

			}
		}
	}
	return bStatus;
}
bool CNoiseSuppression::InitNoiseSuppression(DWORD dwSample, int nMode, bool bStyle)
{
	pNSX_inst = NULL;
	pNS_inst = NULL;
	m_dwSample = dwSample;
	m_nMode = nMode;
	m_bStyle = bStyle;
	if (m_dwSample != 8000 && m_dwSample != 16000 && m_dwSample != 32000)
		return false;
	SetStyle();
	if (0 != WebRtcNsx_Create(&pNSX_inst))
	{
		return false;
	}

	if (0 != WebRtcNsx_Init(pNSX_inst,dwSample))
	{
		return false;
	}

	if (0 != WebRtcNsx_set_policy(pNSX_inst, nMode))
	{
		return false;
	}
	if (0 != WebRtcNs_Create(&pNS_inst))
	{
		return false;
	}

	if (0 != WebRtcNs_Init(pNS_inst, dwSample))
	{
		return false;
	}

	if (0 != WebRtcNs_set_policy(pNS_inst, nMode))
	{
		return false;
	}
	memset(filter_state1, 0, sizeof(filter_state1));
	memset(filter_state12, 0, sizeof(filter_state12));
	memset(Synthesis_state1, 0, sizeof(Synthesis_state1));
	memset(Synthesis_state12, 0, sizeof(Synthesis_state12));

	shInL = new short[160];
	shInH = new short[160];
	shOutL = new short[160];
	shOutH = new short[160];
	shOutData = new short[dwSample / 100];
	return true;
}
void CNoiseSuppression::SetStyle()
{
	if (m_dwSample == 8000 || m_dwSample == 16000)
	{
		if (m_bStyle)
			m_fsStyle = SAMPLE_RATE_NONE_32K_X;
		else
			m_fsStyle = SAMPLE_RATE_NONE_32K;
	}
	else if (m_dwSample == 32000)
	{
		if (m_bStyle)
			m_fsStyle = SAMPLE_RATE_32K_X;
		else
			m_fsStyle = SAMPLE_RATE_32K;
	}
	else
		m_fsStyle = ILLEGAL_SAMPLE_RATE;
}