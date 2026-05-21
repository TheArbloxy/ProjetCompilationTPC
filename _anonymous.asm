section .bss
    x: resb 1

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
_foo:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov dword [rbp - 4], edi
    mov dword [rbp - 8], esi
    push 10
    push 5
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
_foo2:
    push rbp
    mov rbp, rsp
    sub rsp, 16
_main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    call _foo2
    push rax
    pop rsi
    mov [x], esi
    push 10
    pop rdi
    call _foo
    push 10
    pop rdi
    call my_putchar
    push 1
    pop rax
    mov rsp, rbp
    pop rbp
    ret
