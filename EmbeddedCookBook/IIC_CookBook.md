# SWIIC_CookBook

## 建立时间与保持时间

**权威参考：[I2C Timing Characteristics (intel.com)](https://www.intel.com/content/www/us/en/docs/programmable/683771/current/i2c-timing-characteristics.html)**

![image-20230320220440907](https://s2.loli.net/2023/03/20/7PvbLCf8wulZHNQ.png)

对于一个通讯速率为400 kbit/s的IIC设备，其start，stop，ack，nack，waitack等时序所需要的信号保持时间如下：

| Description                                                  | Fast Mode |         |      |
| :----------------------------------------------------------- | :-------- | ------- | ---- |
|                                                              | **Min**   | **Max** |      |
| **SCL high period**                                          | 0.6       | —       | μs   |
| **SCL LOWperiod**                                            | 1.3       |         |      |
| **Setup time for serial data line (SDA) data to SCL**(从设置SDA到允许SCL采样) | 0.1       | —       | μs   |
| **Hold time for SCL to SDA data**      （从SCL负跳变到允许SDA变化设置数据） | 0         | 0.6     | μs   |
| **Setup time for a repeated start condition **SCL正跳变后到允许SDA负跳变（START信号） | 0.6       | —       | μs   |
| **Hold time for a repeated start condition** 从SDA负跳变到允许SCL负跳变（主机钳制时钟） | 0.6       | —       | μs   |
| **Setup time for a stop condition**从SCL正跳变时钟到允许SDA正跳变（结束信号） | 0.6       | —       | μs   |
| **SDA high pulse duration between STOP and START**（SDA跳变信号采样时间） | 1.3       | —       | μs   |



- setup time是指在时钟有效沿之前，数据输入端信号必须保持稳定的最短时间。SDA线上的数据必须在SCL上升沿之前保持稳定一段时间（即建立时间），以便接收器能够正确地读取数据。

* hold time是指在时钟有效沿之后，数据输入端信号必须保持稳定的最短时间。hold time时序检查确保新数据不会在触发器稳定输出初始数据之前过早到达D端而覆盖其初始数据。(因为采样电平到寄存器需要时间)

## is delay necessary between SDA and SCL?

> 看看下放start时序的经典实现，SDA置1到SCL置1并没有延时，这是否合理呢？ 注意，[SDA数据到SCL的建立时间：0.25 μs](https://www.intel.com/content/www/us/en/docs/programmable/683771/current/i2c-timing-characteristics.html)[1 ](https://www.intel.com/content/www/us/en/docs/programmable/683771/current/i2c-timing-characteristics.html) （很短）

~~~cpp
void IIC_Start(void)
{
    IIC_SDA(1);
    IIC_SCL(1);
    IIC_Delay();
    IIC_SDA(0);
    IIC_Delay();
    IIC_SCL(0);//此后钳住SCL总线准备通讯。
    IIC_Delay();
}
~~~

> @Eachan:
>
> ​     在很多软件IIC实现中，start时序内，SDA置1到SCL置1之间没有添加延时函数，这是否合理，考虑到SDA数据到SCL的建立时间不到1微妙
>
> @Newbing:
>
> 在很多软件IIC实现中，由于SDA数据到SCL的建立时间非常短（通常不到1微秒），因此在start时序内，SDA置1到SCL置1之间可能不会添加延时函数。这种做法在某些情况下是可以接受的。
>
> 但是，==**这种做法并不总是合理的**==。如果硬件平台的执行速度非常快，那么SDA数据到SCL的建立时间可能会过短，从而导致通信错误。因此，在实现软件IIC时，应该根据硬件平台的具体情况来确定是否需要添加延时函数。（比如我开的168Mhz？）