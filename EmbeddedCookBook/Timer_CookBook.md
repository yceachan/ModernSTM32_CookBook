# 0.OverView

> [您好，这是Bing。STM32的定时器有多种寄存器，它们的功能和配置取决于定时器的模式和特性。](https://www.digikey.com/en/maker/projects/getting-started-with-stm32-timers-and-timer-interrupts/d08e6493cefa486fb1e79c43c0b08cc6)[1](https://www.digikey.com/en/maker/projects/getting-started-with-stm32-timers-and-timer-interrupts/d08e6493cefa486fb1e79c43c0b08cc6)[2](https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf)[3](https://www.st.com/resource/en/application_note/dm00042534-stm32-crossseries-timer-overview-stmicroelectronics.pdf)
>
> 一般来说，STM32的定时器寄存器可以分为以下几类：
>
> - 控制寄存器（CR1、CR2等）：用于设置定时器的工作模式、方向、预分频、中断使能等参数。
> - 状态寄存器（SR）：用于显示定时器的运行状态，如溢出、捕获、比较等事件的发生。
> - 计数寄存器（CNT）：用于显示或设置定时器的当前计数值。
> - 重装载寄存器（ARR）：用于设置定时器的最大计数值，当计数值达到该值时，会产生溢出事件并重置计数值。
> - 捕获/比较寄存器（CCR1、CCR2等）：用于设置或读取捕获/比较通道的参考值，当计数值与该值相等时，会产生捕获/比较事件并触发相应的输出动作。
> - 输出控制寄存器（CCMR1、CCMR2等）：用于设置捕获/比较通道的工作模式、极性、滤波等参数。
> - 输出使能寄存器（CCER）：用于使能或禁止捕获/比较通道的输出信号。
> - DMA控制寄存器（DIER）：用于设置DMA请求源和优先级等参数。
> - 选项寄存器（OR）：用于选择外部时钟源或其他特殊功能。
>
> [具体地，不同类型和系列的STM32定时器可能有不同数量和名称的寄存器，请参考相关文档](https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf)[2](https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf)[3](https://www.st.com/resource/en/application_note/dm00042534-stm32-crossseries-timer-overview-stmicroelectronics.pdf)了解更多细节。
>
> [好的，STM32F407有多个定时器，其中包括两个高级定时器（TIM1和TIM8），四个通用定时器（TIM2、TIM3、TIM4和TIM5），两个基本定时器（TIM6和TIM7）和十个通用定时器（TIM9、TIM10、TIM11、TIM12、TIM13和 TIM14）。](https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf)[1](https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf)[2](https://www.st.com/en/microcontrollers-microprocessors/stm32f407-417.html)
>
> [每个定时器都有一组寄存器，用于控制其功能和状态。这些寄存器的名称和功能与我之前提到的类似，但是具体的位数和位域可能有所不同。您可以参考文档](https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf)[1](https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf)[2](https://www.st.com/en/microcontrollers-microprocessors/stm32f407-417.html)查看每个寄存器的详细信息。

# 1.微妙级延时案例：读写CNT寄存器

**如果您想要使用定时器完成微秒级的延时，您可以使用以下步骤：**

- 选择一个合适的定时器，例如TIM2。
- 设置该定时器的预分频系数（PSC）和重装载值（ARR），使得计数周期等于1微秒。例如，如果系统时钟为168MHz，则PSC=167，ARR=0xFFFF。（请查阅datashhet确定计时器所挂载总线，bing确定总线频率）
- ==使能该定时器，并清零计数值（CNT）。==
- 在需要延时的地方，==读取当前计数值==，并与目标延时值比较，直到计数值达到或超过目标延时值为止。
- 关闭该定时器。



**如果您想要使用HAL库操作TIM7，您可以参考以下步骤：**

- 编写一个初始化函数，例如MX_TIM7_Init()，用于设置TIM7的参数。在该函数中，您需要：
  - 设置htim7.Instance为TIM7。
  - 设置htim7.Init.Prescaler为**83**，以实现1MHz的计数频率。
  - 设置htim7.Init.CounterMode为TIM_COUNTERMODE_UP，以实现**向上计数模式**。
  - 设置htim7.Init.Period为0xFFFF，以实现最大的计数范围。
  - 设置htim7.Init.ClockDivision为TIM_CLOCKDIVISION_DIV1，以不分频时钟源。
  - **调用==HAL_TIM_Base_Init(&htim7)==函数，以初始化TIM7基本功能。**



- 在main.c文件中，在需要使用TIM7的地方：
  * 调用==HAL_TIM_Base_Start(&htim7)==函数，以启动TIM7计数器。
  * 调用\_\_HAL_TIM\_==GET_COUNTER==(&htim7)宏或\_\_HAL_TIM_==SET_COUNTER==(&htim7, value)宏，以读取或设置当前计数值。value是一个uint16_t类型的变量或常量。
  * 调用==HAL_TIM_Base_Stop(&htim7)==函数，以停止TIM7计数器。

~~~cpp
//lesser then 65536
void us_delay(uint16_t us) {
    static  TIM_HandleTypeDef *delayer=DELAYER;
    __HAL_TIM_SET_COUNTER(delayer,0);
    HAL_TIM_Base_Start(delayer);
    uint16_t block=0;
    while(block= __HAL_TIM_GET_COUNTER(delayer) < us);
    HAL_TIM_Base_Stop(delayer);
}
~~~

