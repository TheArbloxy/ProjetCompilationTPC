section .bss

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
    mov dword [rbp - 12], edx
    mov eax, dword [rbp - 4]
    push rax
    mov eax, dword [rbp - 8]
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    mov eax, dword [rbp - 12]
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rdi
    call my_putint
_main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push 1
    pop rdi
    call _foo
    push 1
    pop rdi
    call my_putint
    push 0
    pop rax
    mov rsp, rbp
    pop rbp
    ret
