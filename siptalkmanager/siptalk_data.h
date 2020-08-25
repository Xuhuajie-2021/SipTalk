#pragma once
#include "db/db_sqlite3.h"

//命令名字,需要与bizDta.h同步
#define SIP_CMD_ANSWER         "answer_call"
#define SIP_CMD_CALL           "make_call"
#define SIP_CMD_HANGUP         "hangup_call"
#define SIP_CMD_CANCEL         "cancel_call"
#define SIP_CMD_INCOMING       "state_incoming"

#define SIP_PLUGIN_NAME         L"sipPlugin.exe"

#define HEART_GAP              5000

namespace nim_comp
{
	//Sip的状态集合
	enum SipStatus
	{
		sip_status_default,
		sip_status_offline,
		sip_status_online,
		sip_status_calling,
		sip_status_ringing,
		sip_status_answering,
		sip_status_connected,
		sip_status_hunguping,
		sip_status_disconnected,  //断开连接了，和在线的状态有点类似，用两种状态可以更好的满足业务
	};

	//需要与bizDta.h同步
	enum SipErrorCode
	{
		sip_error_ok = 0,
		sip_error_unknown,
		sip_error_server_error,
		sip_error_calling_error,
		sip_error_hangup_error,
		sip_error_answer_error,
		sip_error_cancel,
		sip_error_timeout,
		sip_error_notfound,
	};

	//Sip的简单通知消息体集合，从WM_APP开始 需要与bizDta.h同步
	//wParam 错误码 
	enum SipMessage
	{
		SIP_MSG_ONLINE = WM_APP + 1,    //lParam携带sipWnd
		SIP_MSG_OFFLINE,
		SIP_MSG_CALLING,
		SIP_MSG_ANSERING,
		SIP_MSG_HANGUPING,
		SIP_MSG_CONNECTED,
		SIP_MSG_INCOMING,
		SIP_MSG_DISCONNECT,

		//心跳
		SIP_MSG_HEART, 
	};

// 	//Sip的状态业务回调
	class ISipStatusNotify
	{
	public:
		virtual ~ISipStatusNotify(){}

		/**
		* @brief 状态回调
		* @param[out] status   状态
		* @param[out] call_id  保留，多路通话时的时候需要这个字段，目前不需要
		* @param[out] id       分配的id，或电话号码（如8005 或 13000000001）
		* @param[out] code     错误码    
		* @return 无
		* @remark： 当code非sip_error_ok时，表明此时的status中出现了错误。有些错误情况建议维持原来的状态
		（如挂断失败，此时如果已经是在线，则在线，连接中，则继续连接中比较好）。
		（如拨打超时失败，则需要从calling转换成online，业务层好好斟酌取舍）
		（当状态是断开连接时，code适合提示用户为啥出错)
		*/
		virtual void OnSipStatusNotify(SipStatus status, std::string call_id, std::string id, SipErrorCode code) = 0;
 	};

	//Sip的命令集合
	enum SipCmd
	{
		sip_cmd_register,
		sip_cmd_call,
		sip_cmd_answer,
		sip_cmd_handup,
	};

	//Sip的协议类型
	enum SipProtocol
	{
		sip_p_udp, 
		sip_p_tcp,
		sip_p_tls,   //暂不支持
	};

	struct restartParam
	{
		HWND hwnd_;
		std::string server_;
		std::string id_;
		std::string user_;
		std::string pwd_;
		std::string stun_;
		SipProtocol p_;
	};

}