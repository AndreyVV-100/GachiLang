section .text

Scanf:

    push rbp 
    mov  rbp, rsp
    push rbx
    push rcx

    ; Print "Input: "
    mov rax, 0x00203a7475706e49
    push rax
    mov rax, 1
    mov rdi, 1
    mov rsi, rsp
    mov rdx, 7
    syscall
    pop rax

    sub rsp, 0x40 ; CreateBuffer

    ; Input

    mov rax, 0
    mov rdi, 0
    mov rsi, rsp
    mov rdx, 0x40
    syscall

    ; Translation

    xor rdi, rdi ; Counter
    xor rax, rax ; Number
    xor rdx, rdx ; Good mul number
    xor rbx, rbx ; Char iterator
    mov r10, 10  ; Base of system count

    mov rcx, rsp
    mov bl, byte [rcx]
    inc rcx

    ; Check '-'

    mov rsi, 1
    cmp bl, 0x2D ; char '-'
    jne WhileInteger
    mov rsi, -1
    mov bl, byte [rcx]
    inc rcx

    WhileInteger:

        mul r10
        sub bl, 0x30 ; char to num
        add rax, rbx

        mov bl, byte [rcx]
        inc rcx

        cmp bl, 0x2E ; char '.'
        je Divisional
        cmp bl, 0x0A ; char '\n'
        je CreateReturn
        
        jmp WhileInteger

    Divisional:

        mov bl, byte [rcx]
        inc rcx

        WhileDivisional:

            mul r10
            sub bl, 0x30 ; char to num
            add rax, rbx
            inc rdi

            mov bl, byte [rcx]
            inc rcx

            cmp bl, 0x0A ; char '\n'
            jne WhileDivisional

    CreateReturn:

        mul rsi ; Check sign
        cvtsi2sd xmm0, rax

        mov rax, 1
        cmp rdi, 0
        je CreatePoint

        WhileNeedPow:

            mul r10
            dec rdi
            cmp rdi, 0
            jne WhileNeedPow

        CreatePoint:

            cvtsi2sd xmm1, rax
            divsd xmm0, xmm1
            movq rax, xmm0

    EndScanf:    

        add rsp, 0x40
        pop rcx
        pop rbx
        mov rsp, rbp
        pop rbp
        ret
