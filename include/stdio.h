#ifndef STDIO_H
#define STDIO_H

#include <limits.h>
#include <stdarg.h>
#include <drivers/vga/vga.h>
#include <string.h>
#include <stdlib.h>

int printf(const char* restrict format, ...);

#endif
