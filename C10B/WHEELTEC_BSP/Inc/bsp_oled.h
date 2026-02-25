#ifndef __OLED_H
#define __OLED_H	

#include <stdint.h>

void OLED_Refresh_Gram(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size,uint8_t mode);
void OLED_ShowNumber(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
void OLED_ShowString(uint8_t x,uint8_t y,const char *p);
void OLED_ShowShortNum(uint8_t x,uint8_t y,short num,uint8_t len,uint8_t size);
void OLED_Init(void);
void OLED_ShowFloat(uint8_t show_x,uint8_t show_y,const float needtoshow,uint8_t zs_num,uint8_t xs_num);

#endif  
	 
