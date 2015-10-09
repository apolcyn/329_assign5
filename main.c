#include <msp430.h> 
#include <stdio.h>

void write_cmd(char cmd) {
	P2OUT = 0;

	// upper nibble
	P1OUT = cmd;
	__delay_cycles(10);
	P2OUT = BIT0;      // raise enable
	__delay_cycles(10);
	P2OUT = 0;   // lower enable

	// lower nibble
	__delay_cycles(10);
	P1OUT = cmd << 4;
	__delay_cycles(10);
	P2OUT = BIT0;
	__delay_cycles(10);
	P2OUT = 0;

    __delay_cycles(100000);  // wait a long time, allow operation to complete
    P1OUT = 0; // clear P1OUT,just keeping don't care lines low
}

void write_data(char data) {
	P2OUT = BIT2;
	P1OUT = 0;


	// upper nibble
	P1OUT = data;
	__delay_cycles(10);
	P2OUT |= BIT0;     // set enable high
	__delay_cycles(10);
	P2OUT &= BIT2;     // lower enable, but keep RS high

	// lower nibble
	__delay_cycles(10);
	P1OUT = data << 4;
	__delay_cycles(10);
	P2OUT |= BIT0;
	__delay_cycles(10);
	P2OUT &= BIT2;
    __delay_cycles(100000); // Wait a long time, allow operation to complete
    P1OUT = 0;  // clear P1OUT,just keeping don't care lines low
}

/* Writes a string of characters to DDRAM */
write_msg(char* arr) {
	while(*arr) {
		write_data(*arr++);
	}
}

/*
 * main.c
 */
int hours = 0, minutes = 0, seconds = 0;
char msg_buffer[20];

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    BCSCTL1 = CALBC1_1MHZ; // Set range
    DCOCTL = CALDCO_1MHZ;

    P1DIR |= BIT0 + 0xF0;
    P1SEL |= BIT0;

    P2DIR = BIT0 | BIT1 | BIT2;

    /* initialize */
     __delay_cycles(1000000); // delay 1 second after power up

    /* Write the first command to put it in nibble mode */
        P2OUT = 0;
        P1OUT = BIT5;
        __delay_cycles(10);
        P2OUT = BIT0;
        __delay_cycles(10);
        P2OUT = 0;

        write_cmd(BIT2 | BIT3 | BIT5); // 2 line mode, display on

        __delay_cycles(2000000);

        write_cmd(BIT0 | BIT1 | BIT2| BIT3); // display on, cursor on, blink on
        __delay_cycles(2000000);

        write_cmd(BIT0); // display clear
        __delay_cycles(2000000);

        write_cmd(BIT1 | BIT2); // increment mode, shift off

        write_cmd(BIT1); // return home, set DDRAM address to 0x00

        write_cmd(BIT2 | BIT3 | BIT5);  // function set, put in two line mode, nibble mode


    TACTL = TASSEL_1 + MC_1;

    TACCR0 = 32768;
    CCTL0 = CCIE;

    __enable_interrupt();
	
	return 0;
}

int increment_seconds() {
	seconds = (seconds + 1) % 60;
	return seconds == 0;
}

int increment_minutes(int val) {
	minutes = (minutes + val) % 60;
	return minutes == 0 && val;
}

void increment_hours(int val) {
	hours = (hours + val) % 24;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	P1OUT ^= BIT6;
	increment_hours(increment_minutes(increment_seconds()));

	if(hours < 10)
		sprintf(msg_buffer, "0%d:", hours);
	else
		sprintf(msg_buffer, "%d:", hours);

	if(minutes < 10)
		sprintf(msg_buffer + 3, "0%d:", minutes);
	else
	    sprintf(msg_buffer + 3, "%d:", minutes);

	if(seconds < 10)
		sprintf(msg_buffer + 6, "0%d", seconds);
	else
		sprintf(msg_buffer + 6, "%d", seconds);

	write_cmd(BIT1);
	write_msg(msg_buffer);

}
