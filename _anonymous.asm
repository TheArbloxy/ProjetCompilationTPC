section .bss
    x: resd 1
    y: resd 1

global _start

section .text
extern my_getchar
extern my_putchar
extern my_getint
extern my_putint

_start:
    push 0
    pop rsi
    mov [x], esi
    push 2
    pop rsi
    mov [y], esi
    mov eax, [x]
    push rax
    pop rax
    cmp rax, 0
    je .L0
    push 44
    pop rsi
    mov [y], esi
.L0:
    push 44
    pop rsi
    mov [y], esi
    mov eax, [y]
    push rax
    pop rdi
    call my_putint
    push 10
    pop rdi
    call my_putchar
    mov rax, 60
    mov rdi, 0
    syscall