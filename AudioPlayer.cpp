
#include "stdafx.h"
#include "AudioPlayer.h"

CAudioPlayer::CAudioPlayer()
{

}
CAudioPlayer::~CAudioPlayer()
{
	if (m_sInstnce)
		delete m_sInstnce;
}
//-------------------------------���ļ����в���---------------------//
bool CAudioPlayer::AudioStartPlayer(const char* filename)
{
	return true;
}
//--------------------------------�������ݿ�--------------------------///
bool CAudioPlayer::SetAudioFormat(WORD channel, DWORD sampleRate, WORD bitsPerSample)
{
	m_wWaveFormat.m_wChannels = channel;
	m_wWaveFormat.m_dwSamplesRate = sampleRate;
	m_wWaveFormat.m_wBitsPerSample = bitsPerSample;
	m_wWaveFormat.m_wBlockAlign = channel * bitsPerSample / 8;
	m_wWaveFormat.m_dwAvgBytesPerSec = m_wWaveFormat.m_wBlockAlign * sampleRate;
	m_bSetAudioFormat = true;
	return true;
}
bool CAudioPlayer::AudioStartPalyer(const char* filedata, size_t filesize)
{
	if (!m_bSetAudioFormat)//û�����ò����ļ��޷�����
		return false;
	WAVEFORMATEX pFormat;
	pFormat.wFormatTag = WAVE_FORMAT_PCM;
	//simple,uncompressed format
	pFormat.nChannels = m_wWaveFormat.m_wChannels;//ͨ����
	pFormat.nSamplesPerSec = m_wWaveFormat.m_dwSamplesRate; // ������
	pFormat.nAvgBytesPerSec = m_wWaveFormat.m_dwAvgBytesPerSec;
	// = nSamplesPerSec * n.Channels * wBitsPerSample/8
	pFormat.nBlockAlign = m_wWaveFormat.m_wBlockAlign; // = n.Channels * wBitsPerSample/8
	pFormat.wBitsPerSample = m_wWaveFormat.m_wBitsPerSample;
	//16 for high quality, 8 for telephone-grade
	pFormat.cbSize = 0;

	HWAVEOUT     hWaveOut;
	WAVEHDR      WaveOutHdr;
	MMRESULT result;
	result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);
	if (result)
	{
		//printf("�豸��ʧ��");
		return false;
	}


	WaveOutHdr.lpData = (LPSTR)filedata;
	WaveOutHdr.dwBufferLength = filesize;
	WaveOutHdr.dwBytesRecorded = 0;
	WaveOutHdr.dwUser = 0L;
	WaveOutHdr.dwFlags = 0L;
	WaveOutHdr.dwLoops = 0L;
	waveOutPrepareHeader(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));

	result = waveOutWrite(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));
	if (result != MMSYSERR_NOERROR)
	{
		//TRACE("Failed to play sound%d", WAVERR_UNPREPARED);
		return false;
	}
	do {} while (waveOutUnprepareHeader(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING);
	waveOutClose(hWaveOut);
	return true;
}

//-------------------------�ڶ��ַ�������ʵ��------------------------------------//
void CALLBACK CAudioPlayer::waveOutProc(//�ص��������ڸ��¿��л���������
	HWAVEOUT hWaveOut,
	UINT uMsg,
	DWORD dwInstance,
	DWORD dwParam1,
	DWORD dwParam2
	)
{
	int* freeBlockCounter = (int*)dwInstance;

	if (uMsg != WOM_DONE)
		return;
	(*freeBlockCounter)++;
}
WAVEHDR* CAudioPlayer::allocateBlocks(int size, int count)
{
	unsigned char* buffer;
	int i;
	WAVEHDR* blocks;
	DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;

	if (!(buffer =new unsigned char[totalBufferSize]))
	{
		//TRACE("Memory allocationerror\n");
		ExitProcess(1);
	}
	memset(buffer, 0, sizeof(char)*totalBufferSize);
	blocks = (WAVEHDR*)buffer;
	buffer += sizeof(WAVEHDR) * count;
	for (i = 0; i < count; i++) {
		blocks[i].dwBufferLength = size;
		blocks[i].lpData = (LPSTR)buffer;
		buffer += size;
	}
	return blocks;
}
void CAudioPlayer::freeBlocks(WAVEHDR* blockArray)
{

	delete[] blockArray;
}
void CAudioPlayer::writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{

	WAVEHDR* current;
	int remain;
	current = &waveBlocks[waveCurrentBlock];
	while (size > 0) {

		if (current->dwFlags & WHDR_PREPARED)
		{
			waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));
		} 
		if (size < (int)(m_sBlockSize - current->dwUser)) {
			memcpy(current->lpData + current->dwUser, data, size);
			current->dwUser += size;
			break;
		}
		remain = m_sBlockSize - current->dwUser;
		memcpy(current->lpData + current->dwUser, data, remain);
		size -= remain;
		data += remain;
		current->dwBufferLength = m_sBlockSize;
		//----------------------------��Ƶ������ǿ����----------------------//
		cap.AudioFileGain(current->lpData, current->dwBufferLength);//��Ƶ����
		cns.NoiseSuppressionProcessT(current->lpData, current->dwBufferLength);//��Ƶ����
		//-----------------------------------------------------------------//
		
		waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));
		waveFreeBlockCount--;
		while (!waveFreeBlockCount)
			Sleep(10);
		waveCurrentBlock++;
		waveCurrentBlock %= m_sBlockCount;
		current = &waveBlocks[waveCurrentBlock];
		current->dwUser = 0;
	}
}
bool CAudioPlayer::InitData(WORD channel, DWORD sampleRate, WORD bitsPerSample, size_t  sBlockSize, size_t sBlockCount )
{
	WAVEFORMATEX wfx;
	m_sBlockCount = sBlockCount;
	m_sBlockSize = sBlockSize;
	m_bSetAudioFormat = false;
	waveBlocks = waveBlocks = allocateBlocks(m_sBlockSize, m_sBlockCount);;

	waveFreeBlockCount = m_sBlockCount;
	waveCurrentBlock = 0;

	//------------������Ƶ��ʽ
	wfx.nSamplesPerSec = sampleRate;
	wfx.wBitsPerSample = bitsPerSample;
	wfx.nChannels = channel;
	wfx.cbSize = 0;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
	//---------��ʼ����Ƶ��ǿ��������-------------
	//cap.InitAudioGain(sampleRate, AgcModeFixedDigital, 20);//��Ƶ��ǿ
	cns.InitNoiseSuppression(sampleRate, 2, false);//��Ƶ����
	//-----------------------
	if (waveOutOpen(
		&hWaveOut,
		WAVE_MAPPER,
		&wfx,
		(DWORD_PTR)waveOutProc,
		(DWORD_PTR)&waveFreeBlockCount,
		CALLBACK_FUNCTION
		) != MMSYSERR_NOERROR)
	{
		//TRACE("unable toopen wave mapper device\n");
		return false;
	}
	return true;
}
void CAudioPlayer::InputData(const char* fileData, size_t size)
{
	writeAudio(hWaveOut, (LPSTR)fileData, size);
}
void CAudioPlayer::close()
{
	freeBlocks(waveBlocks);
	waveOutClose(hWaveOut);
}
CAudioPlayer* CAudioPlayer::m_sInstnce = NULL;
//-------------------------------------------------------------------------------------------------
CAudioPlayer* CAudioPlayer::Instance()
{
	if (m_sInstnce == NULL)
	{
		return new CAudioPlayer();
	}
	return m_sInstnce;
}