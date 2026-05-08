global _start
section .text
_start:
    push 6
    push 3
    pop rbx
    pop rsi
    sub rsi, rbx
    push rsi
    pop rsi
    mov rax, 60
    mov rdi, 0
    syscall