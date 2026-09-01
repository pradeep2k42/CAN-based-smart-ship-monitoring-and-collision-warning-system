#include<lpc21xx.h>
#include"delay_header.h"
#define lcd_d 0xf<<14
#define RS    1<<12
#define E     1<<13

void lcd_initialization(void);
void lcd_command(unsigned char);
void lcd_data(unsigned char);
void lcd_integer(int);
void lcd_str(unsigned char *);

void lcd_initialization()
{
IODIR0=lcd_d|RS|E;
lcd_command(0x01);
lcd_command(0x02);
lcd_command(0x0c);
lcd_command(0x28);
}

void lcd_command(unsigned char cmd)
{
IOCLR0=lcd_d;
IOSET0=(cmd&0xf0)<<10;
IOCLR0=RS;
IOSET0=E;
delay_milliseconds(2);
IOCLR0=E;

IOCLR0=lcd_d;
IOSET0=(cmd&0x0f)<<14;
IOCLR0=RS;
IOSET0=E;
delay_milliseconds(2);
IOCLR0=E;
}

void lcd_data(unsigned char databyte)
{
IOCLR0=lcd_d;
IOSET0=(databyte&0xf0)<<10;
IOSET0=RS;
IOSET0=E;
delay_milliseconds(2);
IOCLR0=E;

IOCLR0=lcd_d;
IOSET0=(databyte&0x0f)<<14;
IOSET0=RS;
IOSET0=E;
delay_milliseconds(2);
IOCLR0=E;
} 

void lcd_integer(int number)
{
unsigned char arr[5];
signed char i=0;
if(number==0)
lcd_data('0');
else
{
if(number<0)
{
lcd_data('-');
number=-number;
}
while(number>0)
{
arr[i++]=number%10;
number=number/10;
}
}
for(--i;i>=0;i--)
lcd_data(arr[i]+48);
}

void lcd_str(unsigned char *s)
{
unsigned char count=0;
while(*s)
{
lcd_data(*s++);
count++;
if(count==16)
lcd_command(0xc0);
}
}

