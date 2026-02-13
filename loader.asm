bits 64
org 0x300000

%ifndef START_LOCATION
%error "START_LOCATION must be defined"
%endif

%define PAYLOAD_DEST 0x400000
%define G_HT         0x800000
%define HMUL         111

; Stack layout for up to 21 models (284 bytes):
;   [rsp+0]   output ptr (from push)
;   [rsp+16]  bitlength
;   [rsp+24]  pr0
;   [rsp+28]  pr1
;   [rsp+32]  ent[21]     (84 bytes, disp8, 4B stride)
;   [rsp+116] weights[21] (84 bytes, disp8)
;   [rsp+200] cmasks[21]  (84 bytes, disp32)
; Registers: r9=bpos, r10=base_prob, r11=tinymask, r13=num_models
;            r8=compressed_ptr, r14=bitpos, r15=value, ebp=range, ebx=low

ehdr:
    db      0x7F, "ELF", 2, 1, 1, 0      ; e_ident[0..7]        (bytes 0-7)

_start:                                  ; e_ident[8..15]       (bytes 8-15)
    mov     edi, payload_compressed      ;   5 bytes
    jmp     short init_b                 ;   2 bytes
    db      0                            ;   1 byte pad

    dw      2                            ; e_type = ET_EXEC     (bytes 16-17)
    dw      0x3E                         ; e_machine            (bytes 18-19)
    dd      1                            ; e_version            (bytes 20-23)
    dq      _start                       ; e_entry              (bytes 24-31)
    dq      56                           ; e_phoff              (bytes 32-39)

init_b:                                  ; e_shoff+e_flags+e_ehsize (bytes 40-53)
    mov     esi, PAYLOAD_DEST            ;   5 bytes
    push    START_LOCATION               ;   5 bytes
    jmp     short init_c                 ;   2 bytes
    dw      0                            ;   e_ehsize filler

    dw      56                           ; e_phentsize          (bytes 54-55)

phdr:                                    ; overlaps e_phnum..e_shstrndx (bytes 56-63)
    dd      1                            ; p_type  (= e_phnum=1)
    dd      7                            ; p_flags (= e_shnum=7, e_shstrndx=0)
    dq      0                            ; p_offset             (bytes 64-71)
    dq      0x300000                     ; p_vaddr              (bytes 72-79)
    dq      0x300000                     ; p_paddr (ignored)    (bytes 80-87)
    dq      filesize                     ; p_filesz             (bytes 88-95)
    dq      0x10000000                   ; p_memsz              (bytes 96-103)

init_c:                                  ; p_align (ignored)    (bytes 104-111)
    movzx   eax, word [rdi]              ;   3 bytes - bitlength
    movzx   r13d, byte [rdi+6]           ;   5 bytes - num_models
                                         ;   falls through to decompress4kc

decompress4kc:
    sub     rsp, 276
    push    rsi
    mov     [rsp+16], eax
    imul    eax, r13d
    dec     eax
    bsr     ecx, eax
    push    2
    pop     rax
    shl     eax, cl
    dec     eax
    or      eax, 15
    xchg    eax, r11d
    mov     eax, [rdi+2]
    xor     ecx, ecx
    xor     edx, edx
.wl:test    eax, eax
    jz      .wd
.wo:add     eax, eax
    jz      .wd
    jnc     .wz
    inc     edx
    jmp     .wo
.wz:mov     [rsp+116+rcx*4], edx
    mov     ebp, eax
    mov     bpl, [rdi+7+rcx]
    mov     [rsp+200+rcx*4], ebp
    inc     ecx
    jmp     .wl
.wd:mov     ecx, r13d
    lea     r8, [rdi+rcx+7]
    push    10
    pop     r10
    lea     ecx, [r11+1]
    mov     edi, G_HT
    xor     eax, eax
    rep stosq
    xor     r14d, r14d
    xor     r15d, r15d
    mov     ebp, 0x80000000
    xor     ebx, ebx
    push    31
    pop     rcx
.il:bt      [r8], r14d
    adc     r15d, r15d
    inc     r14d
    loop    .il
    xchg    eax, r9d
.body:
    mov     [rsp+24], r10d
    mov     [rsp+28], r10d
    lea     r12d, [r13-1]
.mdl:
    mov     eax, [rsp+r12*4+200]
    movzx   edx, al
    mov     ecx, r9d
    jrcxz   .bp0

    dec     ecx
    mov     esi, [rsp]
    mov     edi, ecx
    shr     edi, 3
    add     esi, edi
    movzx   edi, byte [rsi]
    bts     edi, 8
    not     ecx
    and     ecx, 7
    inc     ecx
    shr     edi, cl
    xor     eax, edi
    imul    eax, eax, HMUL
    add     al, dil
    dec     eax

.cl:test    dl, dl
    jz      .hr
    dec     esi
    test    dl, 0x80
    jz      .cs
    xor     al, [rsi]
    imul    eax, eax, HMUL
    add     al, [rsi]
    dec     eax
.cs:add     dl, dl
    jmp     .cl
.bp0:
    mov     esi, [rsp]
    imul    eax, eax, HMUL
    dec     eax
    jmp     .cl

.hr:mov     edi, eax
    and     eax, r11d
.pb:lea     ecx, [rax*8+G_HT]
    cmp     byte [rcx+6], 0
    je      .pe
    cmp     [rcx], edi
    je      .po
    inc     eax
    and     eax, r11d
    jmp     .pb
.pe:mov     [rcx], edi
    inc     byte [rcx+6]
.po:mov     [rsp+32+r12*4], ecx
    movzx   eax, byte [rcx+4]
    movzx   edi, byte [rcx+5]
    mov     ecx, [rsp+116+r12*4]
    test    al, al
    jz      .bo
    test    edi, edi
    jnz     .nb
.bo:add     ecx, 2
.nb:shl     eax, cl
    add     [rsp+24], eax
    shl     edi, cl
    add     [rsp+28], edi
    dec     r12d
    jns     .mdl
.mdd:
    mov     eax, ebp
    mov     ecx, [rsp+28]
    mul     ecx
    mov     esi, [rsp+24]
    add     esi, ecx
    div     esi
    xchg    edi, eax
    mov     eax, r15d
    sub     eax, ebx
    cmp     eax, edi
    sbb     esi, esi
    inc     esi
    jae     .ui
    mov     ebp, edi
    jmp     .rn
.ui:add     ebx, edi
    sub     ebp, edi

.rn:test    ebp, ebp
    js      .rd
.rl:add     ebx, ebx
    add     ebp, ebp
    bt      [r8], r14d
    adc     r15d, r15d
    inc     r14d
    test    ebp, ebp
    jns     .rl
.rd:push    1
    pop     rdi
    sub     edi, esi
    lea     ecx, [r13-1]
.ul:mov     eax, [rsp+32+rcx*4]
    inc     byte [rax+4+rdi]
    cmp     byte [rax+4+rsi], 1
    jbe     .nh
    shr     byte [rax+4+rsi], 1
.nh:dec     ecx
    jns     .ul
    mov     ecx, r9d
    imul    ecx, edi
    dec     ecx
    js      .nw
    xor     ecx, 7
    mov     edx, [rsp]
    bts     [rdx], ecx

.nw:inc     r9d
    cmp     r9d, [rsp+16]
    jl      .body

.dn:add     rsp, 284
    ret

payload_compressed:
    incbin  './build/4kc.paq'

filesize equ $ - ehdr
