/** @file siptalk_data.h
* @brief ά����Sip����Ĺ��ܵ�����ͨ��(��Ƽ�https://www.cnblogs.com/xuhuajie/p/13445294.html)
* @copyright (c) 2020-2023, Netease Inc. All rights reserved
* @author Xuhuajie
* @date 2020/8/13
*/

#pragma once
//��������,��Ҫ��bizDta.hͬ��
#define SIP_CMD_ANSWER         "answer_call"
#define SIP_CMD_CALL           "make_call"
#define SIP_CMD_HANGUP         "hangup_call"
#define SIP_CMD_CANCEL         "cancel_call"
#define SIP_CMD_MUTE           "mute"
#define SIP_CMD_INCOMING       "state_incoming"
#define SIP_CMD_DTMF           "send_dtmf"
#define SIP_CMD_CLOSE          "close"

#define SIP_PLUGIN_NAME         L"sipPlugin.exe"

#define HEART_GAP              5000

namespace nim_comp
{
	//Sip��״̬����
	enum SipStatus
	{
		sip_status_default,
		sip_status_offline,      //ע��ʧ���ˣ�sip���������⣿
		sip_status_neterror,     //�������������
		sip_status_online,
		sip_status_calling,
		sip_status_ringing,
		sip_status_answering,
		sip_status_connected,
		sip_status_hunguping,
		sip_status_disconnected,  //�Ͽ������ˣ������ߵ�״̬�е����ƣ�������״̬���Ը��õ�����ҵ��
	};

	//��Ҫ��bizDta.hͬ��
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
		sip_error_meeting_error,
	};

	//Sip�ļ�֪ͨ��Ϣ�弯�ϣ���WM_APP��ʼ ��Ҫ��bizDta.hͬ��
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
		SIP_MSG_AUD_CHECK,            //���⣬ lparam 1 �� 0 �� 

		//����
		SIP_MSG_HEART, 
	};

// 	//Sip��״̬ҵ��ص�
	class ISipStatusNotify
	{
	public:
		virtual ~ISipStatusNotify(){}

		/**
		* @brief ״̬�ص�
		* @param[out] status   ״̬
		* @param[out] call_id  ��������·ͨ��ʱ��ʱ����Ҫ����ֶΣ�Ŀǰ����Ҫ
		* @param[out] id       �����id����绰���루��8005 �� 13000000001��
		* @param[out] code     ������    
		* @param[out] origin   ԭʼ������,disconnectʱ��Ч
		* @return ��
		* @remark�� ��code��sip_error_okʱ��������ʱ��status�г����˴�����Щ�����������ά��ԭ����״̬
		����Ҷ�ʧ�ܣ���ʱ����Ѿ������ߣ������ߣ������У�����������бȽϺã���
		���粦��ʱʧ�ܣ�����Ҫ��callingת����online��ҵ���ú�����ȡ�ᣩ
		����״̬�ǶϿ�����ʱ��code�ʺ���ʾ�û�Ϊɶ����)
		*/
		virtual void OnSipStatusNotify(SipStatus status, std::string call_id, std::string id, SipErrorCode code,int origin) = 0;
 	};

	//Sip�������
	enum SipCmd
	{
		sip_cmd_register,
		sip_cmd_call,
		sip_cmd_answer,
		sip_cmd_handup,
	};

	//Sip��Э������
	enum SipProtocol
	{
		sip_p_udp, 
		sip_p_tcp,
		sip_p_tls,   //�ݲ�֧��
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