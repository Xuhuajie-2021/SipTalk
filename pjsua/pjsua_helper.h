#ifndef __PJSUA_HELPER_H__
#define __PJSUA_HELPER_H__

/* Helper funtions to init/destroy the pjsua */
void PjsuaInit();
void PjsuaDestroy();
void PjsuaAnswer();
void PjsuaHangup();
void PjsuaCancel();
void PjsuaCall(char * id);
void PjsuaMute(float mute);
void PjsuaDtmf(char* number);
void PjsuaCheckDevice();

/* pjsua app callbacks */
void PjsuaOnStarted(pj_status_t status, const char* title);
void PjsuaOnStopped(pj_bool_t restart, int argc, char** argv);
void PjsuaOnConfig(pjsua_app_config *cfg);

void CheckPostMessage(UINT msg, WPARAM w, LPARAM l);
void CheckSendMessage(UINT msg, WPARAM w, LPARAM l);
   
char* dumpPjstr(pj_str_t t);  //need free somewhere

void CheckCmd(char* cmdJson, size_t t);
void SendJsonCmd(char* state, pj_str_t id);

char* getLogPath();

#endif	/* __PJSUA_HELPER_H__ */
