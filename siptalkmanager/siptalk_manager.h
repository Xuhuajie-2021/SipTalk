/** @file siptalk_manager.h
* @brief ά����Sip����Ĺ��ܵ�����ͨ��(��Ƽ�https://www.cnblogs.com/xuhuajie/p/13445294.html)
* @copyright (c) 2020-2023, Netease Inc. All rights reserved
* @author Xuhuajie
* @date 2020/8/13
*/

#pragma once
#include "siptalk_data.h"

namespace nim_comp
{
	class SipTalkManager
	{
	public:
		SINGLETON_DEFINE(SipTalkManager);
	public:
		SipTalkManager();
		~SipTalkManager();

		/**
		* @brief ��������ʼ�����½�һ���޽��洰�ڣ����ڽ��̼�ͨ�ţ�
		* @return true ��ʼ���ɹ� false ��ʼ��ʧ��
		*/
		bool InitEnvironment();

		/**
		* @brief ���������״̬֪ͨ�ص�������Ƴ�ֻ��Ҫһ���ģ�Ŀǰ��֧��ͬʱ�����·������
		*/
		void AddListener(ISipStatusNotify* notify);

		/**
		* @brief ������Ƴ�״̬֪ͨ�ص�
		*/
		void RemoveListener(ISipStatusNotify* notify);

		/**
		* @brief ���������������Զ�ע�ᣩ
		* @param[in] server sip��������ַ����Ҫ���˿ڣ�
		* @param[in] id     �����id����绰���루��8005 �� 13000000001��
		* @param[in] user   ��Ȩ���û���������ʹ��id
		* @param[in] pwd    ��Ȩ������
		* @param[in] p      Э�����ͣ�Ĭ��TCP
		* @return true �����ɹ� false ����ʧ��
		*/
		bool StartSipPlugin(const std::string& server,const std::string &id,const std::string &user,
			const std::string& pwd,const std::string& stun,SipProtocol p=sip_p_tcp);
		/**
		* @brief �رղ��
		*/
		void StopSipPlugin();
		/**
		* @brief ���е绰
		* @param[in] id     �Է������id����绰���루��8005 �� 13000000001��
		* @return true ���гɹ� false ����ʧ��
		* @remark : Э���ʽ��  { "func":"make_call","param":"id" }
		*/
		bool MakeCall(const std::string& id);


		/**
		* @brief Ӧ��绰
		* @param[in] call_id    �Ựid��Ŀǰ����Ҫ����·ͨ����ʱ������ã�����
		* @return true Ӧ��ɹ� false Ӧ��ʧ��
		* @remark : Э���ʽ��  { "func":"answer_call","param":"call_id" }
		*/
		bool AnswerCall(const std::string& call_id);

		/**
		* @brief �ܾ��绰
		* @param[in] call_id    �Ựid��Ŀǰ����Ҫ����·ͨ����ʱ������ã�����
		* @return true Ӧ��ɹ� false Ӧ��ʧ��
		* @remark : Э���ʽ��  { "func":"cancel_call","param":"call_id" }
		*/
		bool CancelCall(const std::string& call_id);

		/**
		* @brief �Ҷϵ绰
		* @param[in] call_id    �Ựid��Ŀǰ����Ҫ����·ͨ����ʱ������ã�����
		* @return true �Ҷϳɹ� false �Ҷ�ʧ��
		* @remark : Э���ʽ��  { "func":"hangup_call","param":"call_id" }
		*/
		bool HangUpCall(const std::string& call_id);


		/**
		* @brief ���ò�����ھ��������ͨ��
		* @param[in] hWnd   ͨ�ŵĲ�����ھ��
		* @return ��
		*/
		void SetSipWnd(HWND hWnd);
		/**
		* @brief ������Ƿ���Ƿ���Ҫ����
		*/
		void CheckSipPluginAlive();
		/**
		* @brief ������ݵ�״̬��Ϣ
		*/
		void SetStatus(SipStatus status, std::string id, SipErrorCode code);

	private:
		/**
		* @brief ���������������Զ�ע�ᣩ
		* @return true �����ɹ� false ����ʧ��
		*/
		bool startSipPlugin();
		/**
		* @brief ���̼�ͨ�ŷ�������
		*/
		void SendCmd(const std::string& funcname,const std::string& cmdparam);

		/**
		* @brief �Դ�����Ϣ������
		*/
		static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
	private:
		SipStatus    sip_status_;
		restartParam param_;  //��������
		HWND         sip_wnd_; //����Ĵ��ھ����������̼�ͨ��
		time_t       lasttimestamp_;  
		bool         bInit_;

		ISipStatusNotify* notify_;
	};
}