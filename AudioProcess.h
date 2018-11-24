#pragma once
/*---------------˵��----------------------
PCM��Ƶ���ݿ����潵�봦��ͷ�ļ�
1 8KHZ, 16KHZ, 32KHZ������, 16λPCM���ݿ�
-----------------------------------------*/
#include "WebRtcHead/NoiseSuppression.h"
#include "WebRtcHead/AudioGain.h"
#define AUDIOSAMPLE  16000                 //��Ƶ������
#define AGCSTR       20                   //��Ƶ����ǿ��
#define AGCMODE      AgcModeAdaptiveAnalog  //����ģʽ  ����
#define NSMODE       2                    //0,1,2,,3,4����Խ�߽���Խ�õ��ή������ 
#define NSSTYLE      false                //���뷽ʽ:true������, false������
class CAudioProcess
{
public:
	CAudioProcess();
	~CAudioProcess();
public:
	bool InitAudioData(DWORD dwSample = AUDIOSAMPLE, AgcMode agcMode = AGCMODE, WORD wAgcStr = AGCSTR, WORD wNSMode = NSMODE, bool wNSStyle = NSSTYLE);
	bool AudioProcessT(char* pData, DWORD dSize);
private:
	CAudioGain m_cAgc;//�������
	CNoiseSuppression m_cNs;//�������
};

