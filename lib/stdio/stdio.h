#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <limits.h>
#include "../../drivers/vga/vga.h"
#include "../string/string.h"
#include "../stdlib/stdlib.h"

int printf(const char* restrict format, ...);

#endif
