section .bss
    x: resd 1
    y: resd 1
    z: resd 1

global _start

section .text
extern my_getchar
extern my_putchar
extern my_getint
extern my_putint

_start:
    push 10
    pop rsi
    mov [x], esi
    push 5
    pop rsi
    mov [y], esi
    push 13
    pop rsi
    mov [z], esi
    mov eax, [x]
    push rax
    mov eax, [y]
    push rax
    pop rbx
    pop rax
    cmp rax, rbx
    jg .Lbool_false_0
    jmp .Lor_right_1
.Lor_right_1:
    mov eax, [x]
    push rax
    mov eax, [z]
    push rax
    pop rbx
    pop rax
    cmp rax, rbx
    jg .Lbool_false_0
    jmp .Lbool_true_0
.Lbool_true_0:
    push 1
    jmp .Lbool_end_0
.Lbool_false_0:
    push 0
.Lbool_end_0:
    pop rsi
    mov [x], esi
    mov eax, [x]
    push rax
    pop rax
    cmp rax, 0
    jne .Lifelse_true_2
    jmp .Lifelse_false_2
.Lifelse_true_2:
    push 1
    pop rdi
    call my_putint
    jmp .Lifelse_end_2
.Lifelse_false_2:
    push 0
    pop rdi
    call my_putint
.Lifelse_end_2:
    push 10
    pop rdi
    call my_putchar
    mov rax, 60
    mov rdi, 0
    syscall