.global enable_paging
.global switch_dir
.global flush_tlb_entry
.set KERNEL_CODE_SEGMENT, 0x08

.text


switch_dir:
    mov 4(%esp), %eax
    mov %eax, %cr3
#    lea .flush, %eax
#    jmp *%eax
#    jmp $KERNEL_CODE_SEGMENT, $.flush
#.flush:
    ret
enable_paging:
    mov %cr0, %eax
    or  $0x80000001, %eax
    mov %eax, %cr0
    ret

flush_tlb_entry:
    cli
    invlpg (%edi)
    sti
    ret
