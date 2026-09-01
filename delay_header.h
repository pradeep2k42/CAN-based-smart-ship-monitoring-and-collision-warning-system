#include<lpc21xx.h>
void delay_seconds(unsigned int);
void delay_milliseconds(unsigned int);

void delay_seconds(unsigned seconds)
{
T0PR=60000000-1;   // was 15000000-1 -> now matches PCLK=60MHz (VPBDIV=1)
T0TCR=0X01;
while(T0TC<seconds);
T0TCR=0X03;
T0TCR=0X00;
}

void delay_milliseconds(unsigned milliseconds)
{
T0PR=60000-1;       // was 15000-1 -> now matches PCLK=60MHz (VPBDIV=1)
T0TCR=0X01;
while(T0TC<milliseconds);
T0TCR=0X03;
T0TCR=0X00;
}

