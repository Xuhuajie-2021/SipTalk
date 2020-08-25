#include "pjsua_app.h"
#include "pjsua_app_config.h"
#include "pjsua_helper.h"

extern HWND g_hWndMain;
extern struct  CmdParams g_cmdparam;

static  char *pjparam[] = { "pjsua",
"--use-cli",
"--no-cli-console",
"--quality=4",
#if defined(PJMEDIA_HAS_VIDEO) && PJMEDIA_HAS_VIDEO
"--video",
#endif
#if defined(PJ_SYMBIAN) && PJ_SYMBIAN
/* Can't reuse address on E52 */
"--cli-telnet-port=0",
#else
"--cli-telnet-port=2323",
#endif
#if defined(PJ_CONFIG_BB10) && PJ_CONFIG_BB10
"--add-buddy=sip:169.254.0.2",
#endif
NULL };


#ifdef BIZ_UNIT_TEST
extern HWND g_hWndLbl;
void SetToolTip(char* tooltip)
{
	SetWindowText(g_hWndLbl, tooltip);
}
#endif

void PjsuaInit()
{
	pjsua_app_cfg_t app_cfg;
	pj_status_t status;

	/* Destroy pjsua app first */
	pjsua_app_destroy();

	/* Init pjsua app */
	pj_bzero(&app_cfg, sizeof(app_cfg));
	app_cfg.argc = PJ_ARRAY_SIZE(pjparam) - 1;
	app_cfg.argv = pjparam;
	app_cfg.on_started = &PjsuaOnStarted;
	app_cfg.on_stopped = &PjsuaOnStopped;
	app_cfg.on_config_init = &PjsuaOnConfig;

	status = pjsua_app_init(&app_cfg);
	if (status != PJ_SUCCESS)
		return;

	status = pjsua_app_run(PJ_FALSE);
	if (status != PJ_SUCCESS)
		return;
}


void PjsuaDestroy()
{
	pjsua_app_destroy();
}


void PjsuaAnswer()
{
	pjsua_call_info call_info;
	pjsua_msg_data msg_data_;

	if (current_call != -1) {
		pjsua_call_get_info(current_call, &call_info);
	}
	else {
		/* Make compiler happy */
		call_info.role = PJSIP_ROLE_UAC;
		call_info.state = PJSIP_INV_STATE_DISCONNECTED;
	}

	if (current_call == -1 ||
		call_info.role != PJSIP_ROLE_UAS ||
		call_info.state >= PJSIP_INV_STATE_CONNECTING)
	{
		return;
	}
	else {
		pjsua_msg_data_init(&msg_data_);
		if (PJ_SUCCESS == pjsua_call_answer2(current_call, &call_opt, 200, NULL, &msg_data_))
		{
			PostMessage(g_cmdparam.hwnd_, SIP_MSG_ANSERING, sip_error_ok, 0);
			SET_TOOL_TIP("answering");
		}
		else
		{
			PostMessage(g_cmdparam.hwnd_, SIP_MSG_INCOMING, sip_error_answer_error, 0);
			SET_TOOL_TIP("answering error and back to incoming");
		}
	}
}


void PjsuaHangup()
{
	if (current_call != -1) 
	{
		if(PJ_SUCCESS== pjsua_call_hangup(current_call, 0, NULL, NULL))
		{
			PostMessage(g_cmdparam.hwnd_, SIP_MSG_HANGUPING, sip_error_ok, 0);
			SET_TOOL_TIP("hanguping");
		}
	    else
	    {
			PostMessage(g_cmdparam.hwnd_, SIP_MSG_CONNECTED, sip_error_hangup_error, 0);
		    SET_TOOL_TIP("hanguping error and back to conneced");
	    }
	}
}


void PjsuaCancel()
{
	if (current_call != -1)
	{
		pjsua_call_hangup(current_call, 486, NULL, NULL);
	}
}

void PjsuaCall(char * id)
{
	//组装成<sip:8002@101.231.133.17:5068;transport=tcp>
	char buff[128];
	memset(buff, 0, 128);
	strcpy(buff, "<sip:");
	strcat(buff, id);
	strcat(buff, "@");
	strcat(buff, g_cmdparam.server_);
	strcat(buff, ";transport=tcp>");

	//仍有通话进行，不允许呼叫（尽管pjsip支持多路）
	if (current_call != -1)
	{
		return;
	}

	pjsua_msg_data msg_data_;
	pj_str_t tmp = pj_str(buff);
	pjsua_msg_data_init(&msg_data_);
	if (PJ_SUCCESS == pjsua_call_make_call(current_acc, &tmp, &call_opt, NULL,
		&msg_data_, &current_call))
	{
		PostMessage(g_cmdparam.hwnd_, SIP_MSG_CALLING, sip_error_ok, 0);
		SET_TOOL_TIP("calling");
	}
	else
	{
		PostMessage(g_cmdparam.hwnd_, SIP_MSG_ONLINE, sip_error_calling_error, 0);
		SET_TOOL_TIP("calling error and back to online");
	}
		
}


