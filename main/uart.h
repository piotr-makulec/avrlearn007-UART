#ifndef UART_H_
#define UART_H_

// clock rate - 16 MHz
#define FOSC 16000000
// baud rate - 9600
#define BAUD 9600
// UBBR value
#define MYUBRR FOSC/16/BAUD-1

// size of the receive buffer, always a power of 2
#define UART_RX_BUF_SIZE 32
// mask of the receive buffer
#define UART_RX_BUF_MASK (UART_RX_BUF_SIZE-1)
// size of the transmit buffer
#define UART_TX_BUF_SIZE 16
// mask of the transmit buffer
#define UART_TX_BUF_MASK (UART_TX_BUF_SIZE-1)

/*
 * // RS485 specific definitions
 * #define UART_DE_PORT PORTD
 * #define UART_DE_DIR DDRD
 * #define UART_DE_BIT (1 << PD2)
 * 
 * #define UART_DE_RECEIVE UART_DE_PORT &= ~UART_DE_PIN
 * #define UART_DE_TRANSMIT UART_DE_PORT |= UART_DE_PIN
 * 
 * UART_DE_DIR |= UART_DE_BIT;
 * UART_DE_RECEIVE;
 */

void USART_Init(uint16_t baud);
char uart_getc(void);
void uart_putc(char data);
void uart_puts(char *s);
void uart_putint(int value, int radix);

#endif
