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
;   [rsp+4]   hashmask
;   [rsp+8]   tinymask
;   [rsp+12]  bpos
;   [rsp+16]  bitlength
;   [rsp+20]  ht base
;   [rsp+24]  pr0
;   [rsp+28]  pr1
;   [rsp+32]  ent[21]     (84 bytes, disp8, 4B stride)
;   [rsp+116] weights[21] (84 bytes, disp8)
;   [rsp+200] cmasks[21]  (84 bytes, disp32)

; ===================== ELF Header =====================
ehdr:
    db 0x7F, "ELF", 2, 1, 1, 0
    dq 0
    dw 2
    dw 0x3E
    dd 1
    dq _start
    dq phdr - ehdr
    dq 0
    dd 0
    dw 64
    dw 56
phdr:
    dd 1
    dd 7
    dq 0
    dq 0x300000
    dq 0x300000
    dq filesize
    dq 0x10000000
    dq 0x1000

; ========================= Entry =========================
_start:
    mov     edi, payload_compressed
    mov     esi, PAYLOAD_DEST
    push    START_LOCATION

decompress4kc:
    push    rbp
    push    rbx
    push    r12
    push    r13
    push    r14
    push    r15
    sub     rsp, 276
    push    rsi                         ; [rsp]=output ptr

    mov     eax, [rdi]
    movzx   r9d, byte [rdi+9]
    movzx   ecx, byte [rdi+10]
    movzx   r13d, byte [rdi+8]         ; num_models in r13d

    dec     ecx
    push    1
    pop     rsi
    shl     esi, cl
    dec     esi
    mov     [rsp+4], esi               ; hashmask

    shl     eax, 3
    inc     eax
    mov     [rsp+16], eax              ; bitlength

    imul    eax, r13d
    dec     eax
    bsr     ecx, eax
    push    2
    pop     rax
    shl     eax, cl
    cmp     eax, 16
    jge     .ts
    push    16
    pop     rax
.ts:dec     eax
    mov     [rsp+8], eax               ; tinymask

    ; --- Parse weightmask ---
    mov     eax, [rdi+4]
    xor     ecx, ecx
    xor     edx, edx
.wl:test    eax, eax
    jz      .wd
    cmp     ecx, r13d
    jge     .wd
.wo:add     eax, eax
    jnc     .wz
    inc     edx
    jmp     .wo
.wz:mov     [rsp+116+rcx*4], edx       ; weights[m] (disp8)
    mov     ebp, eax
    mov     bpl, [rdi+11+rcx]
    mov     [rsp+200+rcx*4], ebp       ; cmasks[m] (disp32)
    inc     ecx
    jmp     .wl

    ; --- Compressed data & hash table ---
.wd:mov     ecx, r13d
    lea     r8, [rdi+rcx+11]

    mov     eax, [rsp+8]
    inc     eax
    xchg    ecx, eax
    mov     edi, G_HT
    mov     [rsp+20], edi
    xor     eax, eax
    rep stosq

    ; --- Init decoder ---
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

    mov     [rsp+12], eax              ; bpos = 0

; ================= MAIN LOOP ===================
.ml:mov     ecx, [rsp+12]
    cmp     ecx, [rsp+16]
    jl      .body

.dn:pop     rax                         ; discard output ptr
    add     rsp, 276
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    pop     rbp
    ret

.body:
    mov     [rsp+24], r9d
    mov     [rsp+28], r9d
    xor     r12d, r12d

.mdl:
    cmp     r12d, r13d
    jge     .mdd
    mov     eax, [rsp+r12*4+200]       ; cmasks[m] (disp32)
    movzx   edx, al
    mov     ecx, [rsp+12]
    jecxz   .hsb

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

.hsb:
    imul    eax, eax, HMUL
    dec     eax
.sl:test    dl, dl
    jz      .hr
    test    dl, 0x80
    jz      .ss
    imul    eax, eax, HMUL
    dec     eax
.ss:add     dl, dl
    jmp     .sl

.hr:and     eax, [rsp+4]
    mov     edi, eax
    and     eax, [rsp+8]
    mov     esi, [rsp+20]
.pb:lea     ecx, [rsi+rax*8]
    cmp     byte [rcx+6], 0
    jne     .pu
    mov     [rcx], edi
    inc     byte [rcx+6]
    jmp     .po
.pu:cmp     [rcx], edi
    je      .po
    inc     eax
    and     eax, [rsp+8]
    jmp     .pb

.po:mov     [rsp+32+r12*4], ecx        ; ent[m] (disp8)
    movzx   eax, byte [rcx+4]
    movzx   edi, byte [rcx+5]
    mov     ecx, [rsp+116+r12*4]       ; weights[m] (disp8)
    test    al, al
    jz      .bo
    test    edi, edi
    jnz     .nb
.bo:add     ecx, 2
.nb:shl     eax, cl
    add     [rsp+24], eax
    shl     edi, cl
    add     [rsp+28], edi

    inc     r12d
    jmp     .mdl

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
    jae     .ui
    mov     ebp, edi
    xor     esi, esi
    jmp     .rn
.ui:add     ebx, edi
    sub     ebp, edi
    push    1
    pop     rsi

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

    ; --- Update counters ---
    mov     ecx, r13d
    dec     ecx
.ul:mov     eax, [rsp+32+rcx*4]        ; ent[m] (disp8)
    inc     byte [rax+4+rdi]
    cmp     byte [rax+4+rsi], 1
    jbe     .nh
    shr     byte [rax+4+rsi], 1
.nh:dec     ecx
    jns     .ul

    ; --- Write output bit ---
    test    edi, edi
    jz      .nw
    mov     ecx, [rsp+12]
    dec     ecx
    js      .nw
    xor     ecx, 7
    mov     edx, [rsp]
    bts     [rdx], ecx

.nw:inc     dword [rsp+12]
    jmp     .ml

; ========================= Payload ===============================
payload_compressed:
    incbin  './build/4kc.4kc'

filesize equ $ - ehdr
