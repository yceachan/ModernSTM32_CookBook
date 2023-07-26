//
// Created by 衣陈 on 2023/5/4.
//
/*一个对UART外设的驱动类，包括三级中断的DMARX接受数据。通信部分后续计划采用Socket编程。*/
#include <cstring>
#include "UART.h"
#include "stm32f4xx_hal_dma.h"
#include "dma.h"
#include "LOG.hpp"
#include "HAL2bsp.h"

using namespace cpt;

extern "C"{
void UART5_IRQHandler(void) {
    if(__HAL_UART_GET_FLAG(&huart5,UART_FLAG_IDLE)){
        uint32_t  ndtr = __HAL_DMA_GET_COUNTER(huart5.hdmarx);//ndtr
//        print("uart_ilde_It\n");
        __HAL_UART_CLEAR_IDLEFLAG(&huart5);
        ((UartDev*)HAL2Dev[&huart5])->m_dmarx->Rxcallback(ndtr);
    }
    if(__HAL_UART_GET_FLAG(&huart5,UART_FLAG_FE)){
        print("\nuart5_fe_error");
        __HAL_UART_CLEAR_FEFLAG(&huart5);
    }
    if(__HAL_UART_GET_FLAG(&huart5,UART_FLAG_NE)){
        print("\nuart5_ne_error");
        __HAL_UART_CLEAR_NEFLAG(&huart5);
    }
    if(__HAL_UART_GET_FLAG(&huart5,UART_FLAG_ORE)){
        print("\nuart5_ore_error");
        __HAL_UART_CLEAR_OREFLAG(&huart5);
    }
}

void DMA1_Stream0_IRQHandler(void)
{
    uint32_t  ndtr = __HAL_DMA_GET_COUNTER(huart5.hdmarx);
    //LISR_bit4,半满中断
    if(READ_BIT(DMA1->LISR,DMA_FLAG_HTIF0_4)){
//        print("\ndma half It");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_HTIF0_4);   //清标志位
        ((UartDev*)HAL2Dev[&huart5])->m_dmarx->Rxcallback(ndtr);
    }
    //LISR_bit5,溢满中断
    if(READ_BIT(DMA1->LISR,DMA_FLAG_TCIF0_4)){
//        print("\ndma full It");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_TCIF0_4);  //清标志位
        ((UartDev*)HAL2Dev[&huart5])->m_dmarx->Rxcallback(ndtr);
    }
    if(READ_BIT(DMA1->LISR,DMA_FLAG_FEIF0_4)){
        print("\ndmarx5_fe_error");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_FEIF0_4);  //清标志位
    }
    if(READ_BIT(DMA1->LISR,DMA_FLAG_DMEIF0_4)){
        print("\ndmarx5_dme_error");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_DMEIF0_4);  //清标志位
    }
    if(READ_BIT(DMA1->LISR,DMA_FLAG_TEIF0_4)){
        print("\ndmarx5_te_error");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_TEIF0_4);  //清标志位
    }
}
void USART3_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE)){
        uint32_t  ndtr = __HAL_DMA_GET_COUNTER(huart3.hdmarx);//ndtr
//        print("uart_ilde_It\n");
        __HAL_UART_CLEAR_IDLEFLAG(&huart3);
        ((UartDev*)HAL2Dev[&huart3])->m_dmarx->Rxcallback(ndtr);
    }
    if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_FE)){
        print("\nuart3_fe_error");
        __HAL_UART_CLEAR_FEFLAG(&huart3);
    }
    if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_NE)){
        print("\nuart3_ne_error");
        __HAL_UART_CLEAR_NEFLAG(&huart3);
    }
    if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_ORE)){
        print("\nuart3_ore_error");
        __HAL_UART_CLEAR_OREFLAG(&huart3);
    }
}
void DMA1_Stream1_IRQHandler(void)
{
    uint32_t  ndtr = __HAL_DMA_GET_COUNTER(huart3.hdmarx);
    //LISR_bit4,半满中断
    if(READ_BIT(DMA1->LISR,DMA_FLAG_HTIF1_5)){
//        print("\ndma half It");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_HTIF1_5);   //清标志位
        ((UartDev*)HAL2Dev[&huart3])->m_dmarx->Rxcallback(ndtr);
    }
    //LISR_bit5,溢满中断
    if(READ_BIT(DMA1->LISR,DMA_FLAG_TCIF1_5)){
//        print("\ndma full It");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_TCIF1_5);  //清标志位
        ((UartDev*)HAL2Dev[&huart3])->m_dmarx->Rxcallback(ndtr);
    }
    if(READ_BIT(DMA1->LISR,DMA_FLAG_FEIF0_4)){
        print("\ndmarx3_fe_error");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_FEIF1_5);  //清标志位
    }
    if(READ_BIT(DMA1->LISR,DMA_FLAG_DMEIF1_5)){
        print("\ndmarx3_dme_error");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_DMEIF1_5);  //清标志位
    }
    if(READ_BIT(DMA1->LISR,DMA_FLAG_TEIF1_5)){
        print("\ndmarx3_tei_error");
        ATOMIC_SET_BIT(DMA1->LIFCR,DMA_FLAG_TEIF1_5);  //清标志位
    }
}
}

