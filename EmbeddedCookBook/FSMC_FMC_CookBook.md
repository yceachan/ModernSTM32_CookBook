# 综览

**FSMC**：**Flexible static memory controller** 灵活的静态存储控制器

* **最高支持16位并口，可操纵如8080时序的LCD驱动IC**

**FMC**： **Flexible  memory controller**  灵活存储控制器

* **最高32位并口，可驱动RGB接口（三通道24分量）**

==可编程时序，故可 “Flexible” 驱动广义外部存储器==



fsmc和fmc是==stm32的特色外设/存储器扩展技术==，FMC见于高级些的mcu(F42++)。

![image-20230414001908596](https://s2.loli.net/2023/04/14/YzKqEud5I1XsZ2v.png)



》FSMC 能够连接同步、异步存储器和 16 位 PC 存储卡。其主要用途如下：
● **将 AHB 数据通信事务转换为适当的外部器件协议**
● **满足外部器件的访问时序要求**

![image-20230414002214756](https://s2.loli.net/2023/04/14/HOdIXV5uSUt2ceb.png)

》一个很st的寻址移位

[STM32坑爹的FSMC总线16位数据对应的地址问题，头都晕了！ (amobbs.com 阿莫电子论坛 - 东莞阿莫电子网站)](https://www.amobbs.com/thread-5512680-1-1.html)

![image-20230414002745991](https://s2.loli.net/2023/04/14/pmAY5a1gCOJnByU.png)

# 存储器接口信号

![image-20230414011430710](https://s2.loli.net/2023/04/14/W5xDRmk4N1tUvCH.png)

![image-20230414011921675](https://s2.loli.net/2023/04/14/PgSqWHUN4ckjl6K.png)

# 通用并行时序规则

![image-20230414011451864](https://s2.loli.net/2023/04/14/xNWhUz38ZiDGn6E.png)

# 可编程时序配置寄存器：WAIT,SET,HIZ,HOLD

![image-20230414011742574](https://s2.loli.net/2023/04/14/Puq93aDyIwoF5V7.png)

![image-20230414011536274](https://s2.loli.net/2023/04/14/PHgCk9p83Ffl4ho.png)

![image-20230414011539648](https://s2.loli.net/2023/04/14/QfSjkLqBMzAxU26.png)

# FIFO寄存器

![image-20230414011903183](https://s2.loli.net/2023/04/14/asd4Sx8OYF6hkwv.png)

# 突发传输模式

![image-20230414013532192](https://s2.loli.net/2023/04/14/QmYXgjrObS8HokE.png)

![image-20230414013535401](https://s2.loli.net/2023/04/14/qyTA7Wga5mSenIR.png)

![image-20230414093343794](C:\Users\yceachan\AppData\Roaming\Typora\typora-user-images\image-20230414093343794.png)