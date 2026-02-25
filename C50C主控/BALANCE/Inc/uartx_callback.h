#ifndef __UARTX_CALLBACK_H
#define __UARTX_CALLBACK_H 

#include "system.h"

//机器人接收控制命令的数据长度
#define RECEIVE_DATA_SIZE 11

//机器人接收控制命令的结构体
typedef struct _RECEIVE_DATA_  
{
	unsigned char buffer[RECEIVE_DATA_SIZE];
	struct _Control_Str_
	{
		unsigned char Frame_Header; //1 bytes //1个字节
		float X_speed;	            //4 bytes //4个字节
		float Y_speed;              //4 bytes //4个字节
		float Z_speed;              //4 bytes //4个字节
		unsigned char Frame_Tail;   //1 bytes //1个字节
	}Control_Str;
}RECEIVE_DATA;


#define C10B_HEAD1 0xCC
#define C10B_HEAD2 0xDD
#pragma pack(1) 
typedef struct {
	uint8_t Head1; 
    uint8_t Head2;
	uint8_t k210_alive;
	uint16_t k210_cx; 
	uint16_t k210_size;
	uint16_t color;
	short temperature;
	uint8_t BCCcheck;          
}C10B_Recvmsg;
#pragma pack() 

extern C10B_Recvmsg c10b_recv;

//内部函数
static float XYZ_Target_Speed_transition(u8 High,u8 Low);
static u8 AT_Command_Capture(u8 uart_recv);
static void _System_Reset_(u8 uart_recv);

#endif

