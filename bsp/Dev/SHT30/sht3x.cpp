//
// Created by 衣陈 on 2023/6/28.
//

#include "Sht3x.h"
#define  DEVADR (0x44)
#define  FETCH  (0xE000)
#define  PERIOD_10mps_HIGH (0x2737)

void Sht3x::start_period() {
   this->cmdWrite(PERIOD_10mps_HIGH);
}

void Sht3x::fetch_data(double &T, double &RH) {

   uint8_t tx[2]{FETCH>>8,(uint8_t)FETCH&0x00FF};
   uint8_t rx[5]{0};
   this->seqWR(tx,2,rx,5);

   uint16_t T_raw  = ( rx[0]<<8 ) | rx[1];
   uint16_t RH_raw = ( rx[3]<<8 ) | rx[4];
   T=-45+175*double(T_raw)/double(0xFFFF);
   RH=100*double(RH_raw)/double(0xFFFF);
}
