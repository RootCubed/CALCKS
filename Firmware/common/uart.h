#ifndef UART_H_
#define UART_H_

#define UART_BUFSIZE 128
volatile char uart_buf[UART_BUFSIZE];
volatile int uart_wasRead;

void uart_init(void);
char *uart_getstr();

#endif