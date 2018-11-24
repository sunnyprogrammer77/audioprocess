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
****32KHZ���㽵��
****����32KHZ������������Ҫ��Ƶ������⴦��
****���ݲ���ÿ�δ���10ms����
-------------*/
bool CNoiseSuppression::NoiseSuppressionProcess32K(char *pData, size_t tSize)
{
	bool bStatus = true;//��־λ����ʱû���ô�
	size_t frameSize = m_dwSample / 100;//10ms��������
	size_t len = frameSize*sizeof(short);
	for (size_t index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{
			WebRtcSpl_AnalysisQMF((short *)(pData + index), 320, shInL, shInH, filter_state1, filter_state12);
			//����Ҫ����������Ը�Ƶ�͵�Ƶ�����Ӧ�ӿڣ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ
			if (0 == WebRtcNs_Process(pNS_inst, shInL, shInH, shOutL, shOutH))
			{
				//�������ɹ�������ݽ�����Ƶ�͵�Ƶ���ݴ����˲��ӿڣ�Ȼ���ý����ص�����������ݿ�
				WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, (short*)(pData + index), Synthesis_state1, Synthesis_state12);
			}
		}
	}
	return bStatus;
}

//32KHZ���㽵��
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
			//����Ҫ����������Ը�Ƶ�͵�Ƶ�����Ӧ�ӿڣ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ
			if (0 == WebRtcNsx_Process(pNSX_inst, shInL, shInH, shOutL, shOutH))
			{
				//�������ɹ�������ݽ�����Ƶ�͵�Ƶ���ݴ����˲��ӿڣ�Ȼ���ý����ص�����д���ļ�
				WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, (short*)(pData+index), Synthesis_state1, Synthesis_state12);
			}
		}
	}
	return bStatus;
}


//��32KHZ���㽵��
bool CNoiseSuppression::NoiseSuppressionProcessNoX32K(char *pData, size_t tSize)
{
	bool bStatus = true;//��־λ����ʱû���ô�
	int frameSize = m_dwSample / 100;
	size_t len = frameSize*sizeof(short);
	for (size_t index = 0; index < tSize; index += len)
	{
		if (tSize - index >= len)
		{

			if (0 == WebRtcNsx_Process(pNSX_inst, (short *)(pData + index), NULL, shOutData, NULL))
			{
				//�������ɹ�������ݽ�����Ƶ�͵�Ƶ���ݴ����˲��ӿڣ�Ȼ���ý����ص�����д�����ݿ�

				memcpy((short *)(pData + index), shOutData, len);

			}
		}
	}
	return bStatus;
}

//��32KHZ���㽵��
bool CNoiseSuppression::NoiseSuppressionProcessNo32K(char *pData, size_t tSize)
{
	bool bStatus = true;////��־λ����ʱû���ô�
	int frameSize = m_dwSample / 100;//����ԭ���ݶ೤ÿ�ν�ȡ10ms���ݴ���
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

//--------��ʼ������-----------------------------------------------------------------
bool CNoiseSuppression::InitNoiseSuppression(DWORD dwSample, int nMode, bool bStyle)
{
	//-----------���в�������------------------
	pNSX_inst = NULL;//����
	pNS_inst = NULL;//����
	m_dwSample = dwSample;
	m_nMode = nMode;//��0,1,2,3,4����ģʽ������Խ����Ч��Խ�ã����ǻή������
	m_bStyle = bStyle;//bStyle:true�����㽵�룬false�����㽵��
	if (m_dwSample != 8000 && m_dwSample != 16000 && m_dwSample != 32000)
		return false;
	SetStyle();

	//---------------����---------------------
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
	//----------------����---------------------
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

	//����32KHZ��Ƶ����������
	memset(filter_state1, 0, sizeof(filter_state1));
	memset(filter_state12, 0, sizeof(filter_state12));
	memset(Synthesis_state1, 0, sizeof(Synthesis_state1));
	memset(Synthesis_state12, 0, sizeof(Synthesis_state12));
	shInL = new short[160];
	shInH = new short[160];
	shOutL = new short[160];
	shOutH = new short[160];

	//��32KHZ��Ҫ�Ĳ�������
	shOutData = new short[dwSample / 100];
	return true;
}

/*------------------------------------------
****ͨ���������Լ�����ķ�ʽȷ�����õĺ���
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