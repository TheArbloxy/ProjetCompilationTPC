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
    pop rsi
    mov dword [rbp - 4], esi
    push 4
    pop rsi
    mov dword [rbp - 4], esi
    push 2
    pop rsi
    mov dword [rbp - 4], esi
    push 0
    pop rax
    mov rsp, rbp
    pop rbp
    ret
