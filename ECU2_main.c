#include <lpc21xx.h>
#include "CAN_header.h"
#include "CAN_Transdriver.c"

#define GAS_PIN    8    // P0.8 - Gas sensor digital output
#define FLAME_PIN  9    // P0.9 - Flame sensor digital output

#define TEMP_NORMAL_LIMIT   35
#define TEMP_WARNING_LIMIT  45

unsigned int timeout;

void usdelay(unsigned int n);
void msdelay(unsigned int n);
void adc_init(void);
unsigned int read_temperature(void);
unsigned int read_gas(void);
unsigned int read_flame(void);
unsigned int classify_temp(unsigned int t);
void can2_send_value(unsigned int id, unsigned int value);

int main(void)
{
    unsigned int temp, temp_status, gas, flame;

    IODIR0 &= ~((1<<GAS_PIN) | (1<<FLAME_PIN));   // inputs

    adc_init();
    can2_init();

    while(1){
        temp        = read_temperature();
        temp_status = classify_temp(temp);
        gas         = read_gas();
        flame       = read_flame();

        can2_send_value(0x201, temp);
        can2_send_value(0x202, temp_status);
        can2_send_value(0x203, gas);
        can2_send_value(0x204, flame);

        msdelay(1000);
    }
    //return 0;
}

// --- TEMPERATURE CLASSIFICATION -------------------------------
unsigned int classify_temp(unsigned int t)
{
    if(t > TEMP_WARNING_LIMIT)
	 return 2;   // High Temperature Alarm
    if(t > TEMP_NORMAL_LIMIT)  
	 return 1;   // Warning

    return 0;                              // Normal
}

// --- CAN HELPER ---------------------------------------------------
void can2_send_value(unsigned int id, unsigned int value){
    CAN2_MSG m;
    m.id    = id;
    m.rtr   = 0;
    m.dlc   = 4;
    m.byteA = value;
    m.byteB = 0;
    can2_tx(m);
}

// --- DELAY FUNCTIONS (Timer0, PCLK=60MHz because VPBDIV=1) -------------
void usdelay(unsigned int n){
    T0PR  = 60 - 1;
    T0TCR = 0x03;
    T0TCR = 0x01;
    while(T0TC < n);
    T0TCR = 0x00;
}

void msdelay(unsigned int n)
{
    unsigned int i;
    for(i = 0; i < n; i++)
        usdelay(1000);
}

// --- LM35 TEMPERATURE (ADC0.1 on P0.28) -----------------------------
void adc_init(void)
{
    PINSEL1 |= 0x01000000;    // P0.28 as AD0.1
    ADCR = 0x00200602;         // ADC operational, 10-bit, 11 clocks for conversion
}

unsigned int read_temperature(void)
{
    unsigned int result;
    unsigned int temp_c;

    ADCR |= (1<<24);            // start conversion
    while(!(ADDR & 0x80000000)); // wait till DONE
    result = ADDR;
    result = (result >> 6) & 0x3FF;   // 10-bit ADC value

    // LM35: 10mV per degree C, Vref=3.3V, 10-bit resolution
    temp_c = (result * 330) / 1024;   // scaled integer version of (result*3.3/1024)*100
    return temp_c;
}

unsigned int read_gas(void)
{
    if(((IOPIN0 >> GAS_PIN) & 1) == 0)
	 return 1;   // gas detected
    else
	 return 0;                                   // no gas
}

unsigned int read_flame(void)
{
    if(((IOPIN0 >> FLAME_PIN) & 1) == 0) 
	  return 1;  // fire detected
    else return 0;                                    // no fire
}



