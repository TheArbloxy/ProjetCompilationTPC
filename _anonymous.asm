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
_add:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov dword [rbp - 4], edi
    mov dword [rbp - 8], esi
    mov eax, dword [rbp - 4]
    push rax
    mov eax, dword [rbp - 8]
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
_sub:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov dword [rbp - 4], edi
    mov dword [rbp - 8], esi
    mov eax, dword [rbp - 4]
    push rax
    mov eax, dword [rbp - 8]
    push rax
    pop rbx
    pop rax
    sub rax, rbx
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
_main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push 1
    push 2
    pop rsi
    pop rdi
    call _add
    push rax
    push 4
    push 2
    pop rsi
    pop rdi
    call _sub
    push rax
    pop rbx
    pop rax
    imul rax, rbx
    push rax
    pop rsi
    mov dword [rbp - 4], esi
    mov eax, dword [rbp - 4]
    push rax
    pop rdi
    call my_putint
    push 10
    pop rdi
    call my_putchar
    push 0
    pop rax
    mov rsp, rbp
    pop rbp
    ret
