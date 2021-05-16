section .text

Printf:

    push rbp 
    mov  rbp, rsp
    push rbx
    push rcx

    movsd xmm0, qword [rbp + 16]
    mov rax, 0x408F400000000000 ; 1000 on double
    movq xmm1, rax
    mulsd xmm0, xmm1
    cvtsd2si rax, xmm0

    xor rbx, rbx
    xor rdx, rdx

    cmp rax, 0
    jge CreateBuffer
    inc rbx
    mov rcx, -1
    mul rcx

    CreateBuffer:

    ; divisional part
    mov rsi, rbp
    mov rcx, 10

    ; '\n'
    dec rsi
    mov byte [rsi], 0x0A

    xor rdx, rdx
    div rcx
    dec rsi
    add dl, 0x30
    mov byte [rsi], dl

    xor rdx, rdx
    div rcx
    dec rsi
    add dl, 0x30
    mov byte [rsi], dl

    xor rdx, rdx
    div rcx
    dec rsi
    add dl, 0x30
    mov byte [rsi], dl

    dec rsi
    mov byte [rsi], 0x2E ; char '.'

    ; integer part

    WhileNotZero:

        xor rdx, rdx
        div rcx
        dec rsi
        add dl, 0x30
        mov byte [rsi], dl
        cmp rax, 0
        jne WhileNotZero

    cmp rbx, 0
    je GoPrint
    dec rsi
    mov byte [rsi], 0x2D ; char '-'

    GoPrint:

        ; "Output: "
        sub rsi, 8
        mov rbx, 0x203A74757074754F
        mov qword [rsi], rbx

        add rsp, 0x100
        mov rax, 1
        mov rdi, 1
        mov rdx, rbp
        sub rdx, rsi
        syscall

    pop rcx
    pop rbx
    mov rsp, rbp
    pop rbp
    ret
