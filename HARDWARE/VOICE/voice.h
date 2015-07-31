#ifndef __VOICE_H
#define __VOICE_H

#include "usr_includes.h"

void VOICE_Init(void);					//语音芯片初始化
void VOICE_WR(u8 cmd);					//向语音芯片写指令
void VOICE_PLAY(u8 adr);				//播放adr对应地址段的语音

#endif
