//Piotr Makulec, 2026
//Written for and tested on Arduino (ATmega328p).

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"

// checking the callibration of the internal oscillator - OSCCAL register
int main(void) {
  USART_Init(MYUBRR);
  // unlock interrupt
  sei();

  uint8_t counter, pm = counter = OSCCAL - 20;
  while (1) {
    uart_puts("UART test, OSCCAL = ");
    uart_putint(counter, 10);
    uart_putc('\r');
    uart_putc('\n');
    _delay_ms(500);
    OSCCAL = counter++;
    if (counter > pm + 40) counter = pm;
  }
}
