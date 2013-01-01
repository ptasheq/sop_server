#ifndef INIT_H
#define INIT_H

#include "libs.h"

#define ASCII_TO_INT 48
#define INT_AS_STR_LENGTH 9

void init();
void end();
void check_parameters(int, char*[]);
int strtoint(char *);
int power(int, int);
void msleep(unsigned int);
#endif
