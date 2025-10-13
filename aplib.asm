;; Contains a modified version of the original aPLib decompressor
;; Below is the original copyright notice

;;
;; aPLib compression library  -  the smaller the better :)
;;
;; fasm 64-bit assembler depacker
;;
;; Copyright (c) 1998-2014 Joergen Ibsen
;; All Rights Reserved
;;
;; http://www.ibsensoftware.com/
;;

format ELF64

public _start

section '.rodata' align 1
payload_compressed:
    file './build/4kc.ap'

section '.payload' align 1
payload_decompressed rb 4096*2

section '.text'
_start:
    push payload_decompressed
    pop rdi

    push payload_compressed
    pop rsi

    push START_LOCATION ; must be provided by -d to fasm

    ; Technically UB but because size doesn't exceed 32k
    ; and execution starts with literal, ends up being not needed
    ; xor    ebx, ebx

    mov    dl, 0x80
    push getbit
    pop rbp

literal:
    movsb
    mov    bl, 2
nexttag:
    call   rbp ; getbit
    jnc    literal

    xor    ecx, ecx
    call   rbp ; getbit
    jnc    codepair
    xor    eax, eax
    call   rbp ; getbit
    jnc    shortmatch
    mov    bl, 2
    inc    ecx
    mov    al, 10h
  .getmorebits:
    call   rbp ; getbit
    adc    al, al
    jnc    .getmorebits
    jnz    domatch
    stosb
    jmp    nexttag
codepair:
    call   getgamma_no_ecx
    sub    ecx, ebx
    jnz    normalcodepair
    call   getgamma
    jmp    domatch_lastpos

shortmatch:
    lodsb
    shr    eax, 1
    jz     donedepacking
    adc    ecx, ecx
    jmp    domatch_with_2inc

normalcodepair:
    xchg   eax, ecx
    dec    eax
    shl    eax, 8
    lodsb
    call   getgamma

    ; Uncomment the following 2 instructions if >32kb
    ; cmp    eax, 32000
    ; jae    short domatch_with_2inc
    cmp    ah, 5
    jae    short domatch_with_inc
    cmp    eax, 7fh
    ja     short domatch_new_lastpos

domatch_with_2inc:
    inc    ecx

domatch_with_inc:
    inc    ecx

domatch_new_lastpos:
    xchg   eax, r8d
domatch_lastpos:
    mov    eax, r8d
    mov    bl, 1

domatch:
    push   rsi
    mov    rsi, rdi
    sub    rsi, rax
    rep    movsb
    pop    rsi
    jmp    nexttag

getbit:
    add    dl, dl
    jnz    .stillbitsleft
    mov    dl, [rsi]
    inc    rsi
    adc    dl, dl
  .stillbitsleft:
    ret

getgamma:
    xor    ecx, ecx
getgamma_no_ecx:
    inc    ecx
  .getgammaloop:
    call   rbp ; getbit
    adc    ecx, ecx
    call   rbp ; getbit
    jc     .getgammaloop
donedepacking:
    ret

; Use to preserve System V calling convention
;donedepacking:
;    pop    rbx
;    ret