/* Called when pjsua is started */
void PjsuaOnStarted(pj_status_t status, const char* title)
{
	wchar_t wtitle[128];
	char err_msg[128];

	if (status != PJ_SUCCESS || title == NULL) {
		char err_str[PJ_ERR_MSG_SIZE];
		pj_strerror(status, err_str, sizeof(err_str));
		pj_ansi_snprintf(err_msg, sizeof(err_msg), "%s: %s",
			(title ? title : "App start error"), err_str);
		title = err_msg;
	}

	pj_ansi_to_unicode(title, strlen(title), wtitle, PJ_ARRAY_SIZE(wtitle));
}

/* Called when pjsua is stopped */
void PjsuaOnStopped(pj_bool_t restart, int argc, char** argv)
{ 
	PJ_UNUSED_ARG(argc);
	PJ_UNUSED_ARG(argv);
	if (restart) {
		// Schedule Lib Restart
		PjsuaInit();
	}
	else {
		/* Destroy & quit GUI, e.g: clean up window, resources  */
		PjsuaDestroy();
	}
}

/* Called before pjsua initializing config. */
void PjsuaOnConfig(pjsua_app_config *cfg)
{
	PJ_UNUSED_ARG(cfg);
}

char* dumpPjstr(pj_str_t t)
{
	char* buff = malloc(t.slen+1);
	memset(buff, 0, t.slen + 1);
	memcpy(buff, t.ptr, t.slen);

	return buff;
}


void SendJsonCmd(char* state, pj_str_t id)
{
	//{"state":"state","id":"xxx"}
	char buff[128];
	memset(buff, 0, 128);
	strcpy(buff, "{\"state\":\"");
	strcat(buff, state);
	strcat(buff, "\",\"id\":\"");


	//id "8002" <sip:8002@10.9.10.189>
	//id 提取出8002 <sip: xxx @
	char tempid[64];
	memset(tempid, 0, 64);
	pj_size_t begin = pj_strcspn3(&id, "<sip:", 0);
	pj_size_t end = pj_strcspn3(&id, "@", begin + 1);
	memcpy(tempid, id.ptr + begin + 5, end - begin - 5);

	strcat(buff, tempid);
	strcat(buff, "\"}");

	//发送
	COPYDATASTRUCT   cpd;
	cpd.dwData = 0;
	cpd.cbData = strlen(buff) + 1;
	cpd.lpData = (void*)buff;
	SendMessage(g_cmdparam.hwnd_, WM_COPYDATA, 0, (LPARAM)&cpd);
}

// std::string string_cmd = nbase::UTF16ToUTF8(strCmdLine);
// COPYDATASTRUCT   cpd;
// cpd.dwData = 0;
// cpd.cbData = string_cmd.size() + 1;
// cpd.lpData = (void*)string_cmd.c_str();
// SendMessage(main_wnd, WM_COPYDATA, NULL, (LPARAM)&cpd

//{ "func":"make_call", "param" : "id" }

void CheckCmd(char* cmdJson, size_t t)
{
	pj_pool_t *pool;
	pj_json_elem *elem, *elem1, *elem2;
	pj_json_err_info err;

	pj_str_t cmd;
	pj_str_t param;

	pool = pjsua_pool_create("json", 1000, 1000);

	elem = pj_json_parse(pool, cmdJson, &t, &err);
	if (elem)
	{
		//pj_strcmp()
		elem1 = elem->value.children.next;
		if (elem1 && pj_strcmp2(&elem1->name, "func") == 0)
		{
			cmd = elem1->value.str;
			if (pj_strcmp2(&cmd, SIP_CMD_ANSWER) == 0)
			{
				PostMessage(g_hWndMain, WM_APP_ANSWER, 0, 0);
			}
			else if (pj_strcmp2(&cmd, SIP_CMD_HANGUP) == 0)
			{
				PostMessage(g_hWndMain, WM_APP_HUNGUP, 0, 0);
			}
			else if (pj_strcmp2(&cmd, SIP_CMD_CANCEL) == 0)
			{
				PostMessage(g_hWndMain, WM_APP_CANCEL, 0, 0);
			}
			else if (pj_strcmp2(&cmd, SIP_CMD_CALL) == 0)
			{
				elem2 = elem->value.children.prev;
				param = elem2->value.str;
				char* id = dumpPjstr(param);
				PostMessage(g_hWndMain, WM_APP_CALL, 0, (LPARAM)id);
			}
		}
	}
	//
	pj_pool_release(pool);
}