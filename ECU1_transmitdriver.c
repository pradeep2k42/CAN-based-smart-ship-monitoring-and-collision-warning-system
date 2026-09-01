#include <LPC21xx.H>
#include "CAN_header.h"

void can2_init(void){
    PINSEL1 |= 0x00014000;   // P0.23-->RD2 & P0.24-->TD2
    VPBDIV = 1;               // PCLK = CCLK = 60MHz (fixed for CAN)
    C2MOD = 0x1;               // CAN2 into Reset Mode
    AFMR  = 0x2;                // accept all receiving messages (data/remote)
    C2BTR = 0x001C001D;        // 125Kbps @ PCLK=60MHz
    C2MOD = 0x0;                // CAN2 into Normal Mode
}

void can2_tx(CAN2_MSG m1)
{
    C2TID1 = m1.id;
    C2TFI1 = (m1.dlc << 16);
    if(m1.rtr == 0){            // data frame
        C2TFI1 &= ~(1<<30);
        C2TDA1 = m1.byteA;
        C2TDB1 = m1.byteB;
    }
    else{
        C2TFI1 |= (1<<30);       // RTR=1
    }
    C2CMR = (1<<0)|(1<<5);      // start transmission & select TxBuf1
    while((C2GSR & (1<<3)) == 0); // wait for transmission complete
}

