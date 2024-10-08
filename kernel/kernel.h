#ifndef KERNEL_H
#define KERNEL_H
#include "../lib/string/string.h"
#include "../lib/stdlib/stdlib.h"
#include "../lib/stdio/stdio.h"
#include "../drivers/vga/vga.h"
#include "../drivers/descriptor/descriptor.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/pit/pit.h"
#include "../drivers/syscall/syscall.h"
#include "../multiboot/multiboot.h"
#include "../memory/pmm/pmm.h"
#include "../memory/vmm/vmm.h"
#include "../filesystem/ext2/fs.h"
#include "../filesystem/ide/ide.h"
#endif
