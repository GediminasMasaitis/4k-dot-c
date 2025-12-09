bits 64
org 0x300000

%ifndef START_LOCATION
%error "START_LOCATION must be defined"
%endif

%define PAYLOAD_DEST 0x400000

ehdr:
    db 0x7F, "ELF", 2, 1, 1, 0

getbit:
    add    dl, dl
    jnz    getbit_ret
    xchg   eax, edx
    lodsb
    jmp short getbit_part2
    
    dw 2
    dw 0x3E
    
getbit_part2:
    xchg   eax, edx
    adc    dl, dl
getbit_ret:
    ret
    
    dq start_in_paddr
    dq phdr - ehdr
    
getgamma:
    push   1
    pop    rcx
.loop:
    call   rbp
    adc    ecx, ecx
    nop
    
    call   rbp
    jc     .loop
donedepacking:    
    jmp short getbit_ret
    
    dw 56
    dw 1
    dw 0
    dw 0
    dw 0

phdr:
    dd 1
    dd 7
    dq 0
    dq 0x300000
    
start_in_paddr:
    mov    edi, PAYLOAD_DEST
    jmp short start_in_palign
    db 0
    
    dq filesize
    dq 0x10000000

start_in_palign:
    mov    esi, payload_compressed
    mov    ebp, getbit
    lea    r12d, [rbp + getgamma - getbit]
    push   START_LOCATION
    mov    dl, 0x80

literal:
    movsb
    mov    bl, 2
nexttag:
    call   rbp
    jnc    literal

    call   rbp
    jnc    codepair
    xor    eax, eax
    push   1
    pop    rcx
    call   rbp
    jnc    shortmatch
    mov    bl, 2
    mov    al, 10h
.getmorebits:
    call   rbp
    adc    al, al
    jnc    .getmorebits
    jnz    domatch
    stosb
    jmp    nexttag

shortmatch:
    lodsb
    shr    eax, 1
    jz     donedepacking
    adc    ecx, ecx
    jmp    domatch_new_lastpos

codepair:
    call   r12
    sub    ecx, ebx
    jnz    normalcodepair
    call   r12
    jmp    domatch_lastpos

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

payload_compressed:
    incbin './build/4kc.ap'

filesize equ $ - ehdr