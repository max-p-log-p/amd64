.macro pushaq
    push %rsp  # not sure %rsp is necessary.
    push %rax
    push %rcx
    push %rdx
    push %rbx
    push %rbp
    push %rsi
    push %rdi
.endm # pushaq


.macro popaq
    pop %rdi
    pop %rsi
    pop %rbp
    pop %rbx
    pop %rdx
    pop %rcx
    pop %rax
    pop %rsp  # not sure %rsp is necessary.
.endm # popaq


    .global jccCallout
jccCallout:
    pushaq
    pushf
    call  handleJccCallout
    popf
    popaq
    ret

    .global jmpCallout
jmpCallout:
    pushaq
    pushf
    call  handleJmpCallout
    popf
    popaq
    ret

    .global callCallout
callCallout:
    pushaq
    pushf
    call  handleCallCallout
    popf
    popaq
    jmp *callTarget
    .global retCallout
retCallout:
    pushaq
    pushf
    call  handleRetCallout
    popf
    popaq
    lea   4(%esp),%esp   # adds 4 to esp without modifying eflags
    ret
