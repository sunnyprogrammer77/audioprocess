#include "AsrAudioTanslate.h"

AsrAudioTanslate::AsrAudioTanslate()
{
	audio_file = NULL;
	m_InterceptLen = 200000;
	m_AcountInfoType = "AccountInfo";

}
AsrAudioTanslate::~AsrAudioTanslate()
{
	if (account_info)
		 account_info->ReleaseInstance();
}
bool AsrAudioTanslate::InitAccountInfo()
{
	string account_info_file;
	account_info = AccountInfo::GetInstance();
	account_info_file = account_info_file+ "testdata/" + m_AcountInfoType + ".txt";
	bool account_success = account_info->LoadFromFile(account_info_file);
	//bool account_success = account_info->LoadFromCode();
	if (!account_success)
	{
		printf("AccountInfo read from %s failed\n", account_info_file.c_str());
		getchar();
		return false;
	}
	printf("AccountInfo Read success\n");
	return true;
}
bool AsrAudioTanslate::HciInit()
{
	HCI_ERR_CODE err_code = HCI_ERR_NONE;
	//配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
	string init_config = "";
	init_config += "appKey=" + account_info->app_key();              //灵云应用序号
	init_config += ",developerKey=" + account_info->developer_key(); //灵云开发者密钥
	init_config += ",cloudUrl=" + account_info->cloud_url();         //灵云云服务的接口地址
	init_config += ",authpath=" + account_info->auth_path();         //授权文件所在路径，保证可写
	init_config += ",logfilepath=" + account_info->logfile_path();   //日志的路径
	init_config += ",loglevel=5";								     //日志的等级
	init_config += ",logfilesize=512";								 //日志文件的大小
	//其他配置使用默认值，不再添加，如果想设置可以参考开发手册
	err_code = hci_init(init_config.c_str());
	if (err_code != HCI_ERR_NONE)
	{
		printf("hci_init return (%d:%s)\n", err_code, hci_get_error_info(err_code));
		getchar();
		return false;
	}
	printf("hci_init success\n");
	return true;
}
bool AsrAudioTanslate::HciAsrInit()
{
	HCI_ERR_CODE err_code = HCI_ERR_NONE;
	asr_init_config += "initcapkeys=" + account_info->cap_key();                    //建议在initcapkeys中添加需要使用的所有能力以提高第一次识别的效率
	asr_init_config += ",datapath=" + account_info->data_path();                    //如果是本地能力，此处指定本地能力依赖资源
	err_code = hci_asr_init(asr_init_config.c_str());
	if (err_code != HCI_ERR_NONE)
	{
		printf("hci_asr_init return (%d:%s) \n", err_code, hci_get_error_info(err_code));
		return false;
	}
	printf("hci_asr_init success \n");
	return true;
}
void AsrAudioTanslate::SetInterceptLen(size_t len)
{
	m_InterceptLen = len;
}
void AsrAudioTanslate::SetAcountInfoTyp(const char* Type)
{
	m_AcountInfoType = Type;
}
void AsrAudioTanslate::CloseAsr()
{
	hci_asr_release();
}
void AsrAudioTanslate::CloseHci()
{
	hci_release();
}
bool AsrAudioTanslate::StartRecog(const char* audiofile,size_t buffer, bool fileflag, const char* outfile)
{
	audio_file = audiofile;
	if (fileflag)
		return Recog(account_info->cap_key(), asr_init_config, audio_file, outfile, buffer);
	else
		return RealtimeRecog(account_info->cap_key(), asr_init_config, audio_file, outfile, buffer);
}
bool AsrAudioTanslate::Recog(const string &cap_key, const string& recog_config, const char* audio_file, const char * file_name, size_t buffer)
{
	HCI_ERR_CODE err_code = HCI_ERR_NONE;
	// 载入语音数据文件
	HciExampleComon::FileReader voiceData;
	if (voiceData.Load(audio_file) == false)
	{
		printf("Open input voice file %s error!\n", audio_file);
		return false;
	}

	// 启动 ASR Session
	int nSessionId = -1;
	string strSessionConfig = "capkey=" + cap_key;
	strSessionConfig += "," + recog_config;
	//此处也可以传入其他配置，参见开发手册，此处其他配置采用默认值
	printf("hci_asr_session_start config[ %s ]\n", strSessionConfig.c_str());
	err_code = hci_asr_session_start(strSessionConfig.c_str(), &nSessionId);
	if (err_code != HCI_ERR_NONE)
	{
		printf("hci_asr_session_start return (%d:%s)\n", err_code, hci_get_error_info(err_code));
		return false;
	}
	printf("hci_asr_session_start success\n");

	// 识别
	ASR_RECOG_RESULT asrResult;
	printf("hci_asr_recog config[ %s ]\n", recog_config.c_str());
	//截取音频文件长度
	int voice_Length = voiceData.buff_len_;//声音文件长度
	int nlen = 0;
	int m_length = 0;
	std::ofstream ofs(file_name);
	if (!ofs)
	{
		printf("文件打开失败!");
		ofs.close();
		return false;
	}
	while (nlen < voice_Length)
	{
		if (voice_Length - nlen > m_InterceptLen)
		{
			m_length = m_InterceptLen;
		}
		else
		{
			m_length = voice_Length - nlen;
		}
		err_code = hci_asr_recog(nSessionId, voiceData.buff_ + buffer+ nlen, m_length, NULL, NULL, &asrResult);//voiceData.buff_len_
		printf("%d", voiceData.buff_len_);

		if (err_code == HCI_ERR_NONE)
		{
			printf("hci_asr_recog success\n");
			// 输出识别结果
			//PrintAsrResult(asrResult);

			HciExampleComon::SetSpecialConsoleTextAttribute();
			printf("\nrecog result:\n");
			for (int index = 0; index < (int)asrResult.uiResultItemCount; ++index)
			{
				ASR_RECOG_RESULT_ITEM& item = asrResult.psResultItemList[index];
				ofs.write(item.pszResult, strlen(item.pszResult));
				ofs.write("\n", strlen("\n"));
				printf("index:%d\t", index);
				printf("text:");
				HciExampleComon::PrintUtf8String(item.pszResult);
				printf("\tscore:%d\n", item.uiScore);
			}
			printf("\n");
			HciExampleComon::SetOriginalConsoleTextAttribute();




			// 释放识别结果
			hci_asr_free_recog_result(&asrResult);
		}
		else
		{
			printf("hci_asr_recog return (%d:%s)\n", err_code, hci_get_error_info(err_code));
			break;
		}
		nlen += m_length;
	}
	ofs.close();
	// 终止 ASR Session
	hci_asr_session_stop(nSessionId);
	printf("hci_asr_session_stop\n");
	return true;
}

