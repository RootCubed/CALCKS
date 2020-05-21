#ifndef EEP_H_
#define EEP_H_

#include "EEPROMLocs.h"

void eep_initialize();
void eep_write_block(const char *, int, int);
void eep_read_block(void *, int, int);

char uart_getbyte();
void uart_print(const char *);

#endif /* EEP_H_ */