/* @name:Rxdma_start
 * @parm:huart：传入CUBE初始化的串口句柄，应已使能NVIC，已配置好hdmarx成员（dma外设已注册stream的模式，源指针）。
 * */
void UartDev::Rxdma_start(uint32_t rx_fifo_depth) {

    this->m_dmarx= new DmaRxImpl(rx_fifo_depth);
    //set dmastreamX:huart.RX to memory
    auto& pdmas=huart->hdmarx;

    /* Clear DBM bit 关闭双缓冲模式 */
    pdmas->Instance->CR &= (uint32_t)(~DMA_SxCR_DBM);

    /* Configure DMA Stream data length */
    pdmas->Instance->NDTR = this->m_dmarx->RX_FIFO_DEPTH;


     /* Peripheral to Memory */

     /* Configure DMA Stream source address */
     pdmas->Instance->PAR =(uint32_t)& huart->Instance->DR;

     /* Configure DMA Stream destination address */

     pdmas->Instance->M0AR = (uint32_t)this->m_dmarx->Rxbuf.pbuf;

    /*error IT check */
    __HAL_DMA_ENABLE_IT(pdmas,DMA_IT_DME | DMA_IT_TE);
    __HAL_UART_ENABLE_IT(huart , UART_IT_ERR );

    //pdmas->Instance->CR  |= DMA_IT_TC | DMA_IT_HT;
    __HAL_DMA_ENABLE_IT(pdmas,DMA_IT_TC |DMA_IT_HT );
    //pdmas->Instance->CR  |= DMA_SxCR_EN
    __HAL_DMA_ENABLE(pdmas);

    /* Enable the DMA transfer for the receiver request by setting the DMAR bit
     in the UART CR3 register */

    /*Enable the UART_IDLE_IT*/
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);

    ATOMIC_SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);



    ASSERT(__HAL_UART_GET_IT_SOURCE(this->huart,UART_IT_IDLE));
    ASSERT(__HAL_DMA_GET_IT_SOURCE(this->huart->hdmarx,DMA_IT_HT));
    ASSERT(__HAL_DMA_GET_IT_SOURCE(this->huart->hdmarx,DMA_IT_TC));

    return;
}

UartDev::UartDev(UART_HandleTypeDef *huart) : huart(huart){
    HAL2Dev[huart]=this;
}

void UartDev::Rxdma_pause() {
    auto& pdmas=this->huart->hdmarx;
    __HAL_DMA_DISABLE_IT(pdmas,DMA_IT_TC |DMA_IT_HT );
    __HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);
    ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);
}

void UartDev::Rxdma_resume() {
    auto& pdmas=this->huart->hdmarx;
    __HAL_DMA_ENABLE_IT(pdmas,DMA_IT_TC |DMA_IT_HT );
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    ATOMIC_SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);
}

DmaRxImpl::DmaRxImpl(uint32_t rx_fifo_depth)
        : RX_FIFO_DEPTH(rx_fifo_depth), Rxbuf(rx_fifo_depth) {
}

void DmaRxImpl::xfer_buf(int32_t lo, int32_t hi) {
    while(lo < hi){
        packbuf[pk_tail++] =this->Rxbuf.pbuf[lo++]; //复制正常
        if(packbuf[pk_tail-1] == '\n'){

            this->mesque.emplace(packbuf, pk_tail);
            pk_tail=0;

        }
    }
}

void DmaRxImpl::Rxcallback(uint32_t ndtr) {
    auto&  rpt = this->Rxbuf.rpt  , &wpt=this->Rxbuf.wpt;

    wpt = this->RX_FIFO_DEPTH - ndtr;

    if(rpt > wpt) {
        xfer_buf(rpt,this->RX_FIFO_DEPTH );
        rpt=0;
    }
    xfer_buf(rpt,wpt);
   rpt=wpt;

}
