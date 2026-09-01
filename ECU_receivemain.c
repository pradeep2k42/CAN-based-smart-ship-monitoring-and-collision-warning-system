#include "CAN_header.h"
#include "CAN_RXUARTdriver.c"
#include "CAN_RXdriver.c"
#include "lcdheader.h"

#define ID_DISTANCE      0x102
#define ID_STATUS        0x103
#define ID_TEMP          0x201
#define ID_GAS           0x203
#define ID_FLAME         0x204

unsigned int last_distance;
unsigned int last_temp;
unsigned int last_gas;

void show_uart(unsigned int status){
    if(status == 0)      
	 uart0_tx_string("STATUS: SAFE\r\n");
    else if(status == 1) 
	 uart0_tx_string("STATUS: WARNING\r\n");
    else                   
	 uart0_tx_string("STATUS: DANGER\r\n");
}

void show_lcd(unsigned int distance, unsigned int status){
    lcd_command(0x80);              // line 1
    lcd_str((unsigned char*)"DIST: ");
    lcd_integer(distance);
    lcd_str((unsigned char*)"CM   ");

    lcd_command(0xC0);              // line 2
    if(status == 0)      
	 lcd_str((unsigned char*)"STATUS: SAFE   ");
    else if(status == 1) 
	 lcd_str((unsigned char*)"STATUS: WARN   ");
    else                   
	 lcd_str((unsigned char*)"STATUS:DANGER  ");
}

void show_lcd_ecu2(unsigned int temp, unsigned int gas, unsigned int flame){
    lcd_command(0x94);              // line 3
    lcd_str((unsigned char*)"TEMP:");
    lcd_integer(temp);
    lcd_str((unsigned char*)"C          ");

    lcd_command(0xD4);              // line 4
    lcd_str((unsigned char*)"GAS:");
    if(gas == 1) 
	lcd_str((unsigned char*)"Y ");
    else           
	 lcd_str((unsigned char*)"N ");

    lcd_str((unsigned char*)"FLAME:");
    if(flame == 1) 
	 lcd_str((unsigned char*)"Y   ");
    else             
	 lcd_str((unsigned char*)"N   ");
}

int main(){
    CAN2_MSG m1;

    can2_init();
    uart0_init(115200);
    lcd_initialization();

    while(1){
        can2_rx(&m1);

        if(m1.rtr == 0){
            switch(m1.id){
                case ID_DISTANCE:
                    last_distance = m1.byteA;
                    uart0_tx_string("DISTANCE  : ");
                    uart0_integer(last_distance);
                    uart0_tx_string(" CM\r\n");
                    break;

                case ID_STATUS:
                    show_uart(m1.byteA);
                    show_lcd(last_distance, m1.byteA);
                    break;

                case ID_TEMP:
                    last_temp = m1.byteA;
                    uart0_tx_string("TEMP      : ");
                    uart0_integer(last_temp);
                    uart0_tx_string(" C\r\n");
                    break;

                case ID_GAS:
                    last_gas = m1.byteA;
                    if(last_gas == 1) 
					 uart0_tx_string("GAS       : DETECTED\r\n");
                    else                 
					 uart0_tx_string("GAS       : NORMAL\r\n");
                    break;

                case ID_FLAME:
                    if(m1.byteA == 1) 
					 uart0_tx_string("FLAME     : DETECTED\r\n");
                    else                 
					 uart0_tx_string("FLAME     : NORMAL\r\n");
                    show_lcd_ecu2(last_temp, last_gas, m1.byteA);
                    break;
            }
        }
    }
}

