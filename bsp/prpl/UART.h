//
// Created by 衣陈 on 2023/5/4.
//
#ifndef IOT_F4TRAIL_UART_H
#define IOT_F4TRAIL_UART_H
#include "main.h"
#include "usart.h"
#include <vector>
#include <map>
#include<list>
#include <string_view>
#include <queue>
#include "component/RingBuf.h"
#include "component/MessQue.h"
class DmaRxImpl;
class UartDev{
public:
    UartDev(UART_HandleTypeDef* huart);
    UART_HandleTypeDef  * huart;
    DmaRxImpl* m_dmarx= nullptr;
    void Rxdma_start(uint32_t rx_fifo_depth);
    void Rxdma_pause();
    void Rxdma_resume();
};

class DmaRxImpl{
public:
    DmaRxImpl(uint32_t rx_fifo_depth);
    uint32_t RX_FIFO_DEPTH;
    cpt::RingBuf Rxbuf;
    std::queue<cpt::MessNode> mesque;
    void xfer_buf(int32_t lo , int32_t hi);
    void Rxcallback(uint32_t ndtr);
protected:
    size_t pk_tail=0;
    static const size_t PACK_FIFO_DEPTH=256;
    uint8_t packbuf[PACK_FIFO_DEPTH];
};

class L610Dev : public UartDev{
public:
    using UartDev::UartDev;
    void mqtt_IoTDA_HW();
    void messup(std::string_view text);

    void protup(std::string_view text);
    void ATlog(uint32_t delay = 10);
    inline void ATcmd(std::string_view cmd);
    void reset();
};

#endif //IOT_F4TRAIL_UART_H
