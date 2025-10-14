.global	tss_flush
.global	switch_process
.global	switch_process_user
.global	start_process
.set	CLEAR_ERRNO_INTNO, 0x08

tss_flush:
	mov $0x28, %ax
	ltr %ax
	ret

# typedef struct {
# 	uint32_t	ds;
# 	uint32_t	edi, esi, ebp, esp, ebx, edx, ecx, eax;
# 	uint32_t	int_no, err_code;
# 	uint32_t	eip, cs, eflafs, useresp, ss;
# } registers_t;

# void switch_process(uint32_t* new_stack);
switch_process:
    # Get parameters
	mov 4(%esp), %eax				# Get new_stack
    mov %eax, %esp  	    		# Set new_stack parameter as ESP
    
    # Restore context from new stack
    pop %eax                		# Restore ds
    mov %eax, %ds
    
    popa                    		# Restore general purpose registers
    
    add $CLEAR_ERRNO_INTNO, %esp    # Skip int_no and err_code
    iret                    		# Return from interrupt (restores eip, cs, eflags, esp, ss)

# void switch_process_user(uint32_t* new_stack, uint32_t dir);
switch_process_user:
    # Get parameters
	mov 4(%esp), %eax				# Get new_stack 
	mov 8(%esp), %ebx				# Get dir
    
	mov %ebx, %cr3					# Reload cr3
    mov %eax, %esp  	    		# Set new_stack parameter as ESP
    
    # Restore context from new stack
    pop %eax                		# Restore ds
    mov %eax, %ds
    
    popa                    		# Restore general purpose registers
    
    add $CLEAR_ERRNO_INTNO, %esp    # Skip int_no and err_code
    iret                    		# Return from interrupt (restores eip, cs, eflags, esp, ss)

# void start_process(uint32_t* new_stack);
start_process:
    mov 4(%esp), %esp
    
    # Restore context from new stack
    pop %eax
    mov %eax, %ds
    popa

    add $CLEAR_ERRNO_INTNO, %esp

    iret
