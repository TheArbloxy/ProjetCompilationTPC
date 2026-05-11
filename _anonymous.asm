section .bss
    a: resd 1
    b: resd 1

global _start

section .text
extern my_getchar
extern my_putchar
extern my_getint
extern my_putint

_start:
    push 5
    push 4
    push 1
    pop rbx
    pop rax
    sub rax, rbx
    push rax
    push 9
    push 3
    pop rbx
    pop rax
    cqo
    idiv rbx
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rbx
    pop rax
    imul rax, rbx
    push rax
    pop rsi
    mov [a], esi
    push 6
    push 3
    pop rbx
    pop rax
    sub rax, rbx
    push rax
    pop rsi
    mov [b], esi
    mov eax, [a]
    push rax
    pop rdi
    call my_putint
    mov rax, 60
    mov rdi, 0
    syscall