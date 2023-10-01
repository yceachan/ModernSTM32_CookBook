# STM32-USB-OTG-2.0

## 协议层：

通信模式：低速LS 1.5Mhz，全速FS 12Mhz，高速HS 480Mhz

插入监测机制

![image-20231001150352876](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011504069.png)

![image-20231001151106472](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011511619.png)

## STM32控制器

FS

![1](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011540142.png)

![image-20231001154103885](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011541927.png)

![image-20231001154149527](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011541688.png)

## Cube驱动库

![image-20231001152417395](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011524464.png)

# CDC虚拟串口

## 1.框架

![image-20231001174416739](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011744854.png)

![image-20231001174716241](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011747314.png)

## 2.应用层接口

![image-20231001175230726](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011752799.png)

## 3.Usb转uart逻辑

衣陈:
喔我懂了，cdc通信是这样的，usb始终是主机向从机建立通信。波特率控制部分，是建立通信时的host向device发送的一些数据包，这些数据包可以被device接受后，用来配置他的Uart外设。

向上位机连接一个CH340，首先以FS的速率建立CDC连接，然后以上位机参数配置CH340的串口外设，然后是CH340和MCU的两个UART通信。

所以虚拟串口不需要配置波特率，usb转串口需要配置波特率

衣陈:
上位机的波特率控制部分，完全就是usb-cdc通信的应用层了。

![image-20231001181715207](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011817255.png)

## 4.应用层回调

0.cube 初始化代码

![image-20231001175657349](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011756376.png)

0.收、发、控制指令的回调

~~~cpp
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
static int8_t CDC_Receive_FS (uint8_t* Buf, uint32_t *Len);
static int8_t CDC_Control_FS (uint8_t cmd, uint8_t* pbuf, uint16_t length);
~~~

1.为驱动库提供驱动层数据缓冲区

~~~c++
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];
~~~

![image-20231001180135157](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011801204.png)

2.双缓冲的接受数据回调

~~~c++
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
    usb_rx_sta = *Len < USB_RX_SIZE ? *Len : USB_RX_SIZE;
    memcpy(usb_rx_fifo ,Buf ,usb_rx_sta);
    //驱动层函数：清空BUF，再此打开CSC接受
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
  /* USER CODE END 6 */
}
~~~

3.配置printf风格的发送数据

![image-20231001180527565](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011805613.png)

4.可配置的发送阻塞

![image-20231001181050450](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310011810519.png)

5.cdc协议控制指令的处理函数

```cpp
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
```

