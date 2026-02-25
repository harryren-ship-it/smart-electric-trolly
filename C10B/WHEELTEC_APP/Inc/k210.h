#ifndef __K210_H
#define __K210_H

#include "main.h"

//与K210通信的数据包 --- 巡线功能
#define K210_HEAD 0xCC
#define K210_END  0xDD
#pragma pack(1) 
typedef struct {
	uint8_t Head;         
	uint16_t Cam_W; 
	uint16_t Cam_H; 
	uint16_t cx; 
	uint16_t color; 
	uint16_t size;
	uint8_t BCCcheck;    
	uint8_t End;          
}K210_Recvmsg;
#pragma pack() 
//与K210通信的数据包 --- 巡线功能

extern K210_Recvmsg k210; //定义接收数据的数据组
uint8_t k210_data_callback(uint8_t recv);
unsigned char calculateBCC(const unsigned char *data, uint16_t length);
#endif /* #ifndef __K210_H */
