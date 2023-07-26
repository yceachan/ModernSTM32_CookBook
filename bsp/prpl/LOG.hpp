//
// Created by 衣陈 on 2023/3/15.
//
/*本LOG日志框架，初步采用宏绑定HAL库指针的方法完成render层工作。实现了一个具名变量LOG宏和泛型print。*/
#ifndef FDC_TRAIL_LOG_HPP
#define FDC_TRAIL_LOG_HPP

#define endl '\n'

#include <string>
#include <usart.h>
#include <memory>

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

#define logln(...) do{print("in ",__FUNCTION__,",l",__LINE__,endl);EXPAND(NTH_LOG(__VA_ARGS__))(__VA_ARGS__);}while(0)

#define LOGTEXT(...) do{print(__VA_ARGS__);}while(0)
#define LOGVAR(...)  do{EXPAND(NTH_LOG(__VA_ARGS__))(__VA_ARGS__);}while(0)
#define LOGFUNC(...) do{ print(__func__,endl);LOGVAR(EXPAND(__VA_ARGS__)); }while(0)


#define logplot(...) do { print("c:",__VA_ARGS__,'\n');}while(0)
//只处理数字数字与字符串



/* @name:print
 * @brirf:print实现层，接受一个PRINTABLE的形参，调用HAL库接口向指定LOGGER串口发送asiic文本。
 * */
template<typename T>
void print(T&& arg){
    if constexpr (std::is_assignable_v<std::string_view ,T>){   //捕捉到char (&)[]类型的trick。
        std::string_view str=arg;
        HAL_UART_Transmit(LOGGER,str.data(),str.length(),300);
    }
    else {
        std::string_view str=std::to_string(arg) +' ';
        HAL_UART_Transmit(LOGGER,str.data(),str.length(),300);
    }
}
/* @name:print
 * @brirf:泛型print接口，可接受任意个PRINTABLE的形参，使用lamda初始化列表解包
 * */
template<typename... Ts>
void print(Ts&&... args) {
    (void) std::initializer_list<int> { ( [&]{print(args);}(),0 )... };
}

/* @name:ASSERT
 * @brirf:ASSERT宏语句，用以裸机环境的断言调试，当断言失效时，调用print接口打印错误日志。
 * */
#define ASSERT(expr){ \
    if((expr)==false){\
    print("\n\n",__FUNCTION__,"-assert failed:\n","[FALSE]:",#expr,endl);} \
    }
/*
 * @name  logln
 * @brief 一个接受任意个PRINTABLE形参，然后逐个逐行打印其名与值的宏魔法
 * @param  print_able * any
 * @notice：打印名将是传入文本的原样,可以为表达式或字面量;底层LOGGER为print方法，请实现,
 * */
#define logln(...) do{print("in ",__FUNCTION__,",l",__LINE__,endl);EXPAND(NTH_LOG(__VA_ARGS__))(__VA_ARGS__);}while(0)




logln(1;




template<typename T>
void print(T *&addr) {
    print((uint32_t)addr);
}

//字符与bool的特化方法，区分左右值引用
template<> void print<char&> (char& ch);
template<> void print<char&&>(char&& ch);
template<> void print<bool&> (bool& ch);
template<> void print<bool&&>(bool&& ch);
template<typename T> void print( T*& addr);


#endif //FDC_TRAIL_LOG_HPP
