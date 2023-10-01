本bsp实现了统一风格的fifo通信接口，风格如下

~~~cpp
extern UartDev puart1;
extern Userfifo usbcdc;

  /* USER CODE BEGIN WHILE */
    while (1) {

        if (usbcdc.rx_sta) {
            usbcdc.print("USB loopback:%.*s\r\n", usbcdc.rx_sta, usbcdc.rx_fifo);
            usbcdc.clear();
        }
        if (puart1.rx_sta) {
            puart1.print("UART loopback:%.*s\r\n", puart1.rx_sta, puart1.rx_fifo);
            puart1.clear();
        }

    }
~~~

# Userfifo

Userfifo 类提供fifo通信 的api，如print ，rx_fifo[],rx_sta，但不维护任何资源

~~~cpp
class Userfifo{
public:
    Userfifo(uint8_t *_tx_fifo, uint32_t _TX_SIZE,
             uint8_t *_rx_fifo0, uint32_t _RX_SIZE,
             uint8_t *_rx_fifo, uint32_t &_rx_sta,
             void (*_render) (uint8_t *, uint32_t));
    uint8_t * rx_fifo ; uint32_t& rx_sta;  const uint32_t  RX_SIZE;


    void print(const char *fmt, ...);
    void clear();
private:
    void (*render) (uint8_t *, uint32_t);
    uint8_t * tx_fifo ; const uint32_t  TX_SIZE;

protected:
    uint8_t * rx_fifo0 ;
};
~~~

使用va_list，调用render回调，实现printf风格发送数据

~~~cpp
void Userfifo::print(const char *fmt, ...) {
    va_list args;
    uint32_t len;
    va_start(args,fmt);
    //将格式化的数据从可变参数列表写入指定大小缓冲区，以空字符填充。返回不包括空字符的写入字符数。
    len = vsnprintf((char *)tx_fifo , TX_SIZE , (char *)fmt , args);
    va_end(args);
    render(tx_fifo , len);
}
~~~

需要在类外定义缓冲区资源与实现缓冲逻辑，适用于驱动库已提供完善的回调接口情境。如USB-CDC应用

![image-20231001222117426](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310012221459.png)

驱动库已提供完善的回调接口，全局缓冲资源的逻辑可以内嵌于移植库。

通过指针传递，在应用层提供面向对象级的抽象。

![image-20231001221226479](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310012212532.png)

# UserDev

对于驱动库未提供完善回调接口的情景，可以继承Userfifo，自行实现回调函数和维护缓冲资源，如串口应用

~~~cpp
class UartDev : public  Userfifo{
public:
    UartDev(uint8_t *_tx_fifo, uint32_t _TX_SIZE,
            uint8_t *_rx_fifo0, uint32_t _RX_SIZE,
            uint8_t *_rx_fifo, uint32_t &_rx_sta,
    void (*_render) (uint8_t *, uint32_t),
            UART_HandleTypeDef *_huart);
    UART_HandleTypeDef *huart;
    void rx_start_it();
    void rx_callback(uint8_t rxd);
};
~~~

![image-20231001221949644](https://gitee.com/Ea_Chan/chan-imgs/raw/master/imgs/202310012219680.png)

