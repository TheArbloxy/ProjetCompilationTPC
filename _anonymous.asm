section .bss
    __charinput resb 1
    __buffer resb 1
    __input resb 1

global _start

section .text

_start:
    call _main
    mov rdi, rax
    mov rax, 60
    syscall
_callWhile:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov dword [rbp - 4], edi
    movsxd rax, dword [rbp - 4]
    push rax
    push rax
    push 2
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    mov rsp, rbp
    pop rbp
    ret
_main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push 0
    pop rsi
    mov dword [rbp - 4], esi
.Lwhile_start_0:
    movsxd rax, dword [rbp - 4]
    push rax
    push rax
    push 100
    pop rbx
    pop rax
    cmp rax, rbx
    jl .Lwhile_true_0
    jmp .Lwhile_end_0
.Lwhile_true_0:
    movsxd rax, dword [rbp - 4]
    push rax
    push rax
    pop rdi
    call _callWhile
    push rax
    pop rsi
    mov dword [rbp - 4], esi
    jmp .Lwhile_start_0
.Lwhile_end_0:
    movsxd rax, dword [rbp - 4]
    push rax
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
    mov rsp, rbp
    pop rbp
    ret

my_getchar: ; read (stdin, __charinput, 1)
   mov rax, 0
   mov rdi, 0
   mov rsi, __charinput
   mov rdx, 1
   syscall

    ; renvoyer le caractère lu
   movzx rax, byte [__charinput]
   ret

my_putchar: ; stocke le caractère
   mov [__buffer], dil

   ; write (stdout, __buffer, 1)
   mov rax, 1
   mov rdi, 1
   mov rsi, __buffer
   mov rdx, 1
   syscall

   ret


my_getint:
   push rbx
   xor rbx, rbx ; résultat = 0

.read_first:
   ; read (stdin, __input, 1)
   mov rax, 0
   mov rdi, 0
   mov rsi, __input
   mov rdx, 1
   syscall

   ; Si read renvoie <= 0, on quitte
   cmp rax, 0
   jle .error

   mov al, [__input]

   ; vérifier chiffre
   cmp al, '0'
   jl .error

   cmp al, '9'
   jg .error
.loop:
   ; result = result * 10
   imul rbx, rbx, 10
   ; convertir ASCII -> entier
   movzx rax, al
   sub rax, '0'
   add rbx, rax

   ; lire caractère suivant
   mov rax, 0
   mov rdi, 0
   mov rsi, __input
   mov rdx, 1
   syscall

   ; si EOF, on s'arrête   cmp rax, 0
   jle .done
   mov al, [__input]
   ; continuer si chiffre

   cmp al, '0'
   jl .done

   cmp al, '9'
   jg .done

   jmp .loop
.done:
   mov rax, rbx
   pop rbx
   ret
.error:
   mov rax, 60
   mov rdi, 5
   syscall

my_putint:
   push rbx
   push r12

   mov rax, rdi
   xor r12, r12

   ; négatif
   cmp rax, 0
   jge .check_zero
   neg rax
   ; sauvegarder rax car my_putchar peut modifier les registres volatils
   push rax
   mov dil, '-'
   call my_putchar
   pop rax

.check_zero:
   cmp rax, 0
   jne .extract
   mov dil, '0'
   call my_putchar
   jmp .done_putint

.extract:
   mov rbx, 10

.loop_extract:
   xor rdx, rdx
   div rbx
   push rdx
   inc r12
   cmp rax, 0
   jne .loop_extract

.print:
   pop rdx
   add dl, '0'
   mov dil, dl
   call my_putchar
   dec r12
   jnz .print

.done_putint:
   pop r12
   pop rbx
   ret
