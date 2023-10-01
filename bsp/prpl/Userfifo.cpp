//
// Created by 衣陈 on 2023/10/1.
//

#include <cstdarg>
#include <cstdio>
#include <utility>
#include <cstring>
#include "Userfifo.h"
#include "main.h"
#include "usart.h"

void Userfifo::print(const char *fmt, ...) {
    va_list args;
    uint32_t len;
    va_start(args,fmt);
    //将格式化的数据从可变参数列表写入指定大小缓冲区，以空字符填充。返回不包括空字符的写入字符数。
    len = vsnprintf((char *)tx_fifo , TX_SIZE , (char *)fmt , args);
    va_end(args);
    render(tx_fifo , len);
}

Userfifo::Userfifo(uint8_t *_tx_fifo, uint32_t _TX_SIZE,
                   uint8_t *_rx_fifo0, uint32_t _RX_SIZE,
                   uint8_t *_rx_fifo, uint32_t &_rx_sta,
                   void (*_render) (uint8_t *, uint32_t))
        : tx_fifo(_tx_fifo)   , TX_SIZE(_TX_SIZE),
          rx_fifo0(_rx_fifo0) , RX_SIZE(_RX_SIZE) ,
          rx_fifo(_rx_fifo)   , rx_sta(_rx_sta) ,
          render(_render){

}

void Userfifo::clear() {
    memset(this->rx_fifo , 0 ,sizeof (this->rx_fifo ));
    this->rx_sta =0;
}


