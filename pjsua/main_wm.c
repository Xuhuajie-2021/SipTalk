/* $Id$ */
/* 
 * Copyright (C) 2013 Teluu Inc. (http://www.teluu.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
//#include <aygshell.h>
#define MAINWINDOWCLASS TEXT("PjsuaDlg")
#define MAINWINDOWTITLE TEXT("PJSUA")
#define LOGO_PATH TEXT("\\Program Files\\pjsua\\pjsua.bmp")

#include "pjsua_app.h"
#include "pjsua_app_config.h"
#include "pjsua_helper.h"

static HINSTANCE	 g_hInst;
static HWND		 g_hWndMenuBar;
HWND		 g_hWndMain;
HWND		 g_hWndLbl;
static HWND		 g_hWndImg;
static HBITMAP		 g_hBmp;

// hwnd&server&id&user&pwd&protocol
struct  CmdParams g_cmdparam;
pj_str_t g_cmd_line;

void CheckCmd( char* cmdJson,size_t t);



LRESULT CALLBACK DialogProc(const HWND hWnd,
			    const UINT Msg, 
			    const WPARAM wParam,
			    const LPARAM lParam) 
{   
    LRESULT res = 0;

    switch (Msg) {
    case WM_CREATE:
	g_hWndMain = hWnd;
	break;
    case WM_CLOSE:
	PostQuitMessage(0);
	break;
	case WM_TIMER:
		if (wParam == ID_TIMER_CHECKALIVE)
		{
			if (!IsWindow(g_cmdparam.hwnd_))
			{
				if (!BIZ_UNIT_TEST)
				{
					PostQuitMessage(0);
				}
			}
		}
		else if (wParam == ID_TIMER_CHECKDEVICE)
		{
			//todo
			KillTimer(g_hWndMain, ID_TIMER_CHECKDEVICE);
			PjsuaCheckDevice();
		}
	break;
    case WM_APP_INIT:
    case WM_APP_RESTART:
	PjsuaInit();
	break;
	case WM_DEVICECHANGE:
		KillTimer(g_hWndMain, ID_TIMER_CHECKDEVICE);
		SetTimer(g_hWndMain,  ID_TIMER_CHECKDEVICE, 1500, NULL);
	break;

    case WM_APP_DESTROY:
	PostQuitMessage(0);
	break;
	case WM_CHAR:
	{
		char* mmm;
		switch (wParam)
		{
		case 'A':
		case 'a':
			mmm = "{ \"func\":\"answer_call\", \"param\" : \"id\" }";
			CheckCmd(mmm, strlen(mmm) + 1);
			break;
		case 'H':
		case 'h':
			mmm = "{ \"func\":\"hangup_call\", \"param\" : \"id\" }";
			CheckCmd(mmm, strlen(mmm) + 1);
			break;
		case 'C':
		case 'c':
			mmm = "{ \"func\":\"make_call\", \"param\" : \"057122222222\" }";
			CheckCmd(mmm, strlen(mmm) + 1);
			break;
		case 'F':
		case 'f':
			mmm = "{ \"func\":\"send_dtmf\", \"param\" : \"256175\" }";
			CheckCmd(mmm, strlen(mmm) + 1);
			break;
		case 'D':
		case 'd':
			mmm = "{ \"func\":\"cancel_call\", \"param\" : \"8002\" }";
			CheckCmd(mmm, strlen(mmm) + 1);
			break;
		case 'M':
		case 'm':
			mmm = "{ \"func\":\"mute\", \"param\" : \"1.0\" }";
			CheckCmd(mmm, strlen(mmm) + 1);
			break;
		case 'N':
		case 'n':
			mmm = "{ \"func\":\"mute\", \"param\" : \"0.0\" }";
			CheckCmd(mmm, strlen(mmm) + 1);
			break;
		case 'T':
		case 't':
			//做一些单元测试
			SendJsonCmd(SIP_CMD_INCOMING, pj_str("\"8002\" <sip:8002@10.9.10.189>"));
			break;
		default:
			break;
		}
	}
 		break;

	case WM_APP_CALL:
	{
		char* id = (char*)lParam;
		PjsuaCall(id);
		free(id);  //id释放，call函数不负责变量释放，万一有些id是不需要释放
	}
	break;

	case WM_APP_ANSWER:
		PjsuaAnswer();
		break;
	case WM_APP_HUNGUP:
		PjsuaHangup();
		break;
	case WM_APP_CANCEL:
		PjsuaCancel();
		break;
	case WM_APP_MUTE:
	{
		char* id = (char*)lParam;
		PjsuaMute(atof(id));
		free(id);  //id释放，call函数不负责变量释放，万一有些id是不需要释放
	}
		break;
	case WM_APP_DTMF:
	{
		char* id = (char*)lParam;
		PjsuaDtmf(id);
		free(id);  //id释放，call函数不负责变量释放，万一有些id是不需要释放
	}
		break;
	case WM_COPYDATA:
	{
		COPYDATASTRUCT* data = (COPYDATASTRUCT*)lParam;
		CheckCmd(data->lpData, data->cbData);
	}
	break;

    default:
	return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

    return res;
}


/* === GUI === */