bool AsrAudioTanslate::RealtimeRecog(const string &cap_key, const string& recog_config, const char* audio_file, const char* outfile, size_t buffer)
{
	HCI_ERR_CODE errCode = HCI_ERR_NONE;
	// 载入音频数据
	HciExampleComon::FileReader voiceData;
	if (voiceData.Load(audio_file) == false)
	{
		printf("Open input voice file %s error!\n", audio_file);
		return false;
	}

	// 启动 ASR Session
	int nSessionId = -1;
	string strSessionConfig = "vadswitch=no,vadtail=1234,vadseg=5678,realtime=yes,capkey=" + cap_key;
	strSessionConfig += "," + recog_config;
	//此处也可以传入其他配置，参见开发手册，此处其他配置采用默认值
	printf("hci_asr_session_start config[ %s ]\n", strSessionConfig.c_str());
	errCode = hci_asr_session_start(strSessionConfig.c_str(), &nSessionId);
	if (errCode != HCI_ERR_NONE)
	{
		printf("hci_asr_session_start return (%d:%s)\n", errCode, hci_get_error_info(errCode));
		return false;
	}
	printf("hci_asr_session_start success\n");

	// 实时识别过程模拟，将待识别音频数据分为多段，逐段传入识别接口
	// 若某次传输音频数据后检测到末端，则跳出循环，终止音频数据传入以准备获取识别结果
	printf("hci_asr_recog config[ %s ]\n", recog_config.c_str());
	int nPerLen = m_InterceptLen;	// 0.2s
	int nLen = 0;            // 当前已传入的长度
	ASR_RECOG_RESULT asrResult;
	while (nLen < voiceData.buff_len_)
	{
		// 本次要传入的参与识别的数据长度，
		// 剩余的多于nPerLen则传入nPerLen个字节，若不足则传入剩余数据
		int nThisLen = 0;
		if (voiceData.buff_len_ - nLen >= nPerLen)
		{
			nThisLen = nPerLen;
		}
		else
		{
			nThisLen = voiceData.buff_len_ - nLen;
		}

		errCode = hci_asr_recog(nSessionId, voiceData.buff_ +buffer + nLen, nThisLen, NULL, NULL, &asrResult);
		if (asrResult.uiResultItemCount > 0)
		{
			//PrintAsrResult(asrResult);
			hci_asr_free_recog_result(&asrResult);
		}

		if (errCode == HCI_ERR_ASR_REALTIME_END)
		{
			errCode = hci_asr_recog(nSessionId, NULL, 0, NULL, NULL, &asrResult);
			if (errCode == HCI_ERR_NONE)
			{
				printf("hci_asr_recog success\n");
				// 输出识别结果
				//PrintAsrResult(asrResult);
				// 释放识别结果
				hci_asr_free_recog_result(&asrResult);
			}
			else
			{
				printf("hci_asr_recog return (%d:%s)\n", errCode, hci_get_error_info(errCode));
				break;
			}
		}
		else if (errCode == HCI_ERR_ASR_REALTIME_WAITING || errCode == HCI_ERR_ASR_REALTIME_NO_VOICE_INPUT)
		{
			//在连续识别的场景，忽略这两个情况，继续识别后面的音频。
			//HCI_ERR_ASR_REALTIME_WAITING （实时识别等待音频）含义是：还没有数据，或者是需要更多数据。
			//HCI_ERR_ASR_REALTIME_NO_VOICE_INPUT 含义是：没有检测到音频起点，即超过了vadHead的范围（此时可以让设备进入休眠状态）

			printf("hci_asr_recog return (%d:%s)\n", errCode, hci_get_error_info(errCode));

			nLen += nThisLen;
		}
		else
		{
			//识别失败
			printf("hci_asr_recog return (%d	:%s)\n", errCode, hci_get_error_info(errCode));
			break;
		}


		//模拟真实说话人语速，发送200ms数据后需等待200ms
#if defined(WIN32) || defined(WIN64)
		Sleep(200);
#else
		//usleep(200 * 1000);
#endif
	}

	// 若未检测到端点，但数据已经传入完毕，则需要告诉引擎数据输入完毕
	// 或者检测到末端了，也需要告诉引擎，获取结果
	if (errCode == HCI_ERR_ASR_REALTIME_WAITING || errCode == HCI_ERR_ASR_REALTIME_END)
	{
		errCode = hci_asr_recog(nSessionId, NULL, 0, NULL, NULL, &asrResult);
		if (errCode == HCI_ERR_NONE)
		{
			printf("hci_asr_recog success\n");
			// 输出识别结果
			//PrintAsrResult(asrResult);
			// 释放识别结果
			hci_asr_free_recog_result(&asrResult);
		}
		else
		{
			printf("hci_asr_recog return (%d:%s)\n", errCode, hci_get_error_info(errCode));
		}
	}

	// 终止 ASR Session
	hci_asr_session_stop(nSessionId);
	printf("hci_asr_session_stop\n");
	return true;
}