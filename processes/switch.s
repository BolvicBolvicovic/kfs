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

# void switch_process(uint32_t** old_stack, uint32_t* new_stack);
switch_process:
    # Get parameters
	mov 4(%esp), %eax				# Get old_stack
	mov 8(%esp), %ebx				# Get new_stack
	add $56, %esp					# Move esp after return adress, where the registers have been saved by the cpu and the irq stub
    
    mov %esp, (%eax)       	 		# Store current ESP at *old_stack
    mov %ebx, %esp  	    		# Set new_stack parameter as ESP
    
    # Restore context from new stack
    pop %eax                		# Restore ds
    mov %eax, %ds
    
    popa                    		# Restore general purpose registers
    
    add $CLEAR_ERRNO_INTNO, %esp    # Skip int_no and err_code
    iret                    		# Return from interrupt (restores eip, cs, eflags, esp, ss)

# void switch_process_user(uint32_t** old_stack, uint32_t* new_stack, uint32_t dir);
switch_process_user:
    # Get parameters
	mov 4(%esp), %eax				# Get old_stack
	mov 8(%esp), %ebx				# Get new_stack
	mov 12(%esp), %ecx				# Get dir
	add $56, %esp					# Move esp after return adress, where the registers have been saved by the cpu and the irq stub
    
    mov %esp, (%eax)       	 		# Store current ESP at *old_stack
	mov %ecx, %cr3					# Reload cr3
    mov %ebx, %esp  	    		# Set new_stack parameter as ESP
    
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
