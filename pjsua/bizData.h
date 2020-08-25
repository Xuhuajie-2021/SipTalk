#ifndef __PJSUA_BIZDATA_H__
#define __PJSUA_BIZDATA_H__
#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ

#define BIZ_UNIT_TEST    1              //����Ϊ����ģʽ����Ʒ�п���ע�͵����߶���Ϊ0
// Windows ͷ�ļ�: 
#include <windows.h>

#define SIP_CMD_ANSWER         "answer_call"
#define SIP_CMD_CALL           "make_call"
#define SIP_CMD_HANGUP         "hangup_call"
#define SIP_CMD_CANCEL         "cancel_call"
#define SIP_CMD_INCOMING       "state_incoming"


//�ڲ��õ���Ϣ����WM_USER��ʼ
enum Sip_MSG_USED
{
	WM_APP_INIT = WM_USER + 1,
	WM_APP_DESTROY,
	WM_APP_RESTART,
    WM_APP_CALL, 
	WM_APP_ANSWER,
	WM_APP_HUNGUP,
	WM_APP_CANCEL,
};

struct CmdParams
{
	HWND hwnd_;
	char server_[32];
	char id_[16];
	char user_[16];
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

//Sip�ļ�֪ͨ��Ϣ�弯�ϣ���WM_APP��ʼ
//wParam ������ 
enum SipMessage
{
	SIP_MSG_ONLINE = WM_APP + 1,    //lParamЯ��sipWnd
	SIP_MSG_OFFLINE,
	SIP_MSG_CALLING,
	SIP_MSG_ANSERING,
	SIP_MSG_HANGUPING,
	SIP_MSG_CONNECTED,
	SIP_MSG_INCOMING,
	SIP_MSG_DISCONNECT,
};


#endif