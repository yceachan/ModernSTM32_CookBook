//
// Created by 衣陈 on 2023/9/26.
/**
 * 对CortexM4内核，DWT外设——data watch tracer 的单例模式封装，可以实现微妙级的计时、延时等功能，可用以发生或检查时序、评估代码性能等。
 * 以饿汉模式实现，在类装载时生成单例。暂未实现加锁等多线程功能。
 * @ref:https://www.runoob.com/design-pattern/singleton-pattern.html
 */
#ifndef UART485_DWT_H
#define UART485_DWT_H
#include "main.h"
#define dwt_ (DWT_Ctrl_SingleTon::getInstance())
class DWT_Ctrl_SingleTon {
public :
    static DWT_Ctrl_SingleTon getInstance() {
        return instance;
    }
    static void set_cnt(uint32_t val);
    static uint32_t get_cnt();
    volatile static void delay_us(uint32_t us);
    static uint32_t get_sysFreq();
private :
    static DWT_Ctrl_SingleTon instance;
    DWT_Ctrl_SingleTon ();


};
#endif //UART485_DWT_H
