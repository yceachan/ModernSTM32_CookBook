//
// Created by 衣陈 on 2023/10/1.
//

#ifndef VPCTRAIL_USERFIFO_H
#define VPCTRAIL_USERFIFO_H
/*
 * 配备收双缓冲的fifo结构，为fifo提供面向对象封装，实现格式化发送
 * 缓冲区资源通过指针配备，可以绑定外部变量，也可以在构造函数中new出来。
 * 本身不处理缓冲逻辑，仅为数据的读写提供接口 print ，clear ,rx_sta
 * 可以使实际通信设备集成此基类，并完善缓冲的回调接口。
 * 实例化本基类，需要提供一个阻塞render函数指针 void (*)(uint8_t* txbuf , uint32_t len) ,它可以由lamda构造。
 * render方法需能够像目标通信处发送已指明长度的raw bin data ， print方法格式化生成数据，并调用render方法。
 * TODO 在 eabi gcc中 ，尝试用std::function 作为render回调，但“无法找到路径”
 */
#include "main.h"
class Userfifo{
public:
    Userfifo(uint8_t *_tx_fifo, uint32_t _TX_SIZE,
             uint8_t *_rx_fifo0, uint32_t _RX_SIZE,
             uint8_t *_rx_fifo, uint32_t &_rx_sta,
             void (*_render) (uint8_t *, uint32_t));
    uint8_t * rx_fifo ; uint32_t& rx_sta;  const uint32_t  RX_SIZE;


    void print(const char *fmt, ...);
    void clear();
private:
    void (*render) (uint8_t *, uint32_t);
    uint8_t * tx_fifo ; const uint32_t  TX_SIZE;

protected:
    uint8_t * rx_fifo0 ;
};


#endif //VPCTRAIL_USERFIFO_H
