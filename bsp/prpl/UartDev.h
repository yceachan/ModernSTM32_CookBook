//
/*
 * Uart驱动类，继承自Userfifo类，配备fifo资源，实现RXNE中断的回调与双级缓冲。
 * TODO 1.为外设声明全局对象puartX,绑定HAL库指针与阻塞TX方法，new缓冲区资源
 * TODO 2.使用rx_start_it，以启动RXNE中断
 * TODO 3.定义 extern "C" Cmisis-USARTxIRQ 函数，在其中调用对应全局对象puartX的回调
 * TODO 4.回调函数已处理双缓冲逻辑，以"\n"结尾为一帧数据包，转移到后级fifo。用户此层访问rx_fifo并调用clear方法清空rx_fifo.
 * TODO 5.使用print方法，格式化发送bin数据包。
 *
 * !待实现基于DMA的收发控制。
 */
//

#ifndef VPCTRAIL_UARTDEV_H
#define VPCTRAIL_UARTDEV_H

#include <map>
#include "usart.h"
#include "main.h"
#include "Userfifo.h"



class UartDev : public  Userfifo{
public:
    UartDev(uint8_t *_tx_fifo, uint32_t _TX_SIZE,
            uint8_t *_rx_fifo0, uint32_t _RX_SIZE,
            uint8_t *_rx_fifo, uint32_t &_rx_sta,
    void (*_render) (uint8_t *, uint32_t),
            UART_HandleTypeDef *_huart);
    UART_HandleTypeDef *huart;
    void rx_start_it();
    void rx_callback(uint8_t rxd);
};
extern UartDev puart1;
extern std::map<UART_HandleTypeDef*,UartDev*> phuart;
#endif //VPCTRAIL_UARTDEV_H
