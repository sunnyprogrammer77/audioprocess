#include "WavFile.h"
#include<iostream>
#include<fstream>
#include<string>
//---------------WavFile构造函数-----------------------//
WavFile::WavFile() :m_file_open(false)
{
	m_sAudioLen = 0;
}
//-------------WavFile析构函数----------------------//
WavFile::~WavFile()
{

}

//-------------传入Wav文件提取头文件信息以及PCM数据块-----------------------//
bool WavFile::OpenWavFile(std::string audio_file)
{
	std::ifstream voice_data(audio_file,std::ios::binary);
	if (!voice_data || voice_data.eof())
	{
		printf("Open input voice file %s error!\n", audio_file.c_str());
		return false;
	}
	voice_data.read((char*)(&HT.riff), sizeof(DWORD));
	if (HT.riff != RIFF_SIGN_ID)
	{
		voice_data.close();
		printf("文件标识符错误 ID:%08X", HT.riff);
		return false;
	}
	voice_data.read((char*)(&HT.file_len), sizeof(DWORD));
	voice_data.read((char*)(&HT.wave), sizeof(DWORD));
	if (HT.wave != WAVE_SIGN_ID)
	{
		voice_data.close();
		printf("文件标识符错误 ID:%08X", HT.wave);
		return false;
	}
	voice_data.read((char*)(&HT.fmt), sizeof(DWORD));
	if (HT.fmt != FMT__SIGN_ID)
	{
		voice_data.close();
		printf("格式标识符错误或格式大小错误 ID:%08X", HT.fmt);
		return false;
	}
	voice_data.read((char*)(&HT.NI1), sizeof(DWORD));
	voice_data.read((char*)(&HT.format_type), sizeof(WORD));
	voice_data.read((char*)(&HT.Channels), sizeof(WORD));
	voice_data.read((char*)(&HT.frequency), sizeof(DWORD));
	voice_data.read((char*)(&HT.trans_speed), sizeof(DWORD));
	voice_data.read((char*)(&HT.dataBlock), sizeof(WORD));
	voice_data.read((char*)(&HT.sample_bits), sizeof(WORD));
	voice_data.seekg(20 + HT.NI1, std::ios::beg);
	m_sAudioLen = 28 + HT.NI1;
	while (1)
	{
		UNKNOW_BLOCK ub;
		voice_data.read((char*)(&ub), sizeof(UNKNOW_BLOCK));
		if (ub.ID == FACT_SIGN_ID)
		{
			m_sAudioLen += sizeof(UNKNOW_BLOCK) + ub.Size;
			voice_data.seekg(ub.Size, std::ios::cur);
			printf("Fact 标签 length: %d\n", ub.Size);
		}
		else
			break;
	}
	voice_data.seekg(-((int)sizeof(UNKNOW_BLOCK)), std::ios::cur);
	voice_data.read((char*)(&HT.data), sizeof(DWORD));
	if (HT.data != DATA_SIGN_ID)
	{
		voice_data.close();
		printf("数据错误");
		return false;
	}
	voice_data.read((char*)(&HT.wav_len), sizeof(DWORD));
	std::cout << HT.wav_len<< std::endl;
	HT.wav_data = new char[HT.wav_len];
	if (HT.wav_data == NULL)
	{
		voice_data.close();
		printf("内存申请失败!");
		return false;
	}
	voice_data.read(HT.wav_data, HT.wav_len);
	voice_data.close();
	m_file_open = true;
	return true;
}

//----------------------获得PCM数据块--------------------------//
char * WavFile::GetVioceData()
{
	if (m_file_open)
	{
		return HT.wav_data;
	}
	return NULL;
}

//----------------------获得声道位数-------------------------//
DWORD WavFile::GetChannels()
{
	if (m_file_open)
	{
		return HT.Channels;
	}
	return 0;
}

//---------------------获得采样频率--------------------------//
DWORD WavFile::GetFrequency()
{
	if (m_file_open)
	{
		return HT.frequency;
	}
	return 0;
}

//-------------------------获得采样位数---------------------------//
DWORD WavFile::GetSample_bits()
{
	if (m_file_open)
	{
		return HT.sample_bits;
	}
	return 0;
}

