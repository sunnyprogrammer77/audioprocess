#pragma once
/*---------------说明----------------------
PCM音频数据块增益降噪处理头文件
1 8KHZ, 16KHZ, 32KHZ采样率, 16位PCM数据块
-----------------------------------------*/
#include "WebRtcHead/NoiseSuppression.h"
#include "WebRtcHead/AudioGain.h"
#define AUDIOSAMPLE  16000                 //音频采样率
#define AGCSTR       20                   //音频增益强度
#define AGCMODE      AgcModeAdaptiveAnalog  //增益模式  三种
#define NSMODE       2                    //0,1,2,,3,4数字越高降噪越好但会降低音质 
#define NSSTYLE      false                //降噪方式:true代表定点, false代表浮点
class CAudioProcess
{
public:
	CAudioProcess();
	~CAudioProcess();
public:
	bool InitAudioData(DWORD dwSample = AUDIOSAMPLE, AgcMode agcMode = AGCMODE, WORD wAgcStr = AGCSTR, WORD wNSMode = NSMODE, bool wNSStyle = NSSTYLE);
	bool AudioProcessT(char* pData, DWORD dSize);
private:
	CAudioGain m_cAgc;//增益对象
	CNoiseSuppression m_cNs;//降噪对象
};

