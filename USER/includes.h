#include "delay.h"
#include "sys.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_it.h"

#include "led.h"
#include "voice.h"
#include "remote.h"
#include "key.h"

#include "motor_move.h"
#include "motor_side.h"
#include "fan.h"

#define Forward				0
#define Backward			1
#define ClkWiz				2
#define	AntiClkWiz		3

#define Run						0
#define	Stop					1

#define speed_high		1000
#define speed_mid			4000
#define speed_low			7000

