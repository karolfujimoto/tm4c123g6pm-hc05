#include "tm4c123gh6pm.h"
#include <inttypes.h>
#include <stdio.h>
/*
 * main.c
 */
void uart_setup();
void digital_setup();
void delay(int usec);
void uart_print(const char *word);
unsigned char rx_char();
void tx_char(unsigned char c);

int main(void) {
	unsigned char t;
	uart_setup();
	digital_setup();
	while(1) {
		tx_char('a');
	}
	return 0;
}
void uart_setup() {
	//Run Mode Clock Gating Control Register 1 - Enable clock to UART1-2 module in run mode - Pag. 460
	SYSCTL_RCGC1_R |= 1;
	//Run Mode Clock Gating Control Register 2 - Enable clock to GPIOC module in run mode - Pag. 464
	SYSCTL_RCGC2_R |= 4;
	//Universal Asynchronous Receiver/Transmitter Run Mode Clock Gating Control
	//Allow enable and disable UART1 modules in run mode - Pag. 344
	SYSCTL_RCGCUART_R |= 2;
	//System Control GPIO Run Mode Gating Control - Allow enable and disable GPIOC modules in run mode  - Pag. 340
	SYSCTL_RCGCGPIO_R |= 0x04;
	//UART1 Control - Clear UARTENable to configure it - Pag. 918
	UART1_CTL_R &= ~0x01;
	//UART Integer Baud-Rate Divisor - The baud rate is 16 times slower than CPU clock - 914
	UART1_IBRD_R = 104; //(16E6/16*38400) = 26,0417 104,16
	//UART Fractional Baud-Rate Divisor - Pag. 915
	UART1_FBRD_R = 11; //0,0417*64 = 7
	//UART Line Control - Set word lengh to 8 bits and enable FIFO - Pag. 916
	UART1_LCRH_R = 0x00000070;//(UART_LCRH_WLEN_8 | UART_LCRH_FEN);
	UART1_CTL_R |= 0x00000001;
	//GPIO Alternate Function Select - Set alternative function to PORTC4-5 - Pag. 671
	GPIO_PORTC_AFSEL_R = 0x30;
	//GPIO Digital Enable - Enable digital functions to PORTC - Pag. 682
	GPIO_PORTC_DEN_R = 0x30;
	//GPIO Port Control  - Specifies a peripheral signal to GPIO - Pag. 688
	//Table with values in page 1351
	GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000;
	//GPIO Analog Mode Select - Disable analog mode to PC4-5 - Pag. 687
	GPIO_PORTC_AMSEL_R &= ~0x30;
}

unsigned char rx_char() {
	while(UART0_FR_R&UART_FR_RXFE){};
	return (unsigned char) (UART0_DR_R&0xFF);
}
void tx_char(unsigned char c) {
	while(UART1_FR_R&UART_FR_TXFF) {};
	UART1_DR_R = c;
}
void uart_print(const char *word) {
	while (*word != '\0') {
		tx_char(*word++);
	}
	tx_char('\n');
}
void digital_setup() {
	//Run Mode Clock Gating Control Register 2 - Enable clock to GPIOB module in run mode - Pag. 464
	SYSCTL_RCGC2_R |= 10;
	//System Control GPIO Run Mode Gating Control - Allow enable and disable GPIO modules in run mode  - Pag. 340
	SYSCTL_RCGCGPIO_R |= 0x0A;
	//GPIO PORT Lock Register - Allow access to Commit Register - Pag. 684
	GPIO_PORTD_DEN_R = 0x00;
	GPIO_PORTB_DEN_R = 0x00;
	GPIO_PORTD_LOCK_R = 0x4C4F434B;
	//GPIO PORT Commit Register - Allow changes in PB - Pag. 685
	GPIO_PORTB_CR_R = 0x02;
	GPIO_PORTD_CR_R = 0x01;
	//GPIO_PORTD_PDR_R = 0x01;
	//GPIO PORT Data Direction - Set PB0 input / PB1 output - Pag. 26
	GPIO_PORTB_DIR_R = 0x02;
	GPIO_PORTD_DIR_R = 0x00;
	//GPIO PORT Analog Mode Select - Disable analog mode for PORTB0-1 - Pag. 687
	GPIO_PORTB_AMSEL_R &= ~0x02;
	GPIO_PORTB_AMSEL_R &= ~0x01;
	//GPIO PORT Interrupt Mask - Set mask to PORTB0 - Pag. 667
	GPIO_PORTD_IM_R = 0x00;
	//GPIO PORT Interrupt Sense - Set edge PORTB 0 edge sensitive - Pag. 664
	GPIO_PORTD_IS_R = 0x00;
	//GPIO PORT Interrupt Both Edges - Enable interruption for both edges PB0 - Pag. 665
	GPIO_PORTD_IBE_R = 0x01;
	GPIO_PORTD_IEV_R = 0x00;
	//GPIO PORT Interrupt Raw Status - Clear for prevent false interruptions - Pag. 668
	GPIO_PORTD_RIS_R = 0;
	//GPIO PORT Interrupt Clear - Clear interrupt flag - Pag. 670
	GPIO_PORTD_ICR_R = 0x01;
	//GPIO PORT Interrupt Mask - Set mask to PORTB0 - Pag. 667
	GPIO_PORTD_IM_R = 0x01;
	//Nested Vectored Interruption Enable - Enable interruption 1 (PORTB) - Pag. 142/105
	NVIC_EN0_R = 1 << 3;
	//GPIO PORT Data Reset - Set 0 to All Ports
	GPIO_PORTB_DATA_R = 0;
	GPIO_PORTD_DATA_R = 0;
	//GPIO PORT Digital Enable - Pag. 682
	GPIO_PORTB_DEN_R = 0x02;
	GPIO_PORTD_DEN_R = 0x01;
}
void delay(int usec) {
	NVIC_ST_CTRL_R = 0x00;
	//SysTick Reload Value Register - Set value to reload time counter - Pag. 140
	//(Required time (us) / (1/Clock)) -1
	NVIC_ST_RELOAD_R = (4*usec)-1;
	//SysTick Current Value Register - Reset time counter value - Pag. 141
	NVIC_ST_CURRENT_R = 0;
	//SysTick Control and Status Register - Enable counter and set clock 16Mhz/4 - Pag. 136
	NVIC_ST_CTRL_R = 0x05;
	while(NVIC_ST_CURRENT_R){}
	//SysTick Control and Status Register - Disable SysTick - Pag. 136
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_CURRENT_R = 0;
}
