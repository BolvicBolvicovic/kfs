#ifndef STDLIB_H
#define STDLIB_H
#include <stddef.h>
#include <stdbool.h>

int     isnum(const char c);
size_t  itoa(char* dest, int nb);
int     atoi(const char *nptr);

#endif