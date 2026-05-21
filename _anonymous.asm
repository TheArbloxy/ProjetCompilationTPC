section .bss
    a: resd 1

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
_main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push 97
    push 4
    push 2
    push 0
    pop rax
    mov rsp, rbp
    pop rbp
    ret
