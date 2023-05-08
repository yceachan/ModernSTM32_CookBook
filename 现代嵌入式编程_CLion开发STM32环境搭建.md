> Before：mdk的ui和编辑体验属实让人难以接受，而vs Code + keil的解决方案仍是没有解决C的**刀耕火种**式的开发体验。
>
> 单片机开发经历了汇编，寄存器c51，库函数时代，笔者还是觉得这些开发方式有些**刀耕火种**，也比较消磨学习的动力。甚至乎笔者觉得Arduino的类objective-C 也还不够方便。（想用stl捏）
>
> 于是折腾半天，终于配置好了这套CLION+CubeMx+OpenOcd，现分享学习过程。
>
> Reference：
>
> [配置CLion用于STM32开发【优雅の嵌入式开发](https://www.zhihu.com/people/zhi-hui-64-54)]([配置CLion用于STM32开发【优雅の嵌入式开发】 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/145801160))
>
> [Windows上使用 OpenOCD 给 STM32 下载程序 - 腾讯云开发者社区-腾讯云 (tencent.com)](https://cloud.tencent.com/developer/article/1840792))
>
> [在CLion中开发STM32 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/160183640)

工具链：

CLion + armgcc  + Cmake + openOCD

CubeMx + HAL

# 工具链下载

## **arm-none-eabi-gcc**

Windows到这里下载：[https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads](https://link.zhihu.com/?target=https%3A//developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) ，选择ZIP压缩包形式的：

## **openOCD**

[openocd-org/openocd: Official OpenOCD Read-Only Mirror (no pull requests) (github.com)](https://github.com/openocd-org/openocd) 官方GIthub开源库，releases中下载

## **MinGW**32

请注意区别MinGW32 与MinGW-w64，前者已经停止维护，

**MinGW**32下载地址[Download File List - MinGW - Minimalist GNU for Windows - OSDN](https://osdn.net/projects/mingw/releases/)



可以参阅[[科普\]MinGW vs MinGW-W64及其它（比较有意思，来自mingw吧） - findumars - 博客园 (cnblogs.com)](https://www.cnblogs.com/findumars/p/7492636.html)



笔者在下载这个工具集时，**频繁报错msys下载失败**。不过还是可以进行开发，实际用时发现有些问题，比如搜索不到一些C++库的路径，需要笔者手动添加搜索路径"armgcc/include"

所以笔者对这个工具集不是很放心。

关于MSYS的参阅[使用MSYS的一些经验 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/266448826)

## MinGW-w64

根据笔者的检索结果，MinGW-w64可完全兼容MinGW32  （我觉得难说）。故选择MinGW-w64选择在官网下载latest10.0MinGW-w64. 不会从从源码编译

**故退而选择CLION内置MINGW工具链：**

其中，make为`C:\Program Files\JetBrains\CLion 2022.2.1\bin\mingw\bin\mingw32-make.exe`



配置三个工具的环境变量

# 配置

## **CLion->项目tool_chain配置**

工具集：CLION-MinGW，

Make ： `C:\Program Files\JetBrains\CLion 2022.2.1\bin\mingw\bin\mingw32-make.exe`

complier ： arm-none-eabi-gcc/g++

**debugger:  Clion built-in GDB**

![image-20230318123956399](https://s2.loli.net/2023/03/18/zfOALRgoiTI5wyq.png)

## **Clion->项目Cmake配置**

![image-20230311201140591](https://s2.loli.net/2023/03/11/kxKbnv1aloLQhfP.png)

## **CLion->项目Run/Debug配置**

debugger选用内置即可，不用armgdb

注意配置cfg文件。在项目中放入cfg文件后他会自动生成camke配置

![image-20230311201325283](https://s2.loli.net/2023/03/11/6xZeu5fyw3TaK8r.png)

需要添加cmake程序和OpenOCD烧录器

## **openOCD中“.cfg”的配置**

笔者使用dap，需要自己手写**"dap.cfg"**

**注意这个.cfg关系到项目的构筑配置，不写入这个文件cmake是build不出的**

.**详见ref**

> **interface** 文件夹存储了各种调试器的配置文件，包括 cmsis-dap、stlink-v2 等。**target** 文件夹则存储了各种芯片的配置文件，例如 *stm32f1x.cfg* 就对应着 STM32F1 系列。正点原子、野火等国内企业出品的开发板不属于官方开发板，自然在 **board** 文件夹中不会有他们的配置文件，因此我们需要根据自己使用的调试器和芯片，写自己的配置文件。

~~~cpp
# choose st-link/j-link/dap-link etc.
adapter driver cmsis-dap
cmsis_dap_backend hid
transport select swd

# 0x10000 = 64K Flash Size
set FLASH_SIZE 0x160000  //视开发板片内flash资源而定

source [find target/stm32f4x.cfg]

# download speed = 1MHz
adapter speed 1000 //
~~~

# include stdc++

> 如果使用稚晖的MingGW32，需要配置这些，CLION内置的MINGW64不用

将main文件改为cpp后缀，在cmakelist文件中，添加头文件搜索路径：

~~~cpp
#include目录
include_directories(Core/Inc Drivers/STM32F4xx_HAL_Driver/Inc Drivers/STM32F4xx_HAL_Driver/Inc/Legacy Drivers/CMSIS/Device/ST/STM32F4xx/Include Drivers/CMSIS/Include
        D:/CubeMX/gcc-arm-none-eabi-10.3-2021.10/arm-none-eabi/include)
~~~

添加上armgcc的include目录`/gcc-arm-none-eabi-10.3-2021.10/arm-none-eabi/include`

**关于本cmakelist各指令的讲解，可参考学长视频：**

[[SWJTU-G308\] CLion 配置嵌入式开发~_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1EP4y1B7AE/?spm_id_from=333.999.0.0&vd_source=d8c39ae6749f658d0698389713f47409)

# printf重定向

参见稚晖文章，笔者觉得还挺麻烦的，遂罢，~~凑合用HAL库吧~~

笔者选择自己搭建一套基于泛型的LOG框架。

# Program

> 如下可以开始优雅地使用c++开发

~~~cpp
void test()
{
    std::vector<std::string> vec={"Hello embedded\n","Hello clion\n","Hello c++17\n","Hello std::vector,string,and object!"};

    for( auto& it:vec)
    {
        HAL_UART_Transmit(&huart5,it.c_str(),it.size(),300);
    }
}
~~~

蓝牙串口信息：

![image-20230311202541092](https://s2.loli.net/2023/03/11/Cuyvc8QY7kMwaDt.png)

> 后：笔者有次和老师聊天时，吐槽到arduino uno 屏蔽太多底层细节，内核如中断由太过简陋，做课设时踩坑不断。 老师说如果能用51实现也是接受的。 笔者禁不住幽幽地吐槽“感觉51，...有点刀耕火种啊。。”
>
> 如何平衡底层和抽象呢，在单片机学习中笔者慢慢学会了翻datasheet。封装如Arduino，踩坑后可以翻datasheet找到Solution。简陋如51，笔者对着刀耕火种的keilIDE和C89的语言标准，委实是没有力气在寄存器上刀耕火种
>
> 从汇编到c/c++到java c#到python matlab。笔者认为stdc++ 是我喜欢的方式。在熟悉底层到寄存器和汇编的前提下，笔者很乐意用HAL库，用对象，用stl去完成任务。
>
> 现代嵌入式设备的应用场景都落实到深度学习，神经网络，cv的部署上了，在大容量设备上，为什么不可以拥抱现代IDE和编程语言呢？关于运行效率，笔者只在嵌入式中听闻过c++效率不行。作为一个玩票的大学生，我可以口嗨一句embedded C过时了。

# Debug

> Reference :[OpenOCD support | CLion Documentation (jetbrains.com)](https://www.jetbrains.com/help/clion/2022.3/openocd-support.html)

启动debug后，内核处于停止状态，记得点击工具栏上的复位，使PC跳转到Reset_Handler上

（详见本项目内核札记）

Reference:

[STM32立方体MX项目 |CLion 文档 (jetbrains.com)](https://www.jetbrains.com/help/clion/2022.3/embedded-development.html?utm_source=product&utm_medium=link&utm_campaign=CL&utm_content=2022.3#prepare-tools)

# 新建项目步骤

## 1.CLion新建stm32工程

## 2.按照ide提示打开cubemx配置初始化代码

## **3.回到clion，为项目添加.cfg文件**，否则将没有构建配置！！！

## 3.运行配置中添加"Openocd运行并下载"，并选中面板配置文件

## 4.配置cmakelist文件：

### 关闭优化(-g0)，添加include路径：

不关，也行，懂编译原理的话。

![image-20230315174456701](https://s2.loli.net/2023/03/15/hK2p75Grq8ZkTAQ.png)

## 5.Programing，配置cmakelist的file，include_dir

![image-20230315182645185](https://s2.loli.net/2023/03/15/ABRC1WYPayvlQL8.png)

## 6.debug，选择svd文件从而监视外设

关于svd文件的意义，详见本项目CortexM内核札记

@ref ：svd2rust：PAC(外设访问层) [Rust嵌入式开发入门(02)--Rust嵌入式社区生态概览_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1ki4y117vp/?spm_id_from=333.337.search-card.all.click)

对应芯片的svd文件在keil中能找到，用evething搜索即可
