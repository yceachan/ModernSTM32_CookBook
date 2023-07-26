//
// Created by 衣陈 on 2023/3/25.
//

#include "IIC.h"
#include"LOG.hpp"

void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(DELAYER , 0);
    HAL_TIM_Base_Start(DELAYER);
    uint16_t block = 0;
    while( ( block =  __HAL_TIM_GET_COUNTER(DELAYER) )  < us);
    HAL_TIM_Base_Stop(DELAYER);
}
inline void setupDelay() { delay_us(1);}
inline void holdonDelay() { delay_us(2);}


void IIC7bitDev::regWrite(uint16_t regADR, uint16_t data) {
    this->start();
    this->sendByte(devADR_WT());

    if(!this->waitAck()) {print("W devADR_WT,Dev noAck\n"); return;}   //设备应答，寄存器无应答？


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
    return PIN_STATE(HAL_GPIO_ReadPin(SDA_PORT,SDA_PIN));
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
    holdonDelay();//SCL low period:1.3us
}

void BaseIICdev::stop() {
    this->set_SDA(LOW);//tf=300ns no setup for data
    this->set_SCL(HIGH);
    setupDelay();//Setup time for a stop condition:0.6s
    this->set_SDA(HIGH);
    holdonDelay();//SDA high pulse duration between STOP and START :1.3us
//    this->set_SCL(LOW);   通讯已结束，空闲上拉即可，无需下拉结束时钟
//    holdonDelay();//SCL low period:1.3us
}

void BaseIICdev::ack(){
    this->set_SDA(LOW); //数据传输后给
    setupDelay();
    this->set_SCL(HIGH);
    holdonDelay();
    this->set_SCL(LOW);
    holdonDelay();
    this->set_SDA(HIGH);//主机应答后释放SDA
}
void BaseIICdev::nack() {
    this->set_SDA(HIGH);
    setupDelay();
    this->set_SCL(HIGH);
    holdonDelay();
    this->set_SCL(LOW); //在本时钟周期应答了SDA：HIGH。为仅邻的IIC:stop创造了条件
    holdonDelay();
}

void BaseIICdev::sendByte(uint8_t data){
    /*0x80≡1000,0000，即发送MSB（最高有效位）*/
    for(uint8_t i=0;i<8;i++)
    {
        set_SDA( ((data & 0x80) >> 7) ? HIGH : LOW );    setupDelay();   //移位优先级高于按位与
        set_SCL(HIGH);  holdonDelay();
        data<<=1;
        set_SCL(LOW);holdonDelay();
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

bool BaseIICdev::waitAck() {  //如果您在类定义内部声明成员函数为内联，则无需在类外部的成员函数定义中再次添加inline关键字。
    PIN_STATE  re = HIGH;

    set_SDA(HIGH);	/* CPU释放SDA总线 */
    setupDelay();
    set_SCL(HIGH);	/* 第 9 帧时钟 ： 采集设备应答信号 零为应答，1为非应答 */

    __HAL_TIM_SET_COUNTER(DELAYER,0);
    HAL_TIM_Base_Start(DELAYER);
    while( ( (re=read_SDA()) == HIGH ) && __HAL_TIM_GET_COUNTER(DELAYER) < 30000 );

    HAL_TIM_Base_Stop(DELAYER);

    set_SCL(LOW); //采样完成，结束时钟;
    return (re == LOW);
}
BaseIICdev::BaseIICdev(uint16_t devADR, GPIO_TypeDef *sclPORT, uint16_t sclPIN, GPIO_TypeDef *sdaPORT, uint16_t sdaPIN) {
    this->devADR=devADR;
    this->SCL_PIN=sclPIN;
    this->SCL_PORT=sclPORT;
    this->SDA_PIN=sdaPIN;
    this->SDA_PORT=sdaPORT;
}

void BaseIICdev::set_SCL(BaseIICdev::PIN_STATE op) {
    HAL_GPIO_WritePin(SCL_PORT,SCL_PIN,GPIO_PinState(op));
}

void BaseIICdev::set_SDA(BaseIICdev::PIN_STATE op) {
    HAL_GPIO_WritePin(SDA_PORT,SDA_PIN,GPIO_PinState(op));
}
