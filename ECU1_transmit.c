#include <lpc21xx.h>
#include "CAN_header.h"
#include "ECU1_transmitdriver.c"

#define TRIG  (1<<3)   // P0.3
#define ECHO  (1<<4)   // P0.4
#define LED   (1<<2)   // P0.2

#define SAFE_LIMIT     150
#define WARNING_LIMIT  50

unsigned int dist;
unsigned int timeout;

void usdelay(unsigned int n);
void msdelay(unsigned int n);
void distance_init(void);
unsigned int distance(void);
void pwm_init(void);
void set_servo_angle(unsigned int pulse_us);
unsigned int classify_status(unsigned int d);
void can2_send_value(unsigned int id, unsigned int value);

int main(void){
    unsigned int angle_pulse[3] = {1000, 1500, 2000};  // servo dummy sweep: 0,90,180 deg
    unsigned int i;
    unsigned int status;

    IODIR0 |= LED;
    IOCLR0  = LED;

    distance_init();
    pwm_init();
    can2_init();

    while(1){
        for(i = 0; i < 3; i++)
		{
            set_servo_angle(angle_pulse[i]);     // servo just moves, no data taken from it
            msdelay(700);

            dist = distance();
            status = classify_status(dist);

            if(dist != 1000)
			 IOSET0 = LED; 
			else 
			  IOCLR0 = LED;

            can2_send_value(0x102, dist);         // Distance only
            can2_send_value(0x103, status);       // Status: 0=SAFE,1=WARNING,2=DANGER

            msdelay(1000);
        }
    }
    //return 0;
}

// --- STATUS CLASSIFICATION --------------------------------------
unsigned int classify_status(unsigned int d)
{
    if(d == 1000)           
	 return 2;
    if(d > SAFE_LIMIT)        
	 return 0;
    if(d > WARNING_LIMIT)     
	return 1;

    return 2;
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

void msdelay(unsigned int n){
    unsigned int i;
    for(i = 0; i < n; i++)
        usdelay(1000);
}

// --- HC-SR04 FUNCTIONS -----------------------------------------------
void distance_init(void){
    IODIR0 |=  TRIG;
    IODIR0 &= ~ECHO;
    IOCLR0  =  TRIG;
}

unsigned int distance(void)
{
    timeout = 0;
    IOCLR0 = TRIG;
    usdelay(5);
    IOSET0 = TRIG;
    usdelay(15);
    IOCLR0 = TRIG;

    timeout = 0;
    while(!(IOPIN0 & ECHO))
	{
        timeout++;
        if(timeout > 50000)
		 return 1000;
    }

    T1PR  = 60 - 1;
    T1TCR = 0x03;
    T1TCR = 0x01;

    timeout = 0;
    while(IOPIN0 & ECHO)
	{
        timeout++;
        if(timeout > 50000)
		{
            T1TCR = 0x00;
            return 1000;
        }
    }
    T1TCR = 0x00;

    dist = T1TC / 58;
    if(dist < 2 || dist > 400) return 1000;
    return dist;
}

// --- SERVO PWM FUNCTIONS (dummy motion, PCLK=60MHz) -----------------------
void pwm_init(void){
    PINSEL0 |= 0x00000008;
    PWMTCR   = 0x02;
    PWMPR    = 60 - 1;
    PWMMR0   = 20000;
    PWMPCR   = 0x0800;
    PWMMCR   = 0x00000002;
    PWMLER   = 0x09;
    PWMTCR   = 0x09;
}

void set_servo_angle(unsigned int pulse_us){
    PWMMR3 = pulse_us;
    PWMLER = 0x08;
}

