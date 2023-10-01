

[(30条消息) 一个严谨的STM32串口DMA发送&接收（1.5Mbps波特率）机制_stm32 usart dma接收发送_Acuity.的博客-CSDN博客](https://blog.csdn.net/qq_20553613/article/details/108367512)

软件目标：设计一个高可用的串口接受不定长数据框架，在尽可能少的中断使用下完成数据从UART_DR到应用层的转发，需要具有不漏包，不粘包。

解决方案：DMA接受器：串口空闲中断+DMA循环模式，传输半满中断+溢满中断。

乒乓缓冲架构：UART_DR —> DMA_Rxbuf —>MemRxSpan

—要求：对数据的完善的消费能力，跟得上DMA_RX的生产能力：配置三层中断及时处理数据

* **UART_DR —> DMA_Rxbuf :**

  为DMA配置FIFO_DEPTH长度的Rx循环传输，其将在DMA_Rxbuf上循环地生产数据。

  —要点，在数据写入缓冲区后的三个中断中，管理好写指针。



* **DMA_Rxbuf —>MemRxSpan：**

  使用串口空闲中断，传输半满中断，溢满中断，三级中断及时地消费Rx_buf上的循环缓冲区数据，将其转移到MemRxfifo中。

  —转移策略：

  ——1 `memset`常数小，别界不好搞，难以判断包尾`endl` 

  ——2 `for each` 常数大，易整定边界，易寻找包尾`endl`

  —要点：要在三个中断中，好生管理循环缓冲区Rxbuf的读写指针
