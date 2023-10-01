# 直接内存访问

本札记中，称 内存直接访问（Direct Memory Access）为DMA，指这种数据传输技术

称dma控制器此外设为dma

称凭借DMA技术的一段IO序列为 **一次**DMA事务，**一段**DMA传输

称DMA传输序列中的单位传输事件为**一个**DMA传输 （包括突发模式下的打包传输）

**框图**

![image-20230411222048309](C:\Users\yceachan\AppData\Roaming\Typora\typora-user-images\image-20230411222048309.png)

# DMA 事务和 DMA 传输

**DMA 事务**
DMA 事务是一次完整的 i/o 操作，例如应用程序的单次读取或写入请求。

**DMA 传输**
DMA 传输是单个硬件操作，用于将数据从计算机内存传输到设备或从设备传输到计算机内存。

**单个 DMA 事务始终包含至少一个 DMA 传输，但一个事务可包含多个传输。**

当基于框架的驱动程序收到 i/o 请求时，驱动程序通常会创建一个 DMA 事务对象来表示请求。 当框架开始为事务提供服务时，它确定设备是否可以在单个传输中处理整个事务。 如果事务太大，则该框架会将事务分为多个传输。

> 以下以dma指代DMA控制器（外设），DMA指代直接内存访问技术

对于数据流的单位搬运（DMA事务中的一次DMA传输），dma寄存器内明确配置好**本次原子数据的收发地址**（地址寄存器与基于NDTR项的偏移量），==在dma得到DMA请求信号后==，dma访问总线搬运数据。（与cpu同级，待仲裁）

* 注意，当DMA传输发生时，cpu无法对内存进行io操作。
*  DMA_SxNDTR 计数器在完成本次事件后递减，记录剩余传输数目

## **dma控制器的DMA传输时序**

DMA传输发生（上次传输完成或软件开启一段全新事物）：：

外设向**dma发送请求**信号(request signal),dma根据通道优先级仲裁==》dma访问外设（将数据携至总线）

==》dma**立即**向外设发送**应答信号**(Acknowledge signal)，外设接受后释放请求信号，请求信号释放后，dma释放应答信号，本次传输结束。

**显然待外设处理完数据后，下次DMA事件才从外设的请求信号开始。**

## 外设的DMA传输请求映射

> f42x以上的架构，看个乐呵即可
>



![image-20230411223150379](C:\Users\yceachan\AppData\Roaming\Typora\typora-user-images\image-20230411223150379.png)

## 外设的DMA传输请求生成器

外设的DMA请求由**外设的触发事件**调制至请求通道

触发事件原理就终于溯源了。

[参阅文档]("C:\Users\yceachan\Desktop\ModernSTM32_CookBook\peripheralCookBook\DMA请求器_ARM文档.pdf")

![image-20230411231348899](https://s2.loli.net/2023/04/11/CmzrBEKiW64UReH.png)

# stm32f4_dma硬件书

## dma控制器硬件资源

有两个dma外设，分别：

有一组状态寄存器，统一管理下属流控制器的状态（中断标志位与清中断器）

有流控制器若干，一个dma控制器可且仲裁式地并行处理若干传输流(dma stream)

![image-20230411223150379](https://s2.loli.net/2023/05/08/dZDOzpSmMbJYuns.png)

每个流控制器可以配置通道，优先级，源/目标地址，传输项，传输模式，中断选项等等。

每个流下有一个4 word 长度的 FIFO缓存，可用于直接模式下的数据缓存 和突发模式下的打包发送

(不放寄存器图了)

## dma控制器软件任务

![image-20230413003906392](C:\Users\yceachan\AppData\Roaming\Typora\typora-user-images\image-20230413003906392.png)

## DMA传输硬件时序

![image-20230413003924749](https://s2.loli.net/2023/04/13/jMIfASK1w8l2NEy.png)

## HAL_UART_DMA_TX 时序分析

* HAL_UART_DMA_TX框架

![image-20230413003713468](https://s2.loli.net/2023/04/13/8eXjTaOhu5iDCd2.png)

![image-20230413003720190](https://s2.loli.net/2023/04/13/JLhaH2AquV5Fypw.png)

* UART字符状态寄存器

> 是对byte的状态置位，用以产生DMA请求

![image-20230413004003893](https://s2.loli.net/2023/04/13/4AgjfJdkaXcQeRh.png)

![](https://s2.loli.net/2023/04/13/4AgjfJdkaXcQeRh.png)



# dma控制器传输地址与模式配置

![image-20230508132828888](https://s2.loli.net/2023/05/08/ZLI5JmDSudfo4Nq.png)

## 直接模式下的FIFO缓存

在直接模式下（当 **DMA_SxFCR 寄存器**中的 **DMDIS 值**为“0”时），不使用 FIFO 的阈值级别。==一旦使能了数据流，DMA 便会预装载第一个数据，将其传输到内部 FIFO。==一旦外设请
求数据传输，DMA 便会将预装载的值传输到配置的目标。然后，==它会使用要传输的下一个数据再次重载内部空 FIFO。==预装载的数据大小为 DMA_SxCR 寄存器中 PSIZE 位字段的值。
只有赢得了数据流的仲裁后，相应数据流才有权访问 AHB 源或目标端口。系统使用在**DMA_SxCR 寄存器** PL[1:0] 位中为每个数据流定义的**优先级执行仲裁。**



(**故直接模式也使用Fifo缓冲数据，所以不用太担心直接模式下丢数据**

![image-20230508133041294](https://s2.loli.net/2023/05/08/FSke2QaDLzUGv3O.png)

## 指针自增Trick:

DMA传输地址可以有一个自增trick：源或目的地为dma流控制器的地址寄存器中存储地址加上基于NDTR（待传输数据项）的偏移量

## 循环模式trick：

对于多段dma事务，每完成一段DMA事务(CNDTR清零)，DMA自动重装载配置NDTR寄存器

> DMA_InitStructure.DMA_MemoryBaseAddr =&RxBuff[0];   
>
> 当先前的CNDTR清零后，由硬件重装载CNDTR和目标源地址（自增trick）

## 双缓冲区模式：

此模式下自动使能循环模式

> 除了有两个存储器指针之外，双缓冲区数据流的工作式与常规（单缓冲区）数据流的一样。
>
> 在一次事务结束后更换dma的传输地址。
>
> 允许软件在处理一个存储器区域的同时，DMA 传输还可填充/使用另一个存储器区域。

![image-20230418233153308](https://s2.loli.net/2023/04/18/ELX8fxSGlR1aJDy.png)

![image-20230418233539935](https://s2.loli.net/2023/04/18/Ioc2zxlwCGUEWH1.png)



## 突发模式下的FIFO阈与节拍：

这里ST文档写的比较丑，总之要配置FIFO阈和节拍

**FIFO阈值**：FIFO深度是固定的4byte，无关阈值设定。

只是当FIFO已填充长度达到阈值时，才会触发一个DMA传输请求。而这一个DMA传输请求下的传输方式也是可以配置的，称为节拍

可配置的FIFO阈值有：1 , 2 ,3 ,4 words

**节拍**：即，(基于源数据宽度)的几项数据为一组，形成一段连续占用总线，不可被仲裁器打断的 DMA传输序列。

可配置的节拍有 ：single, 4 8 16

显然，节拍的配置与源和目标的宽度和FIFO的深度息息相关。

一次突发传输的节拍数*数据宽度等于实际传输的数据长度，不能超FIFO。
