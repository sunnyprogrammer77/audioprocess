#include "stdafx.h"
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
//---------------------------------------------------------------------------------------------------
bool CNoiseSuppression::NoiseSuppressionProcessT(char *pData, size_t tSize)
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
/*-------------------------------------------------------------------------------------------------
****32KHZ浮点降噪
****由于32KHZ采样率数据需要分频因此特殊处理
****数据采用每次处理10ms数据
-------------*/
bool CNoiseSuppression::NoiseSuppressionProcess32K(char *pData, size_t tSize)
{
	bool bStatus = true;//标志位，暂时没有用处
	size_t frameSize = m_dwSample / 100;//10ms采样数据
	size_t len = frameSize*sizeof(short);
	for (size_t index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{
			WebRtcSpl_AnalysisQMF((short *)(pData + index), 320, shInL, shInH, filter_state1, filter_state12);
			//将需要降噪的数据以高频和低频传入对应接口，同时需要注意返回数据也是分高频和低频
			if (0 == WebRtcNs_Process(pNS_inst, shInL, shInH, shOutL, shOutH))
			{
				//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据填充数据块
				WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, (short*)(pData + index), Synthesis_state1, Synthesis_state12);
			}
		}
	}
	return bStatus;
}

//32KHZ定点降噪
bool CNoiseSuppression::NoiseSuppressionProcessX32K(char *pData, size_t tSize)
{
	bool bStatus = true;
	int frameSize = m_dwSample / 100;
	size_t len = frameSize*sizeof(short);
	for (size_t index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{		
			WebRtcSpl_AnalysisQMF((short *)(pData+index), 320, shInL, shInH, filter_state1, filter_state12);
			//将需要降噪的数据以高频和低频传入对应接口，同时需要注意返回数据也是分高频和低频
			if (0 == WebRtcNsx_Process(pNSX_inst, shInL, shInH, shOutL, shOutH))
			{
				//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据写入文件
				WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, (short*)(pData+index), Synthesis_state1, Synthesis_state12);
			}
		}
	}
	return bStatus;
}


//非32KHZ定点降噪
bool CNoiseSuppression::NoiseSuppressionProcessNoX32K(char *pData, size_t tSize)
{
	bool bStatus = true;//标志位，暂时没有用处
	int frameSize = m_dwSample / 100;
	size_t len = frameSize*sizeof(short);
	for (size_t index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{

			if (0 == WebRtcNsx_Process(pNSX_inst, (short *)(pData + index), NULL, shOutData, NULL))
			{
				//如果降噪成功，则根据降噪后高频和低频数据传入滤波接口，然后用将返回的数据写入数据块

				memcpy((short *)(pData + index), shOutData, len);

			}
		}
	}
	return bStatus;
}

//非32KHZ浮点降噪
bool CNoiseSuppression::NoiseSuppressionProcessNo32K(char *pData, size_t tSize)
{
	bool bStatus = true;////标志位，暂时没有用处
	int frameSize = m_dwSample / 100;//不管原数据多长每次仅取10ms数据处理
	size_t len = frameSize*sizeof(short);
	for (size_t index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{
			if (0 == WebRtcNs_Process(pNS_inst, (short *)(pData + index), NULL, shOutData, NULL))
			{
				memcpy((short *)(pData + index), shOutData, len);
			}
		}
	}
	return bStatus;
}

//--------初始化数据-----------------------------------------------------------------
bool CNoiseSuppression::InitNoiseSuppression(DWORD dwSample, int nMode, bool bStyle)
{
	//-----------公有部分数据------------------
	pNSX_inst = NULL;//定点
	pNS_inst = NULL;//浮点
	m_dwSample = dwSample;
	m_nMode = nMode;//有0,1,2,3,4物种模式，数字越大降噪效果越好，但是会降低音质
	m_bStyle = bStyle;//bStyle:true代表定点降噪，false代表浮点降噪
	if (m_dwSample != 8000 && m_dwSample != 16000 && m_dwSample != 32000)
		return false;
	SetStyle();

	//---------------定点---------------------
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
	//----------------浮点---------------------
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

	//用于32KHZ分频参数的设置
	memset(filter_state1, 0, sizeof(filter_state1));
	memset(filter_state12, 0, sizeof(filter_state12));
	memset(Synthesis_state1, 0, sizeof(Synthesis_state1));
	memset(Synthesis_state12, 0, sizeof(Synthesis_state12));
	shInL = new short[160];
	shInH = new short[160];
	shOutL = new short[160];
	shOutH = new short[160];

	//非32KHZ需要的参数设置
	shOutData = new short[dwSample / 100];
	return true;
}

/*------------------------------------------
****通过采样率以及降噪的方式确定调用的函数
------------------------------------------*/
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