/**
 * DWT的寄存器驱动，对于CortexM4，仅有一个DWT。
 */
#include "DWT.h"
struct Reg32{
    volatile uint32_t reg;
};
#define DWT_CTRL           ((( Reg32*)0xE0001000)->reg)
#define DWT_CYCCNT         (((Reg32*)0xE0001004)->reg)
#define DEMCR              (((Reg32*)0xE000EDFC)->reg)
#define  DWT_ENABLE        (1 << 24)
#define DWT_CYCCNT_ENABLE  (1 << 0)
#define __BSP_SYS_FREQ             (168)
inline void DWT_init() {
    ATOMIC_SET_BIT(DEMCR, DWT_ENABLE);
    ATOMIC_SET_BIT(DWT_CTRL, DWT_CYCCNT_ENABLE);
}

DWT_Ctrl_SingleTon::DWT_Ctrl_SingleTon() {
    DWT_init();
}

void DWT_Ctrl_SingleTon::set_cnt(uint32_t val) {
    DWT_CYCCNT = val;
}

uint32_t  DWT_Ctrl_SingleTon::get_cnt() {
    return DWT_CYCCNT;
}
volatile void DWT_Ctrl_SingleTon::delay_us(uint32_t us) {
    DWT_CYCCNT = 0;
    uint32_t  hi =us * __BSP_SYS_FREQ;
    while (DWT_CYCCNT < hi);
}

uint32_t DWT_Ctrl_SingleTon::get_sysFreq() {
    return __BSP_SYS_FREQ;
}
