#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#include "m_pd.h"
#define PD_EXPORT_FUNC	__declspec(dllexport)
#define PD_CDECL		__cdecl
#else
#include "pd/m_pd.h"
#define PD_EXPORT_FUNC
#define PD_CDECL
#endif

struct LinkuinoClient;
struct LinkuinoClient* linkuino_open_device(int);
int linkuino_getversionmajor(struct LinkuinoClient*);
int linkuino_getversionminor(struct LinkuinoClient*);
void linkuino_set_pwm_value(struct LinkuinoClient*, int, float);
void linkuino_set_dout(struct LinkuinoClient*, int);
void linkuino_send(struct LinkuinoClient*);

#ifdef __cplusplus
}
#endif