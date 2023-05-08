>  经多方排除地，一度以为是模块烧了地，笔者手上的模块的板载电感容差为10%，电感L会在LC震荡换算公式中乘上较大的系数，使得测量误差很夸张。但这个FDC测的还是很稳很快的，就是永远找不到真值咯。
>
>  f可作时钟。故频率是可用定时器测量

![image-20230405182446885](C:\Users\yceachan\AppData\Roaming\Typora\typora-user-images\image-20230405182446885.png)

**笔者的评估板除了soc上集成的33pf贴边电容外，还并联了一块电容值未知的金属极板，用以做接触检测等。**

**此外，师兄焊了个容差为10%的电感上去，就只能当大玩具了**

# SYS框图

![image-20230309235332784](https://s2.loli.net/2023/03/09/3kVeRn5m8LzGKZy.png)

$F_{CLK}=40Mhz$，使用内置时钟源

![image-20230309234745616](https://s2.loli.net/2023/03/09/SrHswRi1oBU2JGp.png)

模块上集成一个贴片电感，一个贴片电容，一个金属极板。可采用**差分方式Differential**测量外部电容，暨，在INAB并联上待测量电容得$L(C_{cap} + C_{tar})$ 振荡器，其震荡出$f_{SENSOR}$，如上所述f可做时钟用以测量。



$f_{SENSOR}$经FIN_SEL分频（寄存器配置），向CORE输入$f_{IN}=f_{SENSOR0 }/ CHx\_FIN\_SEL$

## **传感端**

![image-20230310000848359](https://s2.loli.net/2023/03/10/SY1nCB9Da6IcdxA.png)

通常，FDC上的LCtank为 18uH，33pF，得6.5Mhz的f~CAP~

(非TI规范) 

jlc原理图如下

![image-20230310004427175](https://s2.loli.net/2023/03/10/SYBJ45FMNVZ8rvw.png)

## 输出&滤波端

不带分频因子

![image-20230310005406155](https://s2.loli.net/2023/03/10/H6qYgdNsC7mTcAl.png) 

带分频因子

![image-20230310005901424](https://s2.loli.net/2023/03/10/aTidyMVm3EkFjCL.png)



**Gain and OFFSET 数字增益和偏移调整**

* C = sensor capacitance including parasitic PCB capacitance C为全寄生电容包括PCB寄生电容，不包括模块集成LCtank，

* 在本公式中，Csensor指

  ==当然2214已经做好了这部分滤波，所以看个乐呵即可==

![image-20230310002952284](https://s2.loli.net/2023/03/10/5vxSotgjMBKfX7V.png)

**Clock cofiguration requirement**

![image-20230309235019215](https://s2.loli.net/2023/03/09/QKF1rapB9xWSyVu.png)

![image-20230309235140436](https://s2.loli.net/2023/03/09/uVoL9ZF5wRleGbT.png)

**最终换算公式**

$f_{SENSOR}=FIN_{SEL}*f_{REF}*DATAx  /2^{28}=\frac{1}{2\pi \sqrt{L(C_{cap}+C{tar})}}$

$L=18uH,C_{cap}=33pf$

Csensor=.....

## 后端——fIN计数器

![image-20230414192622428](https://s2.loli.net/2023/04/14/KdjkIsiWO5nENb4.png)

fdc文档里好像没有fin测量原理这一段，但根据传感器返回结果是fin与fref的比值，以及RCOUT寄存器配置这些，很好想f是怎么测出的。

# 寄存器map暨软件编程内容

~~~cpp
void Fdc::singleinit() {
    //复位命令寄存器，当写入1在最高位时复位
    this->regWrite(RESET_DEV,0x8000);

    uint8_t did[2]={0};
    //设备ID寄存器
    this->regRead(DEVICE_ID, 2, did);   
    uint16_t  DID = (did[0] << 8 | did[1] );  //整形提升
    LOG(DID);//读出0x3055符合预期

    uint8_t mid[2]={0};
     //制造ID寄存器，两个自检
    this->regRead(MANUFACTURER_ID, 2, mid);   
    uint16_t  MID = (mid[0] << 8 | mid[1] );  //整形提升
    LOG(MID);//读出0x5449符合预期

    //测量周期配置寄存器：测量RCOUNT*16个fref周期，
    this->regWrite(RCOUNT_CH2,0x0200);//100us/sample ::measureTime=4k*Tref(40m^-1) , reg_DATA ::0xFA * 16u * Tref =2kTREF=100us/sample
    
    //等待沉降时间寄存器：LC启震，需要时间稳定，芯片通过写死等待时间来滤去这段时间的f
    this->regWrite(SETTLECOUNT_CH2,0x002C); (T=0x0a * 16 /fREF)//等待沉降时间推算CHx_SETTLECOUNT > Vpk × fREFx × C × π /32 /IDRIVE=4.499 等待沉降时间4us
    
    //时钟配置寄存器
    this->regWrite(CLOCK_DIVIDERS_C_CH2,0x1002);//Fin1分频，fREF1分频 10mhz
    //电流驱动寄存器
    this->regWrite(DRIVE_CURRENT_CH2,0xF000);//电流驱动:1.xxmA（传感器时钟建立+转换时间的驱动电流,驱动电流da，转换快）

    //通道选取与滤波配置寄存器
//    this->regWrite(MUX_CONFIG,0x820D);//多通道模式，滤波带宽10mhz
    this->regWrite(MUX_CONFIG,0x020D);//单通道模式，滤波带宽10mhz
    
    //一些杂七杂八还有解除休眠的寄存器。
    this->regWrite(CONFIG,0x9C01);
}

//还有一个通道数据READY寄存器
 do {
        this->regRead(STATUS,2,rx);
        UNREADCONV=rx[1] & ( CH?(0b00000100):(0b00000010) );
        //降低轮询频率，防止IIC错误
        delay_us(100);
    }while(!UNREADCONV);
~~~

具体配置还是请参阅IC手册。文档**10.2 Typical Application**中有推荐寄存器配置。

文档**9.4 Device Functional Modes**中提及模块上电后处于休眠模式，需写CONFIG寄存器解除

（问就是踩过坑呜呜~~）

![image-20230414194847819](https://s2.loli.net/2023/04/14/Nbfs9EUMLq7YGwC.png)

![image-20230414195413754](https://s2.loli.net/2023/04/14/XsDTNmw7iVJnC8A.png)

# IIC时序要求

400khz fast mode 如下

![image-20230411125440601](https://s2.loli.net/2023/04/11/mIlpxH8NT7bOXnZ.png)