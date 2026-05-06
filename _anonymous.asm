global _start
section .text
_start:
    push 5
    pop rsi
    push 4
    push 2
    pop rbx
    pop rsi
    add rsi, rbx
    push rsi
    pop rsi
    mov rax, 60
    mov rdi, 0
    syscall