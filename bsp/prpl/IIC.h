/*
* Created by 衣陈 on 2023/3/25.
* 面向对象的软件IIC框架，使用继承特性保证程序 对不同地址长度IIC设备的 可扩展性
* 基类BaseIICdev实现了IIC通信的基本时序和对SCL，SDA的GPIO操作封装。此层对HAL库的封装保障了框架在不同平台的移植性，修改set_PIN底层即可
* 派生类应在基类实现了基本时序的基础上，实现读写寄存器的操作。在本框架中，暂时只实现了最常见的7bit IIC设备。
* 本框架使用方法：IIC通信需要微妙延时函数满足时序。框架应传入配置为1mhz的计时器句柄（在下方修改）
* 
* *关于中断管理，根据IIC时序特性，若延时函数被长时中断拉长，通讯一般不会出错，但速率会下降。
* 若iic设备对实时通信有需求，应修改框架配置中断管理，在时序函数中失能中断。
* 
* 在waitAck函数中，设置了30ms等待时间，若在此函数中发生中断，可能会导致主机错失应答信号或等待超时等非预期错误，建议失能中断。
* 
* 本框架设置了两个延时函数：setupDelay=1us,holdonDelay=2us,可认为IIC通讯速率333khz。
* 这两个延时的配置是笔者对IIC协议中，建立时间，保持时间的学习体会。笔者发现现有网上例程往往都用同一个delay一笔带过，不 优 雅 ！
*/

#ifndef FDC_TRAIL_IIC_H
#define FDC_TRAIL_IIC_H
#include "tim.h"
#include"main.h"

#define DELAYER &htim7
void delay_us(uint16_t us);


class BaseIICdev{
public:
    BaseIICdev(uint16_t devADR, GPIO_TypeDef* sclPORT, uint16_t sclPIN, GPIO_TypeDef* sdaPORT, uint16_t sdaPIN);
    virtual void regWrite(uint16_t regADR, uint16_t data)=0;
    virtual void regRead(uint16_t regADR, uint8_t len, uint8_t* Rx_buffer)=0;
    uint16_t devADR;
    GPIO_TypeDef *SCL_PORT,*SDA_PORT;
    uint16_t SCL_PIN,SDA_PIN;
protected:
    void start();
    void stop();

    enum  ACK_CHOICE{ACK=0,NACK=1};
    enum  PIN_STATE {LOW=0,HIGH=1};

    void sendByte(uint8_t data);
    uint8_t readByte();
    void ack();
    void nack();

    bool waitAck();


    inline uint16_t devADR_RD();
    inline uint16_t devADR_WT();

    inline void set_SCL(PIN_STATE op);
    inline void set_SDA(PIN_STATE op);
    inline PIN_STATE read_SDA();
};


class IIC7bitDev : public BaseIICdev{
public:
    IIC7bitDev(uint16_t devADR, GPIO_TypeDef* sclPORT, uint16_t sclPIN, GPIO_TypeDef* sdaPORT, uint16_t sdaPIN);
    virtual void regWrite(uint16_t regADR, uint16_t data);
    virtual void regRead(uint16_t regADR, uint8_t len, uint8_t* Rx_buffer);
};


#endif //FDC_TRAIL_IIC_H