//------------------------获得格式块长度----------------------//
DWORD WavFile::GetNI1()
{
	if (m_file_open)
	{
		return HT.NI1;
	}
	return 0;
}

//------------------------获得音频PCM快的长度-----------------//
DWORD WavFile::GetWav_len()
{
	if (m_file_open)
	{
		return HT.wav_len;
	}
	return 0;
}
//-----------------------保存成PCM文件存储-------------------//
bool WavFile::SavePCMData(std::string file_name)
{
	return SavePCMData(file_name.c_str());
}
bool WavFile::SavePCMData(const char * file_name)
{
	
	if (!m_file_open)
	{
		return false;
	}
	std::ofstream ofs(file_name,std::ios::binary);
	ofs.write(HT.wav_data,HT.wav_len);
	return true;
} 
//------------------------获得wav文件头文件长度--------------//
size_t WavFile::GetAudioHeadLen()
{
	return m_sAudioLen;
}
//------------------------关闭文件------------------------//
void WavFile::close()
{
	if (m_file_open)
	{
		delete[] HT.wav_data;
		HT.wav_data = NULL;
	}
}

//-----------------------PCM文件转化对应的Wav文件输出-----------------------------//
 FILE_STAT WavFile::Check_File_Stata(std::ios& fs)
{
	if (fs.bad())
		return FILE_OPEN_BAD;
	if (fs.fail())
		return FILE_OPEN_FAIL;
	if (fs.eof())
		return FILE_OPEN_EOF;
	return FILE_OPEN_NORMAL;

}
 bool WavFile::PCMTranceWav(const char* file_name, WORD channels, DWORD sample_bits, DWORD frequncy, const char* outfile_name)
 {
	return  PCMTranceWav(file_name, outfile_name,channels, sample_bits, frequncy);
 }
 bool WavFile::PCMTranceWav(const char* file_name, const char* outfile_name, WORD channels, DWORD sample_bits, DWORD frequncy)
{
	HeaderType m_HT;
	m_HT.riff = RIFF_SIGN_ID;
	m_HT.file_len = 0;
	m_HT.wave = WAVE_SIGN_ID;
	m_HT.fmt = FMT__SIGN_ID;
	m_HT.NI1 = 16;
	m_HT.format_type = 1;
	m_HT.Channels = channels;
	m_HT.frequency = frequncy;
	m_HT.sample_bits = sample_bits;
	m_HT.trans_speed = m_HT.Channels*m_HT.sample_bits *frequncy / 8;
	m_HT.dataBlock = m_HT.Channels*m_HT.sample_bits / 8;
	m_HT.data = DATA_SIGN_ID;
	std::ifstream ifs(file_name, std::ios::binary);
	//PCM文件打开
	if (WavFile::Check_File_Stata(ifs) != FILE_OPEN_NORMAL)
	{
		printf("PCM文件打开过程中有错误信息!");
		ifs.close();
		return false;
	}
	//Wav文件打开
	std::ofstream ofs(outfile_name, std::ios::binary);
	FILE_STAT out_file_err = Check_File_Stata(ofs);
	if (out_file_err == FILE_OPEN_BAD || out_file_err == FILE_OPEN_FAIL)
	{
		printf("WAV文件打开过程中有错误信息!");
		ofs.close();
		return false;
	}
	int nLength = 1000;//	数据读取固定长度;	
	char * temp_data = new char[nLength];
	if (temp_data == NULL)
	{
		printf("内存空间申请失败!");
		return false;
	}
	int nLen = nLength;
	int sumlength = 0;
	int h_file_size = 44;
	ofs.seekp(h_file_size,std::ios::beg);
	while (!ifs.eof())
	{
		ifs.read(temp_data, nLength);
		nLen = ifs.gcount();
		sumlength += nLen;
		ofs.write(temp_data, nLen);
	}
	//printf("%d", sumlength);
	ofs.flush();
	m_HT.wav_len = sumlength;
	m_HT.file_len = sumlength + 44 - 8;
	ofs.seekp(0, std::ios::beg);
	ofs.write((char*)&m_HT, h_file_size);
	ifs.close();
	ofs.close();
	return true;
}