pj_status_t GuiInit()
{
    WNDCLASS wc;
    HWND hWnd = NULL;	
    RECT r;
    DWORD dwStyle;
    enum { LABEL_HEIGHT = 30 };
    enum { MENU_ID_EXIT = 50000 };
//    BITMAP bmp;
//    HMENU hRootMenu;
   // SHMENUBARINFO mbi;

    pj_status_t status  = PJ_SUCCESS;
    
    /* Check if app is running. If it's running then focus on the window */
    hWnd = FindWindow(MAINWINDOWCLASS, MAINWINDOWTITLE);

    if (NULL != hWnd) {
	SetForegroundWindow(hWnd);    
	return status;
    }

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)DialogProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName	= 0;
    wc.lpszClassName = MAINWINDOWCLASS;
    
    if (!RegisterClass(&wc) != 0) {
	DWORD err = GetLastError();
	return PJ_RETURN_OS_ERROR(err);
    }

	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD styleEx = WS_EX_TOOLWINDOW;
	if (BIZ_UNIT_TEST)
	{
		style |= WS_VISIBLE;
		styleEx = 0;
	}
	
	/* Create the app. window */
	g_hWndMain = CreateWindowEx(styleEx, MAINWINDOWCLASS, MAINWINDOWTITLE,
		style, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		(HWND)NULL, NULL, g_hInst, (LPSTR)NULL);
	if (g_hWndMain == NULL) {
		DWORD err = GetLastError();
		return PJ_RETURN_OS_ERROR(err);
	}


    /* Create exit menu */
//     hRootMenu = CreateMenu();
//     AppendMenu(hRootMenu, MF_STRING, MENU_ID_EXIT, L"Exit");

    /* Initialize menubar */
//     ZeroMemory(&mbi, sizeof(SHMENUBARINFO));
//     mbi.cbSize      = sizeof(SHMENUBARINFO);
//     mbi.hwndParent  = g_hWndMain;
//     mbi.dwFlags	    = SHCMBF_HIDESIPBUTTON|SHCMBF_HMENU;
//     mbi.nToolBarId  = (UINT)hRootMenu;
//     mbi.hInstRes    = g_hInst;

//     if (FALSE == SHCreateMenuBar(&mbi)) {
// 	DWORD err = GetLastError();
//         return PJ_RETURN_OS_ERROR(err);
//     }

    /* Store menu window handle */
   // g_hWndMenuBar = mbi.hwndMB;

    /* Show the menu */
//     DrawMenuBar(g_hWndMain);
//     ShowWindow(g_hWndMenuBar, SW_SHOW);

    /* Override back button */
//     SendMessage(g_hWndMenuBar, SHCMBM_OVERRIDEKEY, VK_TBACK,
// 	    MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY,
// 	    SHMBOF_NODEFAULT | SHMBOF_NOTIFY));

    /* Get main window size */
    GetClientRect(g_hWndMain, &r);
#if defined(WIN32_PLATFORM_PSPC) && WIN32_PLATFORM_PSPC != 0
    /* Adjust the height for PocketPC platform */
    r.bottom -= GetSystemMetrics(SM_CYMENU);
#endif

    /* Create logo */
