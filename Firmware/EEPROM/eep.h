/*
 * eep.h
 *
 * Created: 20.03.2020 22:23:13
 *  Author: liamb
 */ 


#ifndef EEP_H_
#define EEP_H_

#include "EEPROMLocs.h"

void eep_initialize();
void eep_write_block(const __flash char *, int, int);
void eep_read_block(void *, int, int);

#endif /* EEP_H_ */