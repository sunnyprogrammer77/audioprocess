#pragma once
/*
2018-11-14
���ԶԶ�����ݽ��в���
ʹ��close()��������Ƶ��Ȼ�Ქ��,���޷���
��֧�ֶ��߳�ʹ��
*/
#include <stdio.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <Endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <Audiopolicy.h>
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
#include "../AudioPlayer/NoiseSuppression.h"
#include "../AudioPlayer/AudioGain.h"
//-----------�ڶ��ַ��������������---------------------
#define BLOCK_SIZE 6400
#define BLOCK_COUNT 200
static CRITICAL_SECTION waveCriticalSection;//���̲߳�����ʱ��ʹ��
struct MWAVE_FORMAT
{
	WORD m_wFormatTag;
	WORD m_wChannels;        //ͨ����
	DWORD m_dwSamplesRate;   //������
	DWORD m_dwAvgBytesPerSec;  //ÿ�����Ƶ�� BYTE
	WORD m_wBlockAlign;        //ÿ�β����� BYTE (16 / 8)*nChannel
	WORD m_wBitsPerSample;     //ÿ�β����� bit  16*nChannel
};
class CAudioPlayer
{
private:
	CAudioPlayer();
	~CAudioPlayer();
//----------------------------------------------------------------------------------------------------------------
public:
	static CAudioPlayer* Instance();
	bool SetAudioFormat(WORD channel, DWORD sampleRate, WORD bitsPerSample);//���ö�ȡ���ݿ����Ƶ����
private:
	bool m_bSetAudioFormat;//�Ƿ�������Ƶ�ļ��ĸ�ʽ����
	MWAVE_FORMAT m_wWaveFormat;//������Ƶ����
//----------------------------------------------------------------------------------------------------------------
public:
	bool AudioStartPlayer(const char* filename);//ͨ���ļ���ֱ�Ӳ�������	
	//��һ�ַ��������ݿ������ȡ
	bool AudioStartPalyer(const char* fildata, size_t filesize);//�����ݿ������Ƶ����
private:
	//--------��һ�ַ����������

//---------------------------------------------------------------------------------------------------------------
public:
	//---------�ڶ��ַ����������ݻ��淽���Դ����ݿ��г�С���ݿ�
	static void CALLBACK waveOutProc(HWAVEOUT hWaveOut,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);//�ص�����
	bool InitData(WORD channel, DWORD sampleRate, WORD bitsPerSample, size_t  sBlockSize = BLOCK_SIZE, size_t sBlockCount = BLOCK_COUNT);
	void InputData(const char* fileData, size_t size);
	void close();
private:
	WAVEHDR* allocateBlocks(int size, int count);//�������ݻ���
	void freeBlocks(WAVEHDR* blockArray);//��ջ���
	void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);//���ݲ���
//-------------------------------------------------------------------------------------------------------------
private:
	//--------�ڶ��ַ����������
	//static CRITICAL_SECTION waveCriticalSection;
	WAVEHDR* waveBlocks;
	volatile int waveFreeBlockCount;
	int waveCurrentBlock;
	size_t m_sBlockSize, m_sBlockCount;
	HWAVEOUT hWaveOut;//����Ҫ������
private:
	//-----------�����������ݳ�Ա
	CNoiseSuppression cns;
	CAudioGain      cap;
private:
	static CAudioPlayer* m_sInstnce;
	

};