//     g_hBmp = SHLoadDIBitmap(LOGO_PATH); /* for jpeg, uses SHLoadImageFile() */
//     if (g_hBmp == NULL) {
// 	DWORD err = GetLastError();
// 	return PJ_RETURN_OS_ERROR(err);
//     }
//     GetObject(g_hBmp, sizeof(bmp), &bmp);
// 
//     dwStyle = SS_CENTERIMAGE | SS_REALSIZEIMAGE | SS_BITMAP |
// 	      WS_CHILD | WS_VISIBLE;
//     g_hWndImg = CreateWindow(TEXT("STATIC"), NULL, dwStyle,
// 			     (r.right-r.left-bmp.bmWidth)/2,
// 			     (r.bottom-r.top-bmp.bmHeight)/2,
// 			     bmp.bmWidth, bmp.bmHeight,
// 			     g_hWndMain, (HMENU)0, g_hInst, NULL);
//     if (g_hWndImg == NULL) {
// 	DWORD err = GetLastError();
// 	return PJ_RETURN_OS_ERROR(err);
//     }
//     SendMessage(g_hWndImg, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)g_hBmp);

    /* Create label */
    dwStyle = WS_CHILD | WS_VISIBLE | ES_CENTER;
    g_hWndLbl = CreateWindow(TEXT("STATIC"), NULL, dwStyle,
		0, r.bottom-LABEL_HEIGHT, r.right-r.left, LABEL_HEIGHT,
                g_hWndMain, (HMENU)0, g_hInst, NULL);
    if (g_hWndLbl == NULL) {
	DWORD err = GetLastError();
	return PJ_RETURN_OS_ERROR(err);
    }
    SetWindowText(g_hWndLbl, _T("Please wait.."));

    return status;
}


pj_status_t GuiStart()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (msg.wParam);
}

void GuiDestroy(void)
{
    if (g_hWndMain) {
	DestroyWindow(g_hWndMain);
	g_hWndMain = NULL;
    }
    if (g_hWndMenuBar) {
	DestroyWindow(g_hWndMenuBar);
	g_hWndMenuBar = NULL;
    }
    if (g_hWndLbl) {
	DestroyWindow(g_hWndLbl);
	g_hWndLbl = NULL;
    }
    if (g_hWndImg) {
	DestroyWindow(g_hWndImg);
	g_hWndImg = NULL;
    }
    if (g_hBmp) {
	DeleteObject(g_hBmp);
	g_hBmp = NULL;
    }
    UnregisterClass(MAINWINDOWCLASS, g_hInst);
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
    int nShowCmd
)
{
    int status;

    PJ_UNUSED_ARG(hPrevInstance);
   // PJ_UNUSED_ARG(lpCmdLine);
	g_cmd_line = pj_str(lpCmdLine);
	if (g_cmd_line.slen == 0)
	{
// #ifdef BIZ_UNIT_TEST
// 		g_cmd_line = pj_str("1234&101.231.133.17:5068&8008&8008&Better2@20&101.231.133.21&1&");
// #else
		return 0;
	}
	//g_cmdparam.hwnd_ = pj_strtol(&cmd_line);
	
    PJ_UNUSED_ARG(nShowCmd);

    // store the hInstance in global
    g_hInst = hInstance;

	//允许自由发送接收WM_COPYDATA消息
	HINSTANCE hUser32 = LoadLibrary("user32.dll");
	if (hUser32)
	{
		typedef BOOL(WINAPI* LPChangeWindowMessageFilter)(UINT, DWORD);
		LPChangeWindowMessageFilter pPChangeWindowMessageFilter = (LPChangeWindowMessageFilter)GetProcAddress(hUser32, "ChangeWindowMessageFilter");
		if (pPChangeWindowMessageFilter)
		{
			pPChangeWindowMessageFilter(WM_COPYDATA, 1);
		}
		FreeLibrary(hUser32);
	}

    // Start GUI
    status = GuiInit();
    if (status != 0)
	goto on_return;

    PostMessage(g_hWndMain, WM_APP_INIT, 0, 0);

    status = GuiStart();
	
on_return:
    PjsuaDestroy();
    GuiDestroy();

    return status;
}



