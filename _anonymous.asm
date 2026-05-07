global _start
section .text
_start:
    push 10
    pop rsi
    mov rax, 60
    mov rdi, 0
    syscall