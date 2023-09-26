一款商用温湿度传感器，

湿度测量标称容差2%RH，响应时间8s，恢复时间未标注，测试超过30s。

最大采样速率10sps左右

7位地址16位数据的IIC总线设备

为软件IIC拓展以下通信逻辑

其IIC驱动不是寄存器风格，而是command。

`cmdwrite(u8 cmd)；` //写u8

`seqWR(u8* tx ,u32 tx_len,u8*rx,r32 rx_len)` //写后读



软件主要配置参考datasheet

`4 Operation and Communication`

可配置单次/周期采样模式、测量精度；

自加热指令；