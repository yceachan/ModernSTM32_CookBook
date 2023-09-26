/********************************************/
//*软件IIC驱动框架，适配打拍、DWT、TIM等多种微妙级延时方法以产生通信时序
//*对代码性能、IIC时序细节进行了多处实验和优化，标称配速250khz,实际达到172khz.
//*ref :非常值得参阅的[C++性能优化指南.pdf]
/********************************************/

#include "IIC.h"


/********************************************/
/*毫秒级延时驱动方面，由如下方案：
*1.__NOP。除了不优雅没什么大的缺点，需要借助仪表用实验方法获得经验配置
*2、SYSTICK和其他定时器外设。驱动方式建议直接使用宏，delay_us函数是代码热点，调栈等开销会被无限放大。
*3、DWT_CNT方案。Cortex-M4内核通用方案，f1不适用，DWT内有一个随MCU主时钟自重装载向上计数的计时器。驱动方式见M4权威指南。
/********************************************/
#define __LOG_ENABLE__
#ifdef __LOG_ENABLE__
#include"LOG.hpp"
#else
 void print(char*){return;}
#endif

#define __DWT_FOR_DELAY__
// #define __NOP_FOR_DELAY__
// #define __TIM_FOR_DELAY__
#ifdef __DWT_FOR_DELAY__
#include "DWT.h"
static void delay_us(uint32_t us){
    dwt_.delay_us(us);
}

#ifdef __NOP_FOR_DELAY__
volatile static void delay_us(uint16_t us) {
    uint8_t i;
    /*　
         下面的时间是通过安富莱AX-Pro逻辑分析仪测试得到的。
        CPU主频72MHz时，在内部Flash运行, MDK工程不优化
        循环次数为10时，SCL频率 = 205KHz
        循环次数为7时，SCL频率 = 347KHz， SCL高电平时间1.5us，SCL低电平时间2.87us
         循环次数为5时，SCL频率 = 421KHz， SCL高电平时间1.25us，SCL低电平时间2.375us
    */
    for (i = 0; i < 7; i++) __NOP();
}
#else 
#ifdef __TIM_FOR_DELAY__
//*传入1Mhz的定时器，由于总线访问等开销，在MCU级的主频下，延时时间实际更长(+0.5-1us)
#define DELAYER &htimX
static void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(DELAYER , 0);
    HAL_TIM_Base_Start(DELAYER);
    uint16_t block = 0;
    while( ( block =  __HAL_TIM_GET_COUNTER(DELAYER) )  < us);
    HAL_TIM_Base_Stop(DELAYER);
}
#endif
#endif
#endif

inline void setupDelay() { delay_us(1);}  //用于保证电平变化的顺序    ，建立时间
inline void holdonDelay() { delay_us(2);} //用于SCL HIGH下的 SDA采集，保持时间


void IIC7bitDev::regWrite(uint16_t regADR, uint16_t data) {
    this->start();
    this->sendByte(devADR_WT());

    if(!this->waitAck()) {print("W devADR_WT,Dev noAck\n"); return;}   //


    this->sendByte(regADR);
    if(!this->waitAck()) { print("W regADR,Dev noAck\n");   return;}

    uint8_t  msb=data>>8,lsb=(data<<8)>>8;
    this->sendByte(msb);
    if(!this->waitAck()) { print("W msb,Dev noAck\n");     return;}

    this->sendByte(lsb);
    if(!this->waitAck()) { print("W lsb,Dev noAck\n");     return;}

    this->stop();
}

void IIC7bitDev::regRead(uint16_t regADR, uint8_t len, uint8_t *Rx_buffer) {
    this->start();
    this->sendByte(devADR_WT());
    if(!this->waitAck()) {print("R devADR_WT,Dev noAck\n");  return;}
    this->sendByte(regADR);
    if(!this->waitAck()) { print("R regADR,Dev noAck\n");  return;}

    this->start();
    this->sendByte(devADR_RD());
    if(!this->waitAck()) { print("R devADR_RD,Dev noAck\n");  return;}


    while(len){
        *Rx_buffer++ = this->readByte();
        if(--len) this->ack();
        else      this->nack();
    }
    this->stop();
}


IIC7bitDev::IIC7bitDev(uint16_t devADR, GPIO_TypeDef *sclPORT, uint16_t sclPIN, GPIO_TypeDef *sdaPORT, uint16_t sdaPIN)
        : BaseIICdev(devADR, sclPORT, sclPIN, sdaPORT, sdaPIN) {}


