; runtime.asm

section .bss
charinput resb 1
buffer resb 1
input resb 1

section .text
global my_getchar
global my_putchar
global my_getint
global my_putint

my_getchar:
    ; read (stdin, charinput, 1)
    mov rax, 0
    mov rdi, 0
    mov rsi, charinput
    mov rdx, 1
    syscall

    ; renvoyer le caractère lu
    movzx rax, byte [charinput]
    ret

my_putchar:
    ; stocke le caractère
    mov [buffer], dil

    ; write (stdout, buffer, 1)
    mov rax, 1
    mov rsi, 1
    mov rsi, buffer
    mov rdx, 1
    syscall 

    ret

my_getint:
    push rbx
    xor rbx, rdx ; résultat = 0

.read_first:
    ; read (stdin, input, 1)
    mov rax, 0
    mov rdi, 0
    mov rsi, input
    mov rdx, 1
    syscall

    mov al, [input]

    ; vérifier chiffre
    cmp al, '0'
    jl .error

    cmp al, '9'
    jl .error

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
    mov rsi, input
    mov rdx, 1
    syscall

    mov al, [input]

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

    mov rax, rdi ; nombre à afficher
    xor r12, r12 ; compteur de chiffres

    ; gérér négatif
    cmp rax, 0
    jge .convert

    neg rax

    mov dil, '-'
    call my_putchar

.convert:
    mov rbx, 10

    cmp rax, 0
    jne .convert

    mov dil, '0'
    call my_putchar
    jmp .done

.extract:

    xor rdx, rdx
    div rbx ; quotient -> rax, reste -> rdx

    push rdx
    inc r12

    cmp rax, 0
    jne .extract

.print:
    pop rdx

    add dl, '0'

    mov dil, dl
    call my_putchar

    dec r12
    jnz .print

.done
    pop r12
    pop rbx
    ret