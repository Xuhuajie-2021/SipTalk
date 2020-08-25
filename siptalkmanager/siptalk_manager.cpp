#include "siptalk_manager.h"
#include "shared/util.h"

namespace nim_comp
{
	
	SipTalkManager::SipTalkManager()
	{
		sip_wnd_ = NULL;
		sip_status_ = sip_status_offline;
		notify_ = nullptr;
		lasttimestamp_ = 0;
		bInit_ = false;
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
		//新建一个无界面窗口，用于进程间通信
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
			//设置定时器
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
		assert(notify_ == notify); //目前应该是相同的
		if (notify_ == notify)
		{
			notify_ = nullptr;
		}
	}

	bool SipTalkManager::StartSipPlugin(const std::string& server, const std::string &id, const std::string &user,
		const std::string& pwd, const std::string& stun, SipProtocol p/*=sip_p_udp*/)
	{
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

	//协议格式：  { "func":"make_call","param":"id" }
	bool SipTalkManager::MakeCall(const std::string& id)
	{
		//状态检测
		if (sip_status_ != sip_status_online && sip_status_ != sip_status_disconnected)
		{
			return false;
		}

		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//生成协议
		SendCmd(SIP_CMD_CALL,id);
		return true;
	}

	bool SipTalkManager::AnswerCall(const std::string& call_id)
	{
		//状态检测
		if (sip_status_ != sip_status_ringing)
		{
			return false;
		}

		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//生成协议
		SendCmd(SIP_CMD_ANSWER, call_id);
		return true;
	}

	bool SipTalkManager::CancelCall(const std::string& call_id)
	{
		//状态检测
		if (sip_status_ != sip_status_ringing && sip_status_ != sip_status_calling)
		{
			return false;
		}

		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//生成协议
		SendCmd(SIP_CMD_CANCEL, call_id);
		return true;
	}

	bool SipTalkManager::HangUpCall(const std::string& call_id)
	{
		//状态检测
		if (sip_status_ != sip_status_connected)
		{
			return false;
		}

		if (sip_wnd_ == NULL || !IsWindow(sip_wnd_))
		{
			return false;
		}

		//生成协议
		SendCmd(SIP_CMD_HANGUP, call_id);
		return true;
	}

	void SipTalkManager::SetSipWnd(HWND hWnd)
	{
		sip_wnd_ = hWnd;
	}

	//这里可以检测插件是否还存活着
	void SipTalkManager::CheckSipPluginAlive()
	{
		if (sip_status_ == sip_status_default)
		{
			return;
		}

		//todo 插件如果不在了，就重启, 可以通过进程检测，或者约定的心跳间隔
		if (nbase::win32::FindProcessId(SIP_PLUGIN_NAME) == -1)
		{
			startSipPlugin();
		}
	}

	void SipTalkManager::SetStatus(SipStatus status, std::string id, SipErrorCode code)
	{
		if (code == sip_error_ok)
		{
			sip_status_ = status;
		}
		else
		{
			if ( status != sip_status_disconnected )
			{
				//中间过程的不是断开连接的出错，统一置成在线
				sip_status_ = sip_status_online;
			}
			else
			{
				sip_status_ = sip_status_disconnected;
			}
		}

		if (notify_)
		{
			notify_->OnSipStatusNotify(status, "", id, code);
		}
	}

	bool SipTalkManager::startSipPlugin()
	{
		//关闭掉可能存在的插件
		StopSipPlugin();

		sip_status_ = sip_status_offline;

		//获取插件路径
		std::wstring sipPluginPath = QPath::GetAppPath().append(SIP_PLUGIN_NAME);
		//拼装命令行
		////hwnd&server&id&user&pwd&stun&protocol
		std::string sipCmdLine = nbase::StringPrintf("%d&%s&%s&%s&%s&%s&%d", param_.hwnd_,param_.server_.c_str(),
			param_.id_.c_str(),param_.user_.c_str(),param_.pwd_.c_str(),param_.stun_.c_str(),param_.p_);

		int ret= (int)ShellExecuteW(NULL, L"open", sipPluginPath.c_str(), nbase::UTF8ToUTF16(sipCmdLine).c_str(), NULL, SW_SHOWDEFAULT);

		return ret > 32;

		//尝试运行插件
		//return nbase::win32::RunAppWithCommand(sipPluginPath.c_str(), nbase::UTF8ToUTF16(sipCmdLine).c_str());
	}

	void SipTalkManager::StopSipPlugin()
	{
		nbase::win32::CloseProcess(SIP_PLUGIN_NAME);
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
		//WM_COPYDATA 进行一些命令的操作  其他消息进行一些状态的通知
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
			break;
		case SIP_MSG_OFFLINE:
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
		case SIP_MSG_INCOMING:  //这里从on_call_state回调的，on_incoming_call回调里面包含对方id
			SipTalkManager::GetInstance()->SetStatus(sip_status_ringing, "", (SipErrorCode)wParam);
			break;
		case SIP_MSG_DISCONNECT:  //这里的lParam里面包含了原始的sip协议状态码,暂时未启用
			SipTalkManager::GetInstance()->SetStatus(sip_status_disconnected, "", (SipErrorCode)wParam);
			break;

		case WM_COPYDATA:  //目前仅用于on_incoming_call回调里面包含对方id，可拓展
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
			//定时检测到
			SipTalkManager::GetInstance()->CheckSipPluginAlive();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

}