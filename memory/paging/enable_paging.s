.global enable_paging

enable_paging:
    # load page directory
    mov 4(%esp), %eax
    mov %eax, %cr3

    # enable paging
    mov %cr0, %ebx
    or  0x80000000, %ebx
    mov %ebx, %cr0
    ret
