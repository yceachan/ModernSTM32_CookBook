/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.h
  * @version        : v1.0_Cube
  * @brief          : Header for usbd_cdc_if.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"

/* USER CODE BEGIN INCLUDE */
#define USB_RX_SIZE 512
extern uint8_t  usb_rx_fifo[USB_RX_SIZE];
extern uint32_t usb_rx_sta;
#include "stdarg.h"
void usb_printf(const char * fmt , ...);

#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048

extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;



uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);



#ifdef __cplusplus
}
#endif

#endif /* __USBD_CDC_IF_H__ */

