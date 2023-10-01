//
// Created by 衣陈 on 2023/10/1.
//



#include <cstring>
#include "UartDev.h"
#define UART1_RX_SIZE  256
#define UART1_TX_SIZE  256

extern UartDev puart1(new uint8_t [UART1_TX_SIZE],UART1_TX_SIZE,
                      new uint8_t [UART1_RX_SIZE],UART1_RX_SIZE,
                      new uint8_t [UART1_RX_SIZE],*new uint32_t,
                       [](uint8_t* txbuf ,uint32_t len) {HAL_UART_Transmit(&huart1,txbuf , len,300);},
                       &huart1);
extern std::map<UART_HandleTypeDef*,UartDev*> phuart{{&huart1,&puart1}};
extern "C"{
void USART1_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_RXNE) ){

        uint8_t rxd = huart1.Instance->DR;
        __HAL_UART_CLEAR_FLAG(&huart1,UART_FLAG_RXNE);
        puart1.rx_callback(rxd);

    }
}
};
void UartDev::rx_start_it() {
    __HAL_UART_ENABLE_IT(huart ,UART_IT_RXNE );
}
//双缓冲的rx回调函数。
void UartDev::rx_callback(uint8_t rxd) {
    static uint32_t  fifo0_sta = 0;
   rx_fifo0[fifo0_sta] =rxd;

    fifo0_sta = (fifo0_sta + 1) % this->RX_SIZE;
   if(rxd =='\n'){
       memcpy(this ->rx_fifo + this ->rx_sta ,rx_fifo0,fifo0_sta);
       this -> rx_sta = fifo0_sta;
       memset(rx_fifo0 , 0 ,sizeof(rx_fifo0));
       fifo0_sta = 0;
   }

}

UartDev::UartDev(uint8_t *_tx_fifo , uint32_t _TX_SIZE,
                 uint8_t *_rx_fifo0 , uint32_t _RX_SIZE,
                 uint8_t *_rx_fifo ,uint32_t &_rx_sta,
                 void (*_render)(uint8_t *, uint32_t),
                 UART_HandleTypeDef *_huart)

    : Userfifo(_tx_fifo, _TX_SIZE,
                      _rx_fifo0,_RX_SIZE,
                      _rx_fifo, _rx_sta,
                      _render)
{
    this->huart=_huart;
}
