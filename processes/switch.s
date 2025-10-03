.global switch_process
.global start_process
.global save_child_registers
.set CLEAR_ERRNO_INTNO, 0x08


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

# void start_process(uint32_t* new_stack);
start_process:
    # Load new stack
    mov 4(%esp), %esp               # Get new_stack parameter and set as ESP
    
    # Restore context from new stack
    pop %eax                        # Restore ds
    mov %eax, %ds
    popa                            # Restore general purpose registers

    add $CLEAR_ERRNO_INTNO, %esp    # Skip int_no and err_code

    iret                            # Return from interrupt (restores eip, cs, eflags, esp, ss)

# void save_child_registers(uint32_t** child, registers_t* regs);
save_child_registers:
	push %ebp
	mov %esp, %ebp			# Save parent sp

    mov 8(%ebp),%ecx        # Get child sp
	mov $0, %eax			# Set child fork return value

	mov (%ecx), %esp		# Set sp
	
    pushf
	pusha
    #mov %cr3, %ebx
    #push %ebx

	mov %esp, (%ecx)		# Save child sp

	mov %ebp, %esp			# Restore parent
	pop %ebp
	ret
