import sensor , image , time , lcd
from fpioa_manager import fm
from machine import UART

from board import board_info
from maix import GPIO

# 导入数据打包和解析类方法
import struct
from collections import OrderedDict # 用于创建有序字典

# 定时器模块
from machine import Timer



"""
将需要发送的数据打包 类
"""
class Uart_RecvPack():
    def __init__(self,packmsg,dataformat):

        # 传输的数据包
        self.msg = packmsg

        # 传输的数据格式
        self.recvformat = dataformat

        # 要传输的数据长度
        self.data_len = struct.calcsize(self.recvformat)

    # BCC校验函数
    def calculate_BCC(self,datalist,datalen):
        ref = 0
        for i in range(datalen):
            ref = (ref^datalist[i])
        return ref&0xff

    # 接收读取到的数据列表并解包
    def unpack_value(self,datalist):
        try:
            bccref = self.calculate_BCC( datalist , self.data_len-2 )
            if bccref == datalist[self.data_len-2]:
                tmpmsg = bytes(datalist)
                tmpmsg = struct.unpack(self.recvformat,tmpmsg)
                self.msg.update( zip(self.msg.keys(),tmpmsg ) )
            else:
                return False
        except Exception as e:
            return False

        return True

"""
将需要接收的数据解包 类
"""
class Uart_SendPack():
    def __init__(self,packmsg,dataformat):

        # 传输的数据包
        self.msg = packmsg

        # 传输的数据格式
        self.sendformat = dataformat

        # 要传输的数据长度
        self.data_len = struct.calcsize(self.sendformat)


    # BCC校验函数
    def calculate_BCC(self,datalist,datalen):
        ref = 0
        for i in range(datalen):
            ref = (ref^datalist[i])
        return ref&0xff

    # 将要打包的数据进行BCC校验,并返回最终BCC校验的值
    def pack_BCC_Value(self):
        tmp_list = list( self.msg.values() ) # 将字典的值取出转换成列表

        tmp_packed = struct.pack(self.sendformat,*tmp_list) # 根据指定的数据类型对列表值进行自动打包

        # 自动打包后的数值，进行BCC校验，获得最终需要发送的BCC校验值
        return self.calculate_BCC(tmp_packed,len(tmp_packed)-2)

    # 获取最终要发送的数据列表
    def get_Pack_List(self):
        tmplist = list(self.msg.values())# 将字典的值取出,转换成列表
        return struct.pack(self.sendformat,*tmplist)

""" 创建需要往stm32端发送数据的数据包 """
# 需要发送的数据(如果需要增删数据,则修改此处以及修改数据格式,同时在32端也对应增删即可)
send_pack1_msg = OrderedDict([
        ('Head',0xCC),  # 帧头           uint8_t类型
        ('Cam_W', 320), # 相机的像素宽度  uint16_t 类型
        ('Cam_H', 240), # 相机的像素长度  uint16_t 类型
        ('follow_x',0),  # 需要跟踪的点x   uint16_t 类型
        ('color',0),  # 需要跟踪颜色   uint16_t 类型
        ('size',0),
        ('BccCheck',0), # 数据BCC校验位   uint8_t类型
        ('End',0xDD)    # 帧尾            uint8_t类型
])

# 数据格式 <代表小端模式, B代表uint8_t类型,4H代表4个uint16_t类型,2B代表2个uint8_t类型
send_pack1_format = "<B5H2B"

#实例化数据打包对象
send_pack1 = Uart_SendPack(send_pack1_msg,send_pack1_format)

# 更新需要发送的数据并返回打包结果,将结果直接发送到stm32端即可
def update_sendpack1_data(follow_x,follow_y,size):
    global send_pack1
    send_pack1.msg['follow_x'] = follow_x
    send_pack1.msg['color'] = color
    send_pack1.msg['size'] = size
    send_pack1.msg['BccCheck'] = send_pack1.pack_BCC_Value()
    return send_pack1.get_Pack_List()
