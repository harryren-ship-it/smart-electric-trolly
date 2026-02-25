#include "k210.h"

// 通用 BCC校验和的函数
unsigned char calculateBCC(const unsigned char *data, uint16_t length) {
    unsigned char bcc = 0;
	uint16_t i =0;
    for (i = 0; i < length; i++) {
        bcc ^= data[i];
    }
    return bcc;
}

K210_Recvmsg k210; //定义接收数据的数据组
uint8_t k210_data_callback(uint8_t recv)
{
	static uint8_t recvlen = sizeof(k210);//计算要接收的数据大小
	static uint8_t recv_data[sizeof(k210)];//用于存放接收数据的数组
	static uint8_t recv_counts=0;//接收到的数据计数值
	
	//数据就绪标志位
	uint8_t data_ready = 0;
	
	recv_data[recv_counts] = recv;
	
    if( recv==K210_HEAD || recv_counts>0) //检查帧头是否正确
        recv_counts++;
    else
        recv_counts=0;
	
    if( recv_counts==recvlen )//接收到满足数据长度的数据
    {	
        recv_counts=0;//清空计数值,以便下次使用
        if( recv_data[recvlen-1]== K210_END ) //检查帧尾是否正确
        {		
            //检查BCC校验值是否正确
            if( recv_data[recvlen-2]==calculateBCC(recv_data,recvlen-2) )
            {
                //数据被正确接收后,开始自动解包
                uint8_t *recvptr = (uint8_t*)&k210;
                for(uint8_t i=0;i<recvlen;i++)
                {
                    *recvptr = recv_data[i];
                    recvptr++;
                }
				/////数据解包完成,下面可以开始使用接受到的数据/////////
				data_ready = 1;
            }
        }
    }
	
	//返回解包情况
	return data_ready;
	
}

