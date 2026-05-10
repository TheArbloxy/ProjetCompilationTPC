global _start
section .text
_start:
    push 5
    push 4
    push 1
    pop rbx
    pop rsi
    sub rsi, rbx
    push rsi
    push 9
    push 3
    pop rbx
    pop rsi
    add rsi, rbx
    push rsi
    pop rsi
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