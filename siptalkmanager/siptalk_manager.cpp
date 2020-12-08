#include "siptalk_manager.h"
#include "shared/util.h"

namespace nim_comp
{
	
	SipTalkManager::SipTalkManager()
	{
		sip_wnd_ = NULL;
		sip_status_ = sip_status_default;
		notify_ = nullptr;
		lasttimestamp_ = 0;
		bInit_ = false;
		hasAudio_ = 0;
	}

	SipTalkManager::~SipTalkManager()
	{
		StopSipPlugin();
	}

	bool SipTalkManager::InitEnvironment()
	{
		if (bInit_)
		{
			return true;
		}
		//�½�һ���޽��洰�ڣ����ڽ��̼�ͨ��
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = nbase::win32::GetCurrentModuleHandle();  
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = L"sipTalkManager";
		wcex.hIconSm = NULL;
		RegisterClassEx(&wcex);

		//WS_VISIBLE | WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_TOOLWINDOW | WS_EX_TOPMOST

		param_.hwnd_ = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, L"sipTalkManager", L"sipTalkManager", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, wcex.hInstance, NULL);

		if (!param_.hwnd_)
		{
			return false;
		}
		else
		{
			//���ö�ʱ��
			SetTimer(param_.hwnd_, 1, HEART_GAP, NULL);
			bInit_ = true;
			return true;
		}

	}

	void SipTalkManager::AddListener(ISipStatusNotify* notify)
	{
		notify_ = notify;
	}

	void SipTalkManager::RemoveListener(ISipStatusNotify* notify)
	{
		assert(notify_ == notify); //ĿǰӦ������ͬ��
		if (notify_ == notify)
		{
			notify_ = nullptr;
		}
	}

	bool SipTalkManager::StartSipPlugin(const std::string& server, const std::string &id, const std::string &user,
		const std::string& pwd, const std::string& stun, SipProtocol p/*=sip_p_udp*/)
	{
		QLOG_APP(L"StartSipPlugin{0}{1}{2}{3}")<<server<<id<<pwd<<stun;
		if (server.empty() || id.empty() || pwd.empty())
		{
			return false;
		}
		param_.server_ = server;
		param_.id_ = id;
		if (user.empty())
		{
			param_.user_ = id;
		}
		else
		{
			param_.user_ = user;
		}
		param_.pwd_ = pwd;
		param_.p_ = p;
		param_.stun_ = stun;
		return startSipPlugin();
	}

	//Э���ʽ��  { "func":"make_call","param":"id" }
	bool SipTalkManager::MakeCall(const std::string& id)
	{
		//״̬���
		if (sip_status_ != sip_status_online && sip_status_ != sip_status_disconnected)
		{
			return false;
		}

		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//����Э��
		SendCmd(SIP_CMD_CALL,id);
		return true;
	}

	bool SipTalkManager::AnswerCall(const std::string& call_id)
	{
		//״̬���
		if (sip_status_ != sip_status_ringing)
		{
			return false;
		}

		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//����Э��
		SendCmd(SIP_CMD_ANSWER, call_id);
		return true;
	}

	bool SipTalkManager::CancelCall(const std::string& call_id)
	{
		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//����Э��
		SendCmd(SIP_CMD_CANCEL, call_id);
		return true;
	}

	bool SipTalkManager::HangUpCall(const std::string& call_id)
	{
		//״̬���
		if (sip_status_ != sip_status_connected)
		{
			return false;
		}

		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//����Э��
		SendCmd(SIP_CMD_HANGUP, call_id);
		return true;
	}

	bool SipTalkManager::SetQuiet(bool bQuiet)
	{
		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//����Э��
		if (bQuiet)
		{
			SendCmd(SIP_CMD_MUTE, "0.0");
		}
		else
		{
			SendCmd(SIP_CMD_MUTE, "1.0");
		}
		
		return true;
	}

	bool SipTalkManager::SendDtmf(const std::string& number)
	{
		//״̬���
		if (sip_status_ != sip_status_connected)
		{
			return false;
		}

		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//����Э��
		SendCmd(SIP_CMD_DTMF, number);
		return true;
	}

	void SipTalkManager::SetSipWnd(HWND hWnd)
	{
		sip_wnd_ = hWnd;
	}

	void SipTalkManager::SetHasAudio(int hasAudio)
	{
		hasAudio_ = hasAudio;
	}

	int SipTalkManager::GetHasAudio() const
	{
		return hasAudio_;
	}

	//������Լ�����Ƿ񻹴����
	void SipTalkManager::CheckSipPluginAlive()
	{
		static int retry_time = 0;
		if (sip_status_ == sip_status_default)
		{
			return;
		}
		//todo �����������ˣ�������, ����ͨ�����̼�⣬����Լ�����������
		if (nbase::win32::FindProcessId(SIP_PLUGIN_NAME) == -1)
		{
			startSipPlugin();
		}
		//���5����Ȼ��δ���ߣ�������
		if (sip_status_ == sip_status_offline)
		{
			if (retry_time >= 5)
			{
				startSipPlugin();
				retry_time = 0;
			}
			else
			{
				retry_time++;
			}
		}
	}

	void SipTalkManager::SetStatus(SipStatus status, std::string id, SipErrorCode code, int origin)
	{
		if (status == sip_status_online && code == sip_error_ok)
		{
			//ͨ���е�ʱ����onlineӦ�ú��Ե�
			if (sip_status_ == sip_status_connected ||
				sip_status_ == sip_status_hunguping ||
				sip_status_ == sip_status_calling ||
				sip_status_ == sip_status_ringing ||
				sip_status_ == sip_status_answering 
				)
			{
				return;
			}
		}

		if (code == sip_error_ok)
		{
			sip_status_ = status;
		}
		else
		{
			if ( status != sip_status_disconnected )
			{
				//�м���̵Ĳ��ǶϿ����ӵĳ���ͳһ�ó�����
				sip_status_ = sip_status_online;
			}
			else
			{
				sip_status_ = sip_status_disconnected;
			}
		}

		if (notify_)
		{
			notify_->OnSipStatusNotify(status, "", id, code,origin);
		}
		if (sip_status_ == sip_status_neterror)
		{
			StopSipPlugin();
			return;
		}
	}

	bool SipTalkManager::startSipPlugin()
	{
		//�رյ����ܴ��ڵĲ��
		StopSipPlugin();
		Sleep(100);
		//��Ȼ���ڵĻ���ֱ��ɱ������
		nbase::win32::CloseProcess(SIP_PLUGIN_NAME);
		
		if (sip_status_ == sip_status_neterror)
		{
			return false;
		}
		if (param_.server_.empty() || param_.id_.empty() || param_.pwd_.empty())
		{
			return false;
		}

		sip_status_ = sip_status_offline;

		//��ȡ���·��
		std::wstring sipPluginPath = QPath::GetAppPath().append(SIP_PLUGIN_NAME);
		//ƴװ������
		////hwnd&server&id&user&pwd&stun&protocol
		std::string sipCmdLine = nbase::StringPrintf("%d&%s&%s&%s&%s&%s&%d", param_.hwnd_,param_.server_.c_str(),
			param_.id_.c_str(),param_.user_.c_str(),param_.pwd_.c_str(),param_.stun_.c_str(),param_.p_);

		int ret= (int)ShellExecuteW(NULL, L"open", sipPluginPath.c_str(), nbase::UTF8ToUTF16(sipCmdLine).c_str(), NULL, SW_SHOWDEFAULT);

		return ret > 32;
	}

	void SipTalkManager::StopSipPlugin()
	{
		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return;
		}
		SendCmd(SIP_CMD_CLOSE, "");
	}


	void SipTalkManager::SendCmd(const std::string& funcname, const std::string& cmdparam)
	{
		Json::Value value;
		Json::FastWriter writer;

		value["func"] = funcname;
		value["param"] = cmdparam;

		std::string cmd = writer.write(value);
		COPYDATASTRUCT   cpd;
		cpd.dwData = 0;
		cpd.cbData = cmd.size() + 1;
		cpd.lpData = (void*)cmd.c_str();
		SendMessage(sip_wnd_, WM_COPYDATA, NULL, (LPARAM)&cpd);
	}

	LRESULT CALLBACK SipTalkManager::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		//WM_COPYDATA ����һЩ����Ĳ���  ������Ϣ����һЩ״̬��֪ͨ
		switch (message)
		{
		case SIP_MSG_ONLINE:
			if (lParam != 0)
			{
				HWND hwnd = (HWND)lParam;
				if (IsWindow(hwnd))
				{
					SipTalkManager::GetInstance()->SetSipWnd(hwnd);
				}
			}
			SipTalkManager::GetInstance()->SetStatus(sip_status_online, "", (SipErrorCode)wParam);
			QLOG_APP(L"sipע��ɹ�");
			break;
		case SIP_MSG_OFFLINE:
			QLOG_APP(L"sip������");
			SipTalkManager::GetInstance()->SetStatus(sip_status_offline, "", (SipErrorCode)wParam);
			break;
		case SIP_MSG_CALLING:
			SipTalkManager::GetInstance()->SetStatus(sip_status_calling, "", (SipErrorCode)wParam);
			break;
		case SIP_MSG_ANSERING:
			SipTalkManager::GetInstance()->SetStatus(sip_status_answering, "", (SipErrorCode)wParam);
			break;
		case SIP_MSG_HANGUPING:
			SipTalkManager::GetInstance()->SetStatus(sip_status_hunguping, "", (SipErrorCode)wParam);
			break;
		case SIP_MSG_CONNECTED:
			SipTalkManager::GetInstance()->SetStatus(sip_status_connected, "", (SipErrorCode)wParam);
			break;
		case SIP_MSG_INCOMING:  //�����on_call_state�ص��ģ�on_incoming_call�ص���������Է�id
			SipTalkManager::GetInstance()->SetStatus(sip_status_ringing, "", (SipErrorCode)wParam);
			break;
		case SIP_MSG_DISCONNECT:  //�����lParam���������ԭʼ��sipЭ��״̬��,��ʱδ����
			SipTalkManager::GetInstance()->SetStatus(sip_status_disconnected, "", (SipErrorCode)wParam,lParam);
			QLOG_APP(L"ͨ������--------������{0}") << lParam;
			break;
		case SIP_MSG_AUD_CHECK:  //�����lParam���������ԭʼ��sipЭ��״̬��,��ʱδ����
			SipTalkManager::GetInstance()->SetHasAudio(lParam);
			QLOG_APP(L"��˷�����{0}") << lParam;
			break;

		case WM_COPYDATA:  //Ŀǰ������on_incoming_call�ص���������Է�id������չ
		{
			//{"state":"state_incoming", "id" : "xxx"}
			COPYDATASTRUCT* data = (COPYDATASTRUCT*)lParam;
			std::string jsondata = (char*)data->lpData;
			Json::Reader reader;
			Json::Value value;
			reader.parse(jsondata, value);
			std::string id= value["id"].asString();
			SipTalkManager::GetInstance()->SetStatus(sip_status_ringing, id, sip_error_ok);
		}
			break;
		case WM_TIMER:
			//��ʱ��⵽
			SipTalkManager::GetInstance()->CheckSipPluginAlive();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

}