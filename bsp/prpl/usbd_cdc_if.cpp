
#include "usbd_cdc_if.h"
#include "Userfifo.h"
uint8_t  usb_rx_fifo[USB_RX_SIZE];
uint32_t usb_rx_sta;

/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */
extern Userfifo usbcdc(UserTxBufferFS ,APP_TX_DATA_SIZE,
                       UserRxBufferFS , APP_RX_DATA_SIZE,
                       usb_rx_fifo,usb_rx_sta,
                       [](uint8_t* txbuf ,uint32_t len) {CDC_Transmit_FS(txbuf , len);});
extern "C"{

extern USBD_HandleTypeDef hUsbDeviceFS;
static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);



USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
        {
                CDC_Init_FS,
                CDC_DeInit_FS,
                CDC_Control_FS,
                CDC_Receive_FS,
                CDC_TransmitCplt_FS
        };

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
    /* USER CODE BEGIN 3 */
    /* Set Application Buffers */
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
    return (USBD_OK);
    /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
    /* USER CODE BEGIN 4 */
    return (USBD_OK);
    /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
    /* USER CODE BEGIN 5 */
    switch(cmd)
    {
        case CDC_SEND_ENCAPSULATED_COMMAND:

            break;

        case CDC_GET_ENCAPSULATED_RESPONSE:

            break;

        case CDC_SET_COMM_FEATURE:

            break;

        case CDC_GET_COMM_FEATURE:

            break;

        case CDC_CLEAR_COMM_FEATURE:

            break;

            /*******************************************************************************/
            /* Line Coding Structure                                                       */
            /*-----------------------------------------------------------------------------*/
            /* Offset | Field       | Size | Value  | Description                          */
            /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
            /* 4      | bCharFormat |   1  | Number | Stop bits                            */
            /*                                        0 - 1 Stop bit                       */
            /*                                        1 - 1.5 Stop bits                    */
            /*                                        2 - 2 Stop bits                      */
            /* 5      | bParityType |  1   | Number | Parity                               */
            /*                                        0 - None                             */
            /*                                        1 - Odd                              */
            /*                                        2 - Even                             */
            /*                                        3 - Mark                             */
            /*                                        4 - Space                            */
            /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
            /*******************************************************************************/
        case CDC_SET_LINE_CODING:

            break;

        case CDC_GET_LINE_CODING:

            break;

        case CDC_SET_CONTROL_LINE_STATE:

            break;

        case CDC_SEND_BREAK:

            break;

        default:
            break;
    }

    return (USBD_OK);
    /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
    /* USER CODE BEGIN 6 */
    usb_rx_sta = *Len < USB_RX_SIZE ? *Len : USB_RX_SIZE;
    memcpy(usb_rx_fifo ,Buf ,usb_rx_sta);
    //驱动层函数：清空BUF，再此打开CDC接受
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return (USBD_OK);
    /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
    uint8_t result = USBD_OK;
    /* USER CODE BEGIN 7 */
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
//  if (hcdc->TxState != 0){
//    return USBD_BUSY;
//  }
    uint32_t tick = HAL_GetTick();
    while (hcdc ->TxState) {
        if ( HAL_GetTick() - tick > 10) return USBD_BUSY;
        else break;
    }
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
    result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);

    tick = HAL_GetTick();
    while (hcdc ->TxState) {
        if ( HAL_GetTick() - tick > 10) return USBD_BUSY;
        else break;
    }
    /* USER CODE END 7 */
    return result;
}

//*CDC TX 回调函数。
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
    uint8_t result = USBD_OK;
    /* USER CODE BEGIN 13 */
    UNUSED(Buf);
    UNUSED(Len);
    UNUSED(epnum);
    /* USER CODE END 13 */
    return result;
}



/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
void usb_printf(const char *fmt, ...) {
    va_list args;
    uint32_t len;
    va_start(args,fmt);
    //将格式化的数据从可变参数列表写入指定大小缓冲区，以空字符填充。返回不包括空字符的写入字符数。
    len = vsnprintf((char *)UserTxBufferFS , APP_TX_DATA_SIZE , (char *)fmt , args);
    va_end(args);
    CDC_Transmit_FS(UserTxBufferFS , len);
}

}