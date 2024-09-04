.global enable_paging
.global switch_dir
.global flush_tlb_entry

.text

enable_paging:
    mov %cr0, %eax
    or  $0x80000001, %eax
    mov %eax, %cr0
    ret

switch_dir:
    mov %eax, %cr3
    ret

flush_tlb_entry:
    cli
    invlpg (%edi)
    sti
    ret
