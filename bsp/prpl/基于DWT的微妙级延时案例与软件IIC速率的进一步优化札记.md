​	在["软件IIC通讯速率札记.md"](软件IIC通讯速率札记.md)中，笔者 曾对软件IIC通讯速率感到非常疑惑，现通过DWT编程完成微妙延时，并对软件速率给出解释。

非常值得参阅的[C++性能优化指南.pdf](C++性能优化指南.pdf)

# 1.DWT的微妙级延时编程

DWT是Cortex-M4内核的数据监视单元，IP内包括一个随MCU主时钟自向上计数，软件可读写的32位计时器，适合用以进行微妙级的延时操作或对代码性能进行检测。

详见ModernSTM32_CookBook

> Use of CMSIS compiler intrinsics for register exclusive access
> Atomic 32-bit register access macro to set one or several bits
>
> ——by Struct.aligned

![image-20230531000216577](https://s2.loli.net/2023/05/31/Vi7qr9WXljwCdPe.png)

# 2.理论值分析-392us/250khz

首先，上篇札记的估值有误，测速函数如下:

![image-20230531001040240](https://s2.loli.net/2023/05/31/42nhZSI9EzM87Vt.png)

![image-20230531001106902](C:\Users\yceachan\AppData\Roaming\Typora\typora-user-images\image-20230531001106902.png)

 7bit-adr 16bit-DATA 的IIC设备执行一次regRead逻辑，有三个sendByte两个readByte时序，共5*9+4=49个SCL时钟

总共98时钟，其中SCL配速为250khz，理论延时392us

> 如图的250mhz配速

![image-20230531001359073](https://s2.loli.net/2023/05/31/oaRgFwUqOzT4VDH.png)

# 3.测量值与误差分析-616us/155khz

![image-20230531001636590](https://s2.loli.net/2023/05/31/RYdbVA5mvtqfDBF.png)

> @衣陈：
>
> **使用1mhz和84mhz的普通计时器计时，实际测量结果均为616us**，折合IIC通讯频率155khz
> 这个结果和使用普通定时器作为延时时钟源，使用宏操作寄存器的结果一致（626us）
> 所以应该可以认为约有250us的程序运行开销了，即，一个u8的软件IIC通讯，程序有25us开销

**Q：这样一个for循环，在168mhz下，至于要跑25us吗**

![image-20230531001722512](https://s2.loli.net/2023/05/31/v86FmVYwBR3LTWO.png)



**A**:实验->对GPIO的操纵改为手写寄存器，去HAL封装

> 在ASSERT位空下，实际相差仅为一层函数调栈

![image-20230531001911912](https://s2.loli.net/2023/05/31/zCYTKyiptUVG9ON.png)

![image-20230531003121385](https://s2.loli.net/2023/05/31/TLUO94znoldRIsZ.png)

如图，616us-556us

在两次regRead中，约有300多次IO操作，这三百多次调栈合出了60us的程序开销

-->在f407的168mhz下，调栈开销约为：200ns/call

简单叠加一下，就是50us的债，让你的软件IIC永远无法达到real time。

由这个实验，我们可以认为，250us的程序开销，倒也合理。

# 4.最终结果与小结：556us/172khz

“算法优化是永恒的课题”

一些看起来微不足道的细节，放慢在168mhz的realtime裸机下，置于热点语句内，其性能损耗足以令你否定你的秉心渊塞

by the way,这个软件IIC框架，最初是**872us/110khz**

![image-20230531004016454](https://s2.loli.net/2023/05/31/qaS61CO39f2KdJM.png)

![image-20230531004853396](https://s2.loli.net/2023/05/31/UZLqr5bSNB23746.png)