""" 创建需要往stm32端发送数据的数据包 END """


# 各传感器初始化
lcd.init()
sensor.reset()
sensor.reset(freq=24000000, dual_buff=1)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)

sensor.set_auto_gain(False)      # 颜色跟踪必须关闭自动增益
sensor.set_auto_whitebal(False)  # 颜色跟踪必须关闭白平衡

sensor.skip_frames(time = 1000)

#帧率时钟
clock = time.clock()

# 指定串口引脚并构造UART对象
fm.register(0, fm.fpioa.UART1_RX)
fm.register(1, fm.fpioa.UART1_TX)
uart1 = UART(UART.UART1, 115200)

color_thresholds = [
    (0, 100, 50, 127, -50, 50),#red
    (0, 100, -128, -50, -50, 50),#green
    (0, 100, -50, 50, 30, 127)#yellow
]
color_show=[(255,0,0),(0,255,0),(255,255,0)]


# 追踪的颜色选择
cmd_color = 0

# 辅组接收数据的变量
Last_recv32 = None


# 感兴趣的区域设置:x,y,w,h
# 像素共有：320x240
roi = (40, 200, 240, 40)


g_follow_blob = None
g_last_blob = None
g_errortime = 0
color=-1

# 定时器中断回调函数
def timer_callback(timer):
    global g_follow_blob
    global g_last_blob
    global g_errortime

    # 最大最小面积定义(通过实测获得)
    min_area = 500
    max_area = 2500

    sendcx = 0
    sendcolor = 0
    sendarea = 0
    try:
        # 将最大物体的中心坐标发送到stm32
        if g_follow_blob!=None:
            # 计算识别到的面积
            sendarea = g_follow_blob.rect()[2]*g_follow_blob.rect()[3]
            sendcx = g_follow_blob.cx()
            sendcolor = color

        send_to_32 = update_sendpack1_data(sendcx,sendcolor,sendarea)
        uart1.write(send_to_32)

    except Exception as e:
        print(e)


# 创建一个定时器,period为定时时间,单位ms.
timer = Timer(Timer.TIMER0, Timer.CHANNEL0, mode=Timer.MODE_PERIODIC, period=50, callback=timer_callback)

# 启动定时器
timer.start()



while True:

    #用于计算帧率的函数，这里表示开始计时
    clock.tick()

    try:
        #从传感器捕获一张图像
        img = sensor.snapshot()

        # 框选感兴趣的区域用于提示
        img.draw_rectangle(roi, color=(255 ,255,255),thickness = 3)
        for cmd_color in range(3):
        # 根据色块阈值找出感兴趣区域的色块
            blobs = img.find_blobs([color_thresholds[cmd_color]], pixels_threshold=100, area_threshold=100, merge=True, margin=10,roi = roi)

            max_box_size = 0
            max_blob = None

            if blobs:
                color=cmd_color
                max_blob = blobs[0]
                for blob in blobs:
                    # 面积计算：blob.rect()[2]*blob.rect()[3]
                    if blob.rect()[2]*blob.rect()[3] > max_blob.rect()[2]*max_blob.rect()[3]:
                        max_blob = blob

                # 框选出识别到的色块
                g_follow_blob = max_blob
                img.draw_rectangle(g_follow_blob.rect(), color=(255 , 0,0),thickness = 3)
                img.draw_cross(g_follow_blob.cx(), g_follow_blob.cy(), color=color_show[cmd_color])

        # 显示帧率
        fps = clock.fps()
        #img.draw_string(0, 0, "color:%d" %(color), color=(0, 60, 255), scale=2.0)

        if max_blob!=None:
            img.draw_string(0, 20, "size:%d" %(g_follow_blob.rect()[2]*g_follow_blob.rect()[3]), color=(255, 60, 255), scale=2.0)

        lcd.display(img)

        #显示在LCD上

    except Exception as e:
        print(e)