BaseIICdev::PIN_STATE BaseIICdev::read_SDA() {
    if((SDA_PORT->IDR & SDA_PIN))
    {
       return HIGH;
    }
    else
    {
        return LOW;
    }
}

uint16_t BaseIICdev::devADR_RD() {
    return ((this->devADR<<1)|1);
}

uint16_t BaseIICdev::devADR_WT() {
    return ((this->devADR<<1)|0);;
}



void BaseIICdev::start() {
    this->set_SDA(HIGH);;//tf=300ns no setup for data
    this->set_SCL(HIGH);
    setupDelay();//Setup time for start condition:0.6
    this->set_SDA(LOW);
    holdonDelay();//Hold time for start condition  0.6 fast
    this->set_SCL(LOW);
    setupDelay();//SCL low period:1.3us,SCL低的剩余延时在下一个SDA的setup中。
}

void BaseIICdev::stop() {
    this->set_SDA(LOW);//tf=300ns no setup for data
    this->set_SCL(HIGH);
    setupDelay();//Setup time for a stop condition:0.6s
    this->set_SDA(HIGH);
    holdonDelay();//SDA high pulse duration between STOP and START :1.3us ：1.3us的holdon SDA采集
//    this->set_SCL(LOW);   通讯结束，空闲释放SCL即可,无需拉低SCL。
//    holdonDelay();//SCL low period require:1.3us
}

void BaseIICdev::ack(){
    this->set_SDA(LOW); //准备应答信号
    setupDelay();
    this->set_SCL(HIGH);
    holdonDelay();        //应答信号的保持/采样时间
    this->set_SCL(LOW);
    setupDelay();
    this->set_SDA(HIGH);//主机完成SDA低应答后，释放SDA
}
void BaseIICdev::nack() {
    this->set_SDA(HIGH);
    setupDelay();
    this->set_SCL(HIGH);
    holdonDelay();
    this->set_SCL(LOW); //主机nack非应答后，仅接stop时序，故拉低SCL
    setupDelay();
}

void BaseIICdev::sendByte(uint8_t data){
    /*0x80 = 0b1000,0000 即取得u8首位  */
    for(uint8_t i=0;i<8;i++)
    {

        set_SDA( ((data & 0x80)) ? HIGH : LOW );    setupDelay();   //无需移位，0b1000,0000 = true
        set_SCL(HIGH);  holdonDelay();
        data<<=1;
        set_SCL(LOW);setupDelay();  //SCL LOW要求1.3us,配合SDA的setupDelay 正好
    }
    set_SDA(HIGH);//释放SDA
}
uint8_t BaseIICdev::readByte() {
    uint8_t Rx=0;
    for(uint8_t i=0;i<8;i++)
    {
        Rx<<=1;
        set_SCL(HIGH);  holdonDelay();
        Rx|=this->read_SDA();
        set_SCL(LOW);  holdonDelay();
    }
    return Rx;
}

bool BaseIICdev::waitAck() {  //
    PIN_STATE  re = HIGH;

    set_SDA(HIGH);	/* 释放SDA，此时SDA电平为跟从机SDA线与结果，若从机下拉SDA应答，则为低，若尚未应答，则为高 */
    setupDelay();
    set_SCL(HIGH);	/* 产生*/
    setupDelay();//等待时间为：setupDelay + 0xFF nop
    uint8_t tick=0;
    while( ( (re=read_SDA()) == HIGH ) && ++tick< 0xFF );

    set_SCL(LOW); //为stop时序准备的SCL低
    return (re == LOW);
}
BaseIICdev::BaseIICdev(uint16_t devADR, GPIO_TypeDef *sclPORT, uint16_t sclPIN, GPIO_TypeDef *sdaPORT, uint16_t sdaPIN) {
    this->devADR=devADR;
    this->SCL_PIN=sclPIN;
    this->SCL_PORT=sclPORT;
    this->SDA_PIN=sdaPIN;
    this->SDA_PORT=sdaPORT;
}
//*寄存器驱动，加速IO
void BaseIICdev::set_SCL(BaseIICdev::PIN_STATE op) const {
    if(op != LOW)
    {
        SCL_PORT->BSRR = SCL_PIN;
    }
    else
    {
        SCL_PORT->BSRR = (uint32_t)SCL_PIN << 16U;
    }
}

void BaseIICdev::set_SDA(BaseIICdev::PIN_STATE op) const {
    if(op != LOW)
    {
        SDA_PORT->BSRR = SDA_PIN;
    }
    else
    {
        SDA_PORT->BSRR = (uint32_t)SDA_PIN << 16U;
    }
}
