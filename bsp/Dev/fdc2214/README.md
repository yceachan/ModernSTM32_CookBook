# 1. FDC驱动报告

FDC2214系德州仪器TI推出的一款28位谐振式电容数字转换器，它采用测量LC谐振频率的方法测量端口电容值。该器件采用基于窄带的创新型架构，可对噪声和EMI进行高度抑制；支持宽激励频率范围；标称高达28位分辨率、4.08ksps采样率和250nF最大输入电容；且拥有低至0.3fF的系统噪声、35uA待机电流与2.1mA满载电流的低功耗特性。FDC的高采样率和优秀抗EMI特性保证了对电容值测量的优秀精确度和动态响应特性，是快速测量动态变化响应的理想方案。此外，其优秀的低功耗设计，为长时间实时监测的物联网应用环境提供良好的功耗支持。

> 》FDC电容测量电路原理图。固定电感值，谐振频率即为电容器的平方反比函数$f=\frac{1}{2\pi \sqrt{LC}}→ C=\frac{1}{(2 \pi f)^2L}$
>
> ![image-20230726002757947](https://s2.loli.net/2023/07/26/GXxnYPJKFEz2pbe.png)

part1是设计文档解读与驱动编写。

FDC文档私以为质量一般，频率计内核方面未做任何提及，驱动电流配置就振幅要求一概而过，原理和整定方法并无提及，而若想就FDC取得较好的传感效果，IDRIVE的整定是核心；对测量方案的介绍也是弯弯绕绕的，应用范例似乎有些笔误。

反正笔者初学时犯了挺多迷糊。

## 1.1 SYS框图

> 》 pic：框图：
>
> ![image-20230309234745616](https://s2.loli.net/2023/03/09/SrHswRi1oBU2JGp.png)

由前端LC电路接入部分（4通道，通道有AB两端），信号调理部分、数字频率计内核组成。内核部分Ti未开源。

应用FDC的核心环节是搭建LC振荡电路、配置测量参数和结果读取。

### 1.1.1 LC电路接入层

![image-20230310000848359](https://s2.loli.net/2023/03/10/SY1nCB9Da6IcdxA.png)

就LC电路部署，需PCB上在固定电感值，建议并联基础电容限制震荡频率。待测量电容器有差分（并联接入通道）接入和单端（一端送入通道，一段接地）接入两种模式。差分模式精度高，单端测量范围大。

Ti论坛推荐在接口下方铺地，构成单端电容，抗EMI。

在传感器电容响应测量的应用场景，一般选择差分即可。

### 1.1.2 测量参数

于框图可见，可配置的输入信号FIN_SEL分频器和参考时钟FREF_SEL分频器。

此外，测量参数还包括滤波带宽、RCOUNT频率计数时长（关系测量速率与精度）、SETTLE_DOWN沉降等待时间（多通道测量情境下）、IDRIVE驱动电流配置（核心环节但在FDC文档里未很好提及）。

基本测量方案为，经分频滤波调理的LC振荡信号送入数字频率计内核，作计数器激励时钟；另有参考时钟可配置的计数器CNTR，二者同时计数直至CNTR计数值达到RCOUNT，输出二者比值。

SETTLEDOWN为切换通道时，对IDRIVE驱动电流为LC电路提供能量，达到稳态这一过程的等待时间。

IDRIVE关系到FDC对LC电路补充能量的能力。其选型详见 [LDC电流激励配置文档.pdf](LDC电流激励配置文档.pdf) 或札记下文。

各参数的配置寄存器，或各寄存器的功能，详见fdc文档 [fdc2214_en.pdf](fdc2214_en.pdf) 

文档**10.2 Typical Application**中有推荐寄存器配置。

文档**9.4 Device Functional Modes**中强调了模块上电后处于休眠模式，需写CONFIG寄存器解除

## 1.2 初始化寄存器配置代码

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

//虽然fdc的测量原理支持软件直接轮询数据寄存器，但还是建议读取此寄存器判断数据是否准备好，
//还有一个通道数据READY寄存器
 do {
        this->regRead(STATUS,2,rx);
        UNREADCONV=rx[1] & ( CH?(0b00000100):(0b00000010) );
        //降低轮询频率，防止IIC错误
        delay_us(100);
    }while(!UNREADCONV);
~~~

## 1.3 IIC时序要求

FDC2214通过7位地址16位数据的IIC总线，支持400kHz的快速模式，是屑软件IIC难以启及的高度捏。软件IIC框架设计中，微妙级延迟根据此表整定。包括建立时间和保持时间的选型。（以原子哥为首的软件IIC未免太粗犷哩

![image-20230411125440601](https://s2.loli.net/2023/04/11/mIlpxH8NT7bOXnZ.png)

## 1.4生成数据读取

测得数据为与参考频率之比，是28位数字量格式。有两个16位寄存器MSB和LSB，低28位为数据位；高四位为告警位，可通过配置Config寄存器开启。

# 2.FDC测量报告：

## 2.1 误差分析

此FDC模块测量示值距离标称值较远，始终偏大，误差分析如下：

**1.内部时钟温漂**

>    pic：FDC温漂曲线,参考频率FREF将变化，测出频率为其比例。
>
> ![image-20230419165913693](https://s2.loli.net/2023/04/19/rgBzHp3ARGMVf4O.png)
>
> ![image-20230419170111483](https://s2.loli.net/2023/04/19/G2VExPjpCYOLu86.png)

**2.内部晶振温漂曲线器件制造公差**

 官方评估板的元器件选型，电感容差10%，致命。

**3.测量的数量级与环境噪声** 

官方评估板使用单端连接方法在端口A连接金属极板，用以触摸监测等场景。环境湿度，测量场地等因素影响元器件的介电常数（动态）；pcb、接线座子，面包板，杜邦线的分布电容（1pf量级，静态）

> 》2.1不同端子接线下，零值有1pf的差值
>
> <img src="https://s2.loli.net/2023/04/14/xpX6HgrV2RqhL3A.png" alt="image-20230414200434619" style="zoom: 15%;" />

> 》一项19年电赛开源报告，在纸张测量实验中，测量外部金属极板电容时，极板会有较大电磁干扰
>
> ![image-20230414202611487](https://s2.loli.net/2023/04/14/vhtK48HnIWlScpY.png)
>
> 》极板长时间未短路，出现充电效应
>
> ![image-20230414201632432](https://s2.loli.net/2023/04/14/yEZAOqxCH7s983h.png)

**4.元器件频率特性**

理想的纯电阻，电感、电容模型是不存在的；ESR、ESL、ESL模型；传感其自身的电学特性曲线构成了一幅电容式传感器相同激励，不同谐振频率下的电容响应不同的图景。

 <img src="https://s2.loli.net/2023/04/14/TXOV7HrnN5SywcJ.png" alt="image-20230414222403469" style="zoom: 50%;" />



==而评估板soc上以**单端模式**(single -ended)串联了一个等效电容未知的极板==

![image-20230414202243980](https://s2.loli.net/2023/04/14/CkdSc2w8P6ngUGh.png)





## 2.2 测量速率分析

讨论单通道情境下，仅需配置RCOUNT以编程测量速率（多通道还包括通道使沉降时间SETTLEDOWN）。Ti标称4.08 ksps=245us /sample

在40Mhz参考时钟下，FDC转换速率

$0X100*16/fREF=102 us/sample$

在400k的IIC配速下，

对测量结果的读取时序为：

（写设备地址|WT→写寄存器地址→写设备地址|RD→读取十六位寄存器）*2。

共10个字节时序，8个应答环节，总计98个时钟。

用时245us，暨4.08ksps.

IIC通信时间长于转换所需时间，可直接连续执行measure方法采样，以达到4.08ksps。

## 2.3 可视化测量记录

### 2.3.1 纸张计数实验

> 纸张数-》电容记录表，早期实验，采样方式不科学（串口读数，一眼顶针法）

<img src="https://s2.loli.net/2023/04/14/LGXzFUP3QkxgNhu.png" style="zoom: 33%;" />

> 线性插值曲线与模拟测试。C与d的反比例关系后期精度不了，数据有一个坏点。

<img src="https://s2.loli.net/2023/04/14/XTwJdk52Db4Y3Bf.png" alt="image-20230414203910656" style="zoom: 33%;" />

> MATLAB源码，选取部分样点生成分段线性插值模型，模拟测试输入为记录表中数据添加正负0.08pf的噪声。（下面的程序还没加上噪声，加了结果不变，还是有个坏点。）

<img src="https://s2.loli.net/2023/04/14/hiXRIYjvHpZ23FG.png" alt="image-20230414204039580" style="zoom: 67%;" />



![image-20230414204422104](https://s2.loli.net/2023/04/14/FDnobUmGhSQYKw5.png)

### 2.3.2陶瓷电容器，绝对测量实验。

> 对一些小电容的测量，首先需特别注意电容的频率特性。
>
> 然后，需知，频率式数字电容计方案，绝对漂移是比较明显的，它适合应用于电容传感器响应值快速变化的场景，测量动态响应，比如，呼吸监测。

<img src="https://s2.loli.net/2023/04/14/dHLbVhI6tj38J2U.png" alt="image-20230414220528553" style="zoom:67%;" />

> 发现模块集成电感容差为10%，为输出数据除补偿因子{0.9 ,1.1 }，示值的上界下界如图：

<img src="https://s2.loli.net/2023/04/14/SlnRj5mCdb9HyT1.png" alt="image-20230414221926728" style="zoom: 33%;" />

## 2.4 TI参考译文贴

### 2.4.1 @驱动电流可能影响电容示值

![image-20230415203531365](https://s2.loli.net/2023/04/15/6lHwmtzFyxZqhvK.png)

见3.4.3.电流源驱动影响测量电压，低电压精度不足，高电压会错误打开意

### 2.4.2@fdc不适合绝对测量而适合动态响应

![image-20230415205005706](C:\Users\yceachan\AppData\Roaming\Typora\typora-user-images\image-20230415205005706.png)

### 2.4.3@电容的阻抗-频率特性可能影响测量结果

![image-20230415211204142](https://s2.loli.net/2023/04/15/MLt4oy1cp2G5Hah.png)

# 3FDC应用札记

## 3.1 驱动电流IDRIVE详解

详见文档 [LDC电流激励配置文档.pdf](LDC电流激励配置文档.pdf) 

![image-20230725235039578](https://s2.loli.net/2023/07/25/ewLFsSpATy9uMrd.png)

电感器、电容器有着寄生电阻（ESR），LC电路可等效为RLC并联模型，并表现出谐振频率的特征阻值R~p~,端口电压与其函数关系计算如上图所示。

电阻是LC振荡电路上耗能环节，电路需要能量输入维持振幅，故当驱动电流较低时，振荡会不稳定，笔者尝为FDC配置较低驱动电流，观察到此现象，调高电流后解决。

LC振幅除与激励电流IDRIVE有关外，还有ESR的协变量，故谐振式频率计无法闭环控制振幅，在Ti论坛，官方建议通过实验方法整定IDRIVE配置。

![image-20230726000413813](https://s2.loli.net/2023/07/26/vrlD6QqjOY9uILg.png)

如果振荡赋值较高，基于时钟温漂等效应，积温过热会使测量江都下降。

如果振荡幅值低于1v2，LC信号的信噪比低

如果低于0v5，谐振不稳定、数字频率计效果差