Coterm-M——DWT微妙级延时案例

[ref:ARM-v7](../DDI0403E_e_armv7m_arm.pdf)

[ref:CoterxM4](../arm_cortexm4_processor_trm_100166_0001_04_en.pdf)

# 寄存器map

## Debug控制寄存器DEMCR

![image-20230530215249949](https://s2.loli.net/2023/05/30/vP9EoystQCqUYBw.png)

bit24置1，使能DWT

## DWT_CTRL控制器

![image-20230530215803283](https://s2.loli.net/2023/05/30/8vhfJjXZzqG3WYC.png)

![image-20230530220057500](https://s2.loli.net/2023/05/30/9yFdzBl5DASGiT6.png)

## DWT_CYCCNT主时钟计数器

![image-20230530220722492](https://s2.loli.net/2023/05/30/y4q7Zadc9pVeDQ3.png)

![image-20230530220817999](https://s2.loli.net/2023/05/30/WZsu7V1CoNX8Ugf.png)

# 软件编程

1. 先使能DWT外设，写DEMCR的bit24
2. 使能CYCCNT寄存器之前，先清0。
3. 使能CYCCNT寄存器，这个由DWT的CYCCNTENA 控制，也就是DWT控制寄存器的bit0控制，写1使能
4. 此计数器以PC主频向上自重装载自增。

# Code

![image-20230531000216577](https://s2.loli.net/2023/05/31/Vi7qr9WXljwCdPe.png)