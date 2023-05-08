# 1.cortexM内存管理

> 进来在做Rxdma收发时，遇到了种种灵异现象，其实都是堆栈溢出的锅，踩过这个坑后，本文旨在讲清这个内核大坑。
>
> Ref:[(30条消息) 深入理解STM32内存管理_行稳方能走远的博客-CSDN博客](https://blog.csdn.net/zhuguanlin121/article/details/119799860)
>
> [(30条消息) (GCC)STM32基础详解之内存分配_stm32内存分配_我我我只会printf的博客-CSDN博客](https://blog.csdn.net/qwe5959798/article/details/122562894)

## 32位存储器架构

cortexM作为32位处理器，其寻址空间能够达到4G，（如，将地址装填如寻址寄存器，内核按照此地址去访问所有的存储信息：程序存储器，数据存储器，外设寄存器，etc）

即，所有具备存储意义的信息，其存储地址都将被映射到这个4G的线性空间中，布局如下：

CODE层：0x00-0x2000 0000  FLASH，SRAM，或BOOTLOADER，根据BOOT引导映射

SRAM层：0x2000+                   片内内存

外设层   :   0x4000+                   (根据CMISIS标准下的svd文件，将外设寄存器地址映射到这里)

etc

![在这里插入图片描述](https://img-blog.csdnimg.cn/e19625eec4d04479ab900b2a532c86b2.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3podWd1YW5saW4xMjE=,size_16,color_FFFFFF,t_70)

## STM32_SRAM架构

cortexM内核为SRAM预留了512MB的寻址空间，而实际寻址能力必然首先与物理SRAM_SIZE

其布局如下:



![image-20230508152717203](https://s2.loli.net/2023/05/08/BOZg9H6bofztM2Y.png)

![image-20230508153030652](https://s2.loli.net/2023/05/08/lInoSBKmfcRhzFw.png)

堆溢出： 1. 溢出到栈，若未被使用，无明显后果。  2.溢出到SRAM外，发生硬件错误，mcu死机

栈溢出：1.溢出到堆，若已被使用，数据被纂改。    2.溢出到静态区：修改全局变量。3.溢出到TEXT段：修改程序代码，必然死机。

**(以上的坑笔者全部踩过)**

## C/CPP程序内存布局

**栈区、堆区、全局/静态存储区、常量存储区、代码区**。

* **其中可分为三段：**

Text 段：存放CPU执行的机器指令

BSS 段：(**BlockStarted by Symbol**)  存放未经初始化的全局变量“符号”（而在cpp中，这都会被初始化为0）

DATA段：

​    静态区：存放初始化后的全局变量

​    堆/栈区：乏善可陈

* **程序编译期占用内存：**

Code层 ：代表执行的代码，程序中所有的函数都位于此处。即上述的text段。

RO-data(Read Only)层：代表只读数据，程序中所定义的全局常量数据和字符串都位于此处，如const型。

RW-data(Read Write)层：代表已初始化的读写数据，程序中定义并且初始化的全局变量和静态变量位于此处。

ZI-data(Zero Initialize)层：代表未初始化的读写数据，程序中定义了但没有初始化的全局变量和静态变量位于此处。Keil编译器默认是把你没有初始化的变量都赋值为例0。即上述的bss段。



最终的FLASH，RAM存储信息如下：

**Flash = Code + RO Data + RW Data** （常量与代码信息存储到FLASH中）

**RAM = RW-data + ZI-data **                  （FLASH模式下，RAM存储读写信息，只读信息留FLASH

而调试模式中，代码信息也会被搬运到SRAM中以单步调试



## ARMgcc交叉编译下的内存管理

## 汇编启动文件：startup_xx.s

前述：冯诺依曼架构，指令和数据共用存储空间

负责初始化SP堆栈（用户栈），初始化PC指针(指令栈)，分配NVIC中断向量地址

![image-20230508145113756](https://s2.loli.net/2023/05/08/gUPXViumLFxvwzf.png)

**实现方式：定义弱符号站位，待链接器分配实体**

### 1. SP初始化：data段与bss段地址分配：

声明了未初始化的弱符号 .word若干，在链接过程中，这些符号将被link向对应的

![image-20230508150124661](https://s2.loli.net/2023/05/08/zbW7X9wQNuDh2Gq.png)

### 2.PC初始化：复位指令栈-->main启动引导

> `_estack = ORIGIN(RAM) + LENGTH(RAM)`

在发生复位中断后，PC指针将跳转到此处，执行以下C/CPP运行环境的初始化：

* 设置SP指针                 `ldr   sp, =_estack `（栈顶指针固定为0x2000 +RAM_size）

* 将DATA段数据从FLASH搬运到SRAM中(包括堆栈初始化)
* bss段填零
* 调用sys端初始函数        `bl  SystemInit`
* 调用c的静态区构造函数 `bl __libc_init_array`
* 调用main函数                `bl main`

![image-20230508150249481](https://s2.loli.net/2023/05/08/wGODb64nSq5jug8.png)

### 3.NVIC中断向量分配：

中断向量发生时，PC栈为压栈保持现场数据，CPU切换至特权状态，然后调用存储在此处的中断服务函数（弱符号，待链接）。此中断服务函数弹栈退出后，PC栈弹栈恢复现场。

注意，此处弱符号的override将在c文件中完成，编译器对中断函数生成的符号链接必须与汇编中的弱符号完全一致

**———中断服务函数IRQ应在`extern "C" {void IRQn(void){}}` 中完成，因为cpp具有函数重载等特性，cpp编译器生成的符号链接与表象不一致。**

![image-20230508151430823](https://s2.loli.net/2023/05/08/WmRoFQCwGNnBJqM.png)

## Cube脚本链接文件 xx_FLASH/RAM/ld

此链接脚本声明了cpu的存储资源与用户设置的堆栈大小

![image-20230508152137519](https://s2.loli.net/2023/05/08/IyQcwgeqaStbHl3.png)

![image-20230508152053377](https://s2.loli.net/2023/05/08/cm9ew2Jt7bOW4Gj.png)

## 堆生长实现层system.c_sbrk(size_t incr)

> @brief: _sbrk() allocates memory to the newlib heap and is used by malloc and others from the C library

此函数负责堆的生长，维护一个堆顶指针`__sbrk_heap_end`(在汇编层分配堆栈时，调用此函数扩展堆的大小) 和堆底指针`&_end`,这一段内存区间将分配给newlib 的api使用，

 （我在链接脚本中设定了 ：`_Min_Heap_Size = 0x1000`）

![image-20230508160852398](https://s2.loli.net/2023/05/08/oVK7RlZ3vurgCA2.png)

malloc与free的活动将限在`&_end` 到`__sbrk_heap_end`这个区间。

![image-20230508154537549](https://s2.loli.net/2023/05/08/xqCJ89bjrTgQkHU.png)

> 代码

```cpp
/**
 * @brief _sbrk() allocates memory to the newlib heap and is used by malloc
 *        and others from the C library
 *
 * @verbatim
 * ############################################################################
 * #  .data  #  .bss  #       newlib heap       #          MSP stack          #
 * #         #        #                         # Reserved by _Min_Stack_Size #
 * ############################################################################
 * ^-- RAM start      ^-- _end                             _estack, RAM end --^
 * @endverbatim
 *
 * This implementation starts allocating at the '_end' linker symbol
 * The '_Min_Stack_Size' linker symbol reserves a memory for the MSP stack
 * The implementation considers '_estack' linker symbol to be RAM end
 * NOTE: If the MSP stack, at any point during execution, grows larger than the
 * reserved size, please increase the '_Min_Stack_Size'.
 *
 * @param incr Memory size
 * @return Pointer to allocated memory
 */
void *_sbrk(ptrdiff_t incr)
{
  extern uint8_t _end; /* Symbol defined in the linker script */
  extern uint8_t _estack; /* Symbol defined in the linker script */
  extern uint32_t _Min_Stack_Size; /* Symbol defined in the linker script */
  const uint32_t stack_limit = (uint32_t)&_estack - (uint32_t)&_Min_Stack_Size;
  const uint8_t *max_heap = (uint8_t *)stack_limit;
  uint8_t *prev_heap_end;

  /* Initialize heap end at first call */
  if (NULL == __sbrk_heap_end)
  {
    __sbrk_heap_end = &_end;
  }

  /* Protect heap from growing into the reserved MSP stack */
  if (__sbrk_heap_end + incr > max_heap)
  {
    errno = ENOMEM;
    return (void *)-1;
  }

  prev_heap_end = __sbrk_heap_end;
  __sbrk_heap_end += incr;

  return (void *)prev_heap_end;
}
```

# 内存分配案例

&huart5 :Cube初始化代码中创造的全局外设句柄，位于bss段

this:拷贝构造中的临时右值对象，位于栈上

this->puf: 构造函数中申请的堆上空间，位于堆上

![image-20230508162626868](https://s2.loli.net/2023/05/08/FMrBp9hDqdV8kX1.png)