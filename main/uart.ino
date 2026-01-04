#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "uart.h"

// ----- variables used in the interrupt -----
// receive buffer
volatile char UART_RxBuf[UART_RX_BUF_SIZE];
// "tail" - end index of the receive cyclical buffer
volatile uint8_t UART_RxTail;
// "head" - start index of the receive cyclical buffer
volatile uint8_t UART_RxHead;
// transmit buffer - useful with low baud rates and/or big amounts of data
volatile char UART_TxBuf[UART_TX_BUF_SIZE];
// "tail" - end index of the transmit cyclical buffer
volatile uint8_t UART_TxTail;
// "head" - start index of the transmit cyclical buffer
volatile uint8_t UART_TxHead;

void USART_Init(uint16_t baud) {
  // set the baud rate
  UBRR0H = (uint8_t)(baud >> 8);
  UBRR0L = (uint8_t)baud;
  // allow to transmit and receive
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  // format - 1 stop bit
  UCSR0C = (3 << UCSZ00);

  // the code below is RS485 specific
  #ifdef UART_DE_PORT
    // set the direction control pin as output
    UART_DE_DIR |= UART_DE_BIT;
    // set the device as a receiver as default
    UART_DE_RECEIVE;
  #endif

  // if RS485 is used
  #ifdef UART_DE_PORT
    // turn on the end of receive and of transmit interrrupts
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1<<TXCIE0);
  // if not
  #else
    //turn on the end of receive interrupt
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  #endif
}

//interrupt - change the state of the transmitter to a receiver after the trasmition - RS485 specific
#ifdef UART_DE_PORT
ISR(USART_TXC_vect) {
 UART_DE_PORT &= ~UART_DE_BIT;
}
#endif

// adding one bite to the transmit buffer - in text format; does not send it yet
void uart_putc(char data) {
  uint8_t tmp_head;
  // move the start index one bite, if there's an overflow, the index goes back to 0
  tmp_head = (UART_TxHead + 1) & UART_TX_BUF_MASK;
  // if the buffer is full, the function CAN wait for it to free up
  while(tmp_head == UART_TxTail) {}
  // write the bite to the buffer
  UART_TxBuf[tmp_head] = data;
  UART_TxHead = tmp_head;
  // initialize the interrupt - transmit the content of the transmit buffer
  // UDRIE - UDR buffer is empty
  UCSR0B |= (1 << UDRIE0);
}

// send a string 
void uart_puts(char *s) {
  register char c;
  // stop when char = '/0'
  while ((c = *s++)) uart_putc(c);
}

// send an int converted to a string
void uart_putint(int value, int radix) {
  char string[17];
  itoa(value, string, radix);
  uart_puts(string);
}

// interrupt - transmit the content of the transmit buffer
ISR(USART_UDRE_vect) {
  // check if the head and tail are different is there anything in there
  if(UART_TxHead != UART_TxTail) {
    // move the end index one bite, if there's an overflow, the index goes back to 0
    UART_TxTail = (UART_TxTail + 1) & UART_TX_BUF_MASK;
    // return the bite
    UDR0 = UART_TxBuf[UART_TxTail];
  } else {
    // zeroing the interrupt flag
    UCSR0B &= ~(1 << UDRIE0); 
  }
}

// taking one bite from the receive buffer - in text format
char uart_getc(void) {
  // check if both indexes are equal (the buffer is empty)
  if (UART_RxHead == UART_RxTail) return 0;
  // move the end index one bite, if there's an overflow, the index goes back to 0
  UART_RxTail = (UART_RxTail + 1) & UART_RX_BUF_MASK;
  // return the bite
  return UART_RxBuf[UART_RxTail];
}

// interrupt - receiving data and writing it to the receive buffer
// it happens when there's a bite in the UDR buffer
ISR(USART_RXC_vect) {
  uint8_t tmp_head;
  char data;
  // get the bite from the buffer
  data = UDR0;
  // move the start index one bite, if there's an overflow, the index goes back to 0
  tmp_head = (UART_RxHead + 1) & UART_RX_BUF_MASK;
  // check if the cyclical buffer is too small, "the head eats the tail"
  // the interrupt CANNOT wait for the buffer to free up
  if (tmp_head == UART_RxTail) {
    // placeholder for an error service, e.g. changing some global value
  } else {
    UART_RxHead = tmp_head;
    // write the bite to the receive buffer
    UART_RxBuf[tmp_head] = data;
  }
}
