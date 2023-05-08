//
// Created by 衣陈 on 2023/3/15.
//

#ifndef FDC_TRAIL_LOG_HPP
#define FDC_TRAIL_LOG_HPP
#include<iostream>
#include<string>
#include<cstring>
#define endl '\n'

#include <iostream>
#include <string>
#include <usart.h>
#define LOGGER &huart1
/*
* @name   LOG
* @brief  单具名变量LOG方法，打印此变量的名与值，移植到stm32，底层print是HAL库串口发送
* @param  print_able Lvalue
* @notice 将打印传入此方法的文本，可以是表达式或字面量，前者你可用以表达式求值
* @Author 群佬
*/
#define LOG(A) {print(#A,':',A,endl);}
#define LOG1(_1) LOG(_1)
#define LOG2(_1,...) LOG(_1); LOG1(__VA_ARGS__)
#define LOG3(_1,...) LOG(_1); LOG2(__VA_ARGS__)
#define LOG4(_1,...) LOG(_1); LOG3(__VA_ARGS__)
#define LOG5(_1,...) LOG(_1); LOG4(__VA_ARGS__)
#define LOG6(_1,...) LOG(_1); LOG5(__VA_ARGS__)
#define LOG7(_1,...) LOG(_1); LOG6(__VA_ARGS__)
#define LOG8(_1,...) LOG(_1); LOG7(__VA_ARGS__)
#define LOG9(_1,...) LOG(_1); LOG8(__VA_ARGS__)
#define LOG10(_1,...) LOG(_1); LOG9(__VA_ARGS__)

#define _GET_NTH_ARG(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10, N,...) N //返回传参的第11个参数的文本
#define NTH_LOG(...) _GET_NTH_ARG(__VA_ARGS__,LOG10,LOG9,LOG8,LOG7,LOG6,LOG5,LOG4,LOG3,LOG2,LOG1)//传参的第11个参数是将要分发的LOGx方法

#define EXPAND(args) args
/*
 * @name  logln
 * @brief 一个接受任意个PRINTABLE参数，然后逐个逐行打印其名与值的宏魔法
 * @param  print_able * any
 * @notice：打印名将是传入文本的原样,可以为表达式或字面量;底层LOGGER为print方法，请实现,
 * */
#define logln(...) do{print("in ",__FUNCTION__,",l",__LINE__,endl);EXPAND(NTH_LOG(__VA_ARGS__))(__VA_ARGS__);}while(0)
#define logplot(...) do{print("sample:",__VA_ARGS__,'\n');}while(0)

//只处理数字数字与字符串
template<typename T>
void print(T&& arg){
    if constexpr (std::is_assignable_v<std::string_view ,T>){   //能顺利捕捉到char (&)[]类型的方法。
        std::string_view str=arg;
        HAL_UART_Transmit(LOGGER,str.data(),str.length(),300);
        bool oh=std::is_arithmetic_v<char>;
    }
    else {
        std::string str=std::to_string(arg)+' ';
        HAL_UART_Transmit(LOGGER,str.c_str(),str.length(),300);
    }
}
//字符与bool的特化方法，区分左右值引用
template<> void print<const char&>(const char& ch);
template<> void print<const char&&>(const char&& ch);
template<> void print<const bool&>(const bool& ch);
template<> void print<const bool&&>(const bool&& ch);

//初始化列表+lamda表达式+...解包の黑魔法
template<typename... Ts>
void print(Ts&&... args) {
    (void) std::initializer_list<int> { ( [&]{print(args);}(),0 )... };
}




#endif //FDC_TRAIL_LOG_HPP
