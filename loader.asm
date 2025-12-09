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

bits 64
global _start

section .payload align=1
payload_decompressed:
    resb 4096*2

section .text align=1
_start:
    mov    edi, payload_decompressed
    mov    esi, payload_compressed
    lea    ebp, [rsi + getbit - payload_compressed]
    lea    r12d, [rbp + getgamma - getbit]   ; Compute getgamma from getbit
    push   START_LOCATION
    mov    dl, 0x80

literal:
    movsb
    mov    bl, 2
nexttag:
    call   rbp ; getbit
    jnc    literal

    call   rbp ; getbit
    jnc    codepair
    xor    eax, eax
    push   1
    pop    rcx
    call   rbp
    jnc    shortmatch
    mov    bl, 2
    mov    al, 10h
  .getmorebits:
    call   rbp ; getbit
    adc    al, al
    jnc    .getmorebits
    jnz    domatch
    stosb
    jmp    nexttag
codepair:
    call   r12
    sub    ecx, ebx
    jnz    normalcodepair
    call   r12
    jmp    domatch_lastpos

shortmatch:
    lodsb
    shr    eax, 1
    jz     donedepacking
    adc    ecx, ecx
    jmp    domatch_new_lastpos

normalcodepair:
    xchg   eax, ecx
    dec    eax
    xchg   al, ah
    lodsb
    call   r12
    cmp    ah, 5
    jae    domatch_with_inc
    cmp    eax, 7fh
    ja     domatch_new_lastpos

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
    mov    esi, edi
    sub    esi, eax
    rep    movsb
    pop    rsi
    jmp    nexttag

getbit:
    add    dl, dl
    jnz    .stillbitsleft
    xchg   eax, edx
    lodsb
    xchg   eax, edx
    adc    dl, dl
  .stillbitsleft:
    ret

getgamma:
    push   1
    pop    rcx
  .getgammaloop:
    call   rbp
    adc    ecx, ecx
    call   rbp
    jc     .getgammaloop
donedepacking:
    ret
payload_compressed:
    incbin './build/4kc.ap'
