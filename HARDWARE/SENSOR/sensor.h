#ifndef __SENSOR_H
#define __SENSOR_H

#include "usr_includes.h"

void SENSOR_Init(void); //SENSOR相关IO初始化
u16 SENSOR_Scan(u8 sensor_var); ////SENSOR处理函数,返回对应SENSOR值

#endif
