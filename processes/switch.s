.global switch_process
.global start_process

# void switch_process(uint32_t** old_stack, uint32_t* new_stack);
switch_process:
    # Save current context to stack
    pushf
    pusha
    mov %cr3, %ecx          # Use ECX as temp register
    push %ecx
    
    # Save current ESP to old_stack
    mov 44(%esp), %ecx      # Get old_stack parameter (first argument)
    mov %esp, (%ecx)        # Store current ESP at *old_stack
    
    # Load new stack
    mov 48(%esp), %esp      # Get new_stack parameter and set as ESP
    
    # Restore context from new stack
    pop %ecx
    mov %ecx, %cr3
    popa
    popf
    
    ret

# void start_process(uint32_t* new_stack);
start_process:
    # Load new stack
    mov 4(%esp), %esp       # Get new_stack parameter and set as ESP
    
    # Restore context from stack
    pop %ecx
    mov %ecx, %cr3
    popa
    popf
    
    ret