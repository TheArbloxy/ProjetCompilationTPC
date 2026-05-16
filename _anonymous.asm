section .bss
    x: resd 1

global _start

section .text
extern my_getchar
extern my_putchar
extern my_getint
extern my_putint

_start:
    call _main
    mov rdi, rax
    mov rax, 60
    syscall
_add:
    mov eax, [a]
    push rax
    mov eax, [b]
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    ret
    mov rax, 0
    ret
_main:
    call my_getint
    push rax
    pop rsi
    mov [x], esi
    mov eax, [x]
    push rax
    pop rdi
    call my_putint
    push 10
    pop rdi
    call my_putchar
    push 0
    pop rax
    ret
    mov rax, 0
    ret
