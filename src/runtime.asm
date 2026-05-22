; runtime.asm

section .bss
    __charinput resb 1
    __buffer resb 1
    __input resb 1

section .text
global my_getchar
global my_putchar
global my_getint
global my_putint

my_getchar:
    ; read (stdin, __charinput, 1)
    mov rax, 0
    mov rdi, 0
    mov rsi, __charinput
    mov rdx, 1
    syscall

    ; renvoyer le caractère lu
    movzx rax, byte [__charinput]
    ret

my_putchar:
    ; stocke le caractère
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
    xor rbx, rdx ; résultat = 0

.read_first:
    ; read (stdin, __input, 1)
    mov rax, 0
    mov rdi, 0
    mov rsi, __input
    mov rdx, 1
    syscall

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

    mov al, [__input]

    ; continuer si chiffre
    cmp al, '0'
    jl .done

    cmp al, '9'
    jl .done

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

    mov dil, '-'
    call my_putchar

.check_zero:

    cmp rax, 0
    jne .extract

    mov dil, '0'
    call my_putchar
    jmp .done

.extract:

    mov rbx, 10

.loop:

    xor rdx, rdx
    div rbx

    push rdx
    inc r12

    cmp rax, 0
    jne .loop

.print:

    pop rdx

    add dl, '0'

    mov dil, dl
    call my_putchar

    dec r12
    jnz .print

.done:

    pop r12
    pop rbx

    ret