#include "MMNotificationClient.h"
#include "MMWrap.h"

#ifdef __cplusplus
extern "C" {
#endif

	CMMNotificationClient* mm = NULL;

	extern void CreateMMInstance(HWND hwnd)
	{
		if (NULL == mm)
		{
			mm = new CMMNotificationClient(hwnd);
		}
	}


	extern void ReleaseMMInstance()
	{
		if (NULL!=mm)
		{
			delete mm;
			mm = NULL;
		}
	}

#ifdef __cplusplus
};

#endif