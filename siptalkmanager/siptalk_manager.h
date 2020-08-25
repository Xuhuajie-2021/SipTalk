/** @file siptalk_manager.h
* @brief 维护对Sip插件的功能调用与通信(设计见https://www.cnblogs.com/xuhuajie/p/13445294.html)
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
		* @brief 管理器初始化（新建一个无界面窗口，用于进程间通信）
		* @return true 初始化成功 false 初始化失败
		*/
		bool InitEnvironment();

		/**
		* @brief 界面层增加状态通知回调（先设计成只需要一个的，目前不支持同时拨打多路的需求）
		*/
		void AddListener(ISipStatusNotify* notify);

		/**
		* @brief 界面层移除状态通知回调
		*/
		void RemoveListener(ISipStatusNotify* notify);

		/**
		* @brief 启动插件（插件会自动注册）
		* @param[in] server sip服务器地址（需要带端口）
		* @param[in] id     分配的id，或电话号码（如8005 或 13000000001）
		* @param[in] user   鉴权的用户名，空则使用id
		* @param[in] pwd    鉴权的密码
		* @param[in] p      协议类型，默认TCP
		* @return true 启动成功 false 启动失败
		*/
		bool StartSipPlugin(const std::string& server,const std::string &id,const std::string &user,
			const std::string& pwd,const std::string& stun,SipProtocol p=sip_p_tcp);
		/**
		* @brief 关闭插件
		*/
		void StopSipPlugin();
		/**
		* @brief 呼叫电话
		* @param[in] id     对方分配的id，或电话号码（如8005 或 13000000001）
		* @return true 呼叫成功 false 呼叫失败
		* @remark : 协议格式：  { "func":"make_call","param":"id" }
		*/
		bool MakeCall(const std::string& id);


		/**
		* @brief 应答电话
		* @param[in] call_id    会话id，目前不需要，多路通话的时候才有用，保留
		* @return true 应答成功 false 应答失败
		* @remark : 协议格式：  { "func":"answer_call","param":"call_id" }
		*/
		bool AnswerCall(const std::string& call_id);

		/**
		* @brief 拒绝电话
		* @param[in] call_id    会话id，目前不需要，多路通话的时候才有用，保留
		* @return true 应答成功 false 应答失败
		* @remark : 协议格式：  { "func":"cancel_call","param":"call_id" }
		*/
		bool CancelCall(const std::string& call_id);

		/**
		* @brief 挂断电话
		* @param[in] call_id    会话id，目前不需要，多路通话的时候才有用，保留
		* @return true 挂断成功 false 挂断失败
		* @remark : 协议格式：  { "func":"hangup_call","param":"call_id" }
		*/
		bool HangUpCall(const std::string& call_id);


		/**
		* @brief 设置插件窗口句柄，用于通信
		* @param[in] hWnd   通信的插件窗口句柄
		* @return 无
		*/
		void SetSipWnd(HWND hWnd);
		/**
		* @brief 检测插件是否存活，是否需要重启
		*/
		void CheckSipPluginAlive();
		/**
		* @brief 插件传递的状态信息
		*/
		void SetStatus(SipStatus status, std::string id, SipErrorCode code);

	private:
		/**
		* @brief 启动插件（插件会自动注册）
		* @return true 启动成功 false 启动失败
		*/
		bool startSipPlugin();
		/**
		* @brief 进程间通信发送命令
		*/
		void SendCmd(const std::string& funcname,const std::string& cmdparam);

		/**
		* @brief 自带的消息处理函数
		*/
		static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
	private:
		SipStatus    sip_status_;
		restartParam param_;  //重启参数
		HWND         sip_wnd_; //插件的窗口句柄，方便进程间通信
		time_t       lasttimestamp_;  
		bool         bInit_;

		ISipStatusNotify* notify_;
	};
}