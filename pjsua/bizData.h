#ifndef __PJSUA_BIZDATA_H__
#define __PJSUA_BIZDATA_H__
#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息

#ifdef DEBUG
#define BIZ_UNIT_TEST    1              //设置为测试模式，产品中可以注释掉或者定义为0
#else
#define BIZ_UNIT_TEST   0
#endif


#define ID_TIMER_CHECKALIVE   1
#define ID_TIMER_CHECKDEVICE  2

// Windows 头文件: 
#include <windows.h>

#define SIP_CMD_ANSWER         "answer_call"
#define SIP_CMD_CALL           "make_call"
#define SIP_CMD_HANGUP         "hangup_call"
#define SIP_CMD_CANCEL         "cancel_call"
#define SIP_CMD_MUTE           "mute"
#define SIP_CMD_INCOMING       "state_incoming"
#define SIP_CMD_DTMF           "send_dtmf"
#define SIP_CMD_CLOSE          "close"

//内部用的消息，从WM_USER开始
enum Sip_MSG_USED
{
	WM_APP_INIT = WM_USER + 1,
	WM_APP_DESTROY,
	WM_APP_RESTART,
    WM_APP_CALL, 
	WM_APP_ANSWER,
	WM_APP_HUNGUP,
	WM_APP_CANCEL,
	WM_APP_MUTE,
	WM_APP_DTMF,
};

struct CmdParams
{
	HWND hwnd_;
	char server_[64];
	char id_[32];
	char user_[32];
	char pwd_[64];
	char stun_[32];
};

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

//Sip的简单通知消息体集合，从WM_APP开始
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
	SIP_MSG_AUD_CHECK,            //麦检测， lparam 1 有 0 无 
};


#endif
