邮票孔真难焊啊

HC09的AT指令集挺方便的

# 模块配置

> AT指令书：[HC-09用户手册V1.3.20200512.doc (hc01.com)](https://www.hc01.com/downloads/HC-09.pdf)

~~~cpp
AT+NAME=xxx
AT+ROLE=M/S   //主从模式
AT+CM         //主机模式配置，Hc09的固件比较强大
AT+ADDR       //MAC地址查询，不建议改
AT+UART=115200,N,1 //(115200baud 无校验1停止位)

~~~

![image-20230429201220184](https://s2.loli.net/2023/04/29/y2fuq5UpGEDvMeK.png)

# BLE_GATT广播数据层

![image-20230429201729935](https://s2.loli.net/2023/04/29/79SpbuYvsqjiwUN.png)

BLE_GATT数据层协议：[系统性简述BLE蓝牙_esp32 ble蓝牙_盗版摩羯的博客-CSDN博客](https://blog.csdn.net/u013564470/article/details/123524606?spm=1001.2101.3001.6650.7&utm_medium=distribute.pc_relevant.none-task-blog-2~default~BlogCommendFromBaidu~Rate-7-123524606-blog-126308084.pc_relevant_3mothn_strategy_and_data_recovery&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2~default~BlogCommendFromBaidu~Rate-7-123524606-blog-126308084.pc_relevant_3mothn_strategy_and_data_recovery&utm_relevant_index=12)

![image-20230429194714946](https://s2.loli.net/2023/04/29/PoS2srcbAfZzLjD.png)

**配置文件 (Profile):** Profile 是被蓝牙标准预先定义的一些 Service 的集合，并不真实存在于蓝牙设备中。如果蓝牙设备之间要相互兼容，它们只要支持相同的 Profile 即可。一个蓝牙设备可以支持多个 Profile。

**服务 (Service):** Service 是蓝牙设备对外提供的服务，一个设备可以提供多个服务，比如电量信息服务、系统信息服务等。**每个服务由一个 UUID 唯一标识。**

**特征 (Characteristic):** 每个 Service 包含 0 至多个 Characteristic。**Characteristic 包含一个值 (value)和 0 至多个描述符 (Desciptor) 组成**。在与蓝牙设备通信时，主要就是通过读写 Characteristic 的 value 完成。 **每个 Characteristic 由一个 UUID 唯一标识**。 （是透传数据包的基本单元）

**属性描述符 (Descriptor):** Descriptor 是描述特征值的属性的值————————————————

框架：

HC09-Profile：

->sys_Service :

->uart_Service:  通过AT指令集可配置模块的透传服务UUID 

​                           是BLE通讯时的上层数据包，包含了此数据包的UUID和Characteristic子包

--->TX_Characteristic ：是GATT协议的底层数据包，封装通讯的内容（value）和etc

--->RX_Characteristic： 同上，具有自身的UUID

> @abcd是自己配置的abcd_Service,其余是模组其他service
>
> ![image-20230429212505139](https://s2.loli.net/2023/04/29/eTyQP6bJjHv9KDh.png)

# BLE_LinkLayer链路层状态机

LinkLayer具有如下状态机：

**就绪状态（观察者）（Standby State）**：初始状态，即不发送数据，也不接收数据。根据上层协议的命令（如GAP）可由其它任何一种状态进入，也可以切换到除Connection状态外的任意一种状态。

**广播状态（Advertising State）**：可以通过广播通道发送数据的状态，由Standby状态进入。广播的数据可以由处于Scanning或者Initiating状态的实体接收。上层协议可通过命令将Advertising状态切换回Standby状态。连接成功后可切换为Connection状态。

**扫描状态（Scanning State）**：通过广播通道接收数据的状态，Scanning状态可用于侦听一定区域内的广播数据，有被动扫描和主动扫描两个子状态，被动扫描仅接收广播报文，主动扫描则发送扫描请求给广播态设备，并获取附加的扫描响应数据。Scanning状态的设备只能进入Standby状态，条件是上层协议发送的停止扫描命令。

**初始化状态（Initiating State）**：为了像设备发起连接，**链路层需要处于Initiating状态。发起连接，将携带 connection request（连接请求）响应广播者，侦听自己试图连接的设备**，如果收到了来自该设备的**connectable广播报文**，链路层会向其**发送连接请求**并进入Connection状态，当连接成功后对端的广播设备也会进入Connection状态。Initiating状态由Standby状态进入，如果不再发起连接或连接失败则返回Standby状态，如果连接成功则建立连接的双方都进入Connection状态。
**连接状态（Connection State）**：Connection状态是**两个实体间建立了单独通道的状态**，在通道建立之后，由Initiating或者Advertising自动切换而来。通道断开后，会重新回到Standby状态。（区分于IBECON模式）
![image-20230429202305284](https://s2.loli.net/2023/04/29/mnERQxwlsykLXfv.png)

# BLE通讯层

## 服务器-客户端模式（透传实现模式）

![image-20230429205406184](https://s2.loli.net/2023/04/29/oJegjGmYqikROfH.png)

> 建立连接过程：从机进入广播模式，广播自身的SUUID。主机进入扫描模式，扫描具有LUUID的模块，一旦发现，尝试建立连接。成功后，两个设备一起进入连接模式

**Tx-Rx模块建立主从连接关系下**

GATT层：透传服务中有两个特征，

一个特征是主机对从机的写操作，由主机Tx发送这种数据包

一个特征是从机对主机的通知或指示（模块为通知），由从机Tx发这种数据包

事实上两个特征可以复用为一路，即，此Characteristic具有通知和写属性，但在链路层协议上，Characteristic数据包的值永远是半双工传递的,链路层为协调冲突请求，但不好说（半双工模式）

![image-20230429210511019](https://s2.loli.net/2023/04/29/1SqMoLW6GdUuDBw.png)

> 半双工characterstic

![image-20230429211403688](https://s2.loli.net/2023/04/29/wXmugioh9QrzsMj.png)

## 广播-扫描模式（Ibeacon模式）

> 

![image-20230429210009537](https://s2.loli.net/2023/04/29/8SbLPUxw9ErmC4s.png)

TX模块发送数据-->打包为具有TX_UUID的TX_Characteristic数据包-->打包为具有SUUID的uart_Service数据包--->通过BLE向外广播此uart_Service

->RX模块周期性扫描，扫描Tx的广播包，读取SUUID，是模块Profile中订阅的uart_Service

-->拆出TX_Characteristic-->根据UUID和Descriptor，拆包，最终得到Rx_byte.

# BLE_UUID配置

## 16位UUID

蓝牙核心规范制定了两种不同的UUID，一种是基本的128位UUID，一种是代替基本UUID的16位UUID。 所有的蓝牙技术联盟定义UUID共用了一个基本的UUID： `0x0000xxxx-0000-1000-8000-00805F9B34FB`

其中xxxx即为16为UUID

## 128位UUID

HC09相比HC08，额外支持配置128位UUID，AT接口：

~~~CPP
//广播ID，仅16位
AT+LUUID=ABCD   // BLE产品通过定义 LUUID 来区分、识别自己的产品。
    
//服务ID
AT+SUUID128=********-****-****-****-************
AT+SUUID16=xxxx

//透传ID
AT+TUUID16A=xxxx 和 AT+TUUID16B=xxx //
    
AT+TUUID128A=********-****-****-****-************
AT+TUUID128B=********-****-****-****-************
~~~

# HC09连接HC08实验

~~~CPp
//HC08配置：
AT+BAUD=115200
AT+NAME=EA_HC08
AT+LUUID=4396
AT+SUUID=4396
AT+TUUID=4396
    
//HC09配置
AT+UART=115200,N,1
AT+NAME=EA_HC09
AT+LUUID=4396
AT+SUUID16=4396
AT+TUUID16A=4396
AT+TUUID16B=4396
AT+ROLE=M  //模块重启，并进入主机模式，成功连接，开始透传
~~~

