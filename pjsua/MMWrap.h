#ifndef _MMWRAP_H__
#define _MMWRAP_H__

//对COM的C接口封装
#ifdef __cplusplus
extern "C" {
#endif

	extern void CreateMMInstance(HWND hwnd);
	extern void ReleaseMMInstance();

#ifdef __cplusplus
};
#endif


#endif
