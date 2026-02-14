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
;   [rsp+24]  pr0
;   [rsp+28]  pr1
;   [rsp+32]  ent[21]     (84 bytes, disp8, 4B stride)
;   [rsp+116] weights[21] (84 bytes, disp8)
;   [rsp+200] cmasks[21]  (84 bytes, disp32)
; Registers: r9=bpos, r10=bitlength, r11=tinymask, r13=num_models
;            r8=compressed_ptr, r14=bitpos, r15=value, ebp=range, ebx=low

ehdr:
    db      0x7F, "ELF", 2, 1, 1, 0

load_input:
    mov     edi, payload_compressed
    jmp     short zero_ebx
    db      0

    dw      2
    dw      0x3E

zero_ebx:
    xor     ebx, ebx
    jmp     short setup_stack

    dq      load_input
    dq      56

setup_stack:
    enter   276, 0
    movzx   eax, word [rdi]
    mov     r10d, eax
    jmp     short load_output

    dw      0
    dw      56

phdr:
    dd      1
    dd      7
    dq      0
    dq      0x300000

load_output:
    mov     esi, PAYLOAD_DEST
    jmp     short init_header
    db      0

    dq      filesize
    dq      0x10000000

init_header:
    push    rsi
    mov     r13b, [rdi+6]
    mul     r13d

decompress4kc:
    dec     eax
    bsr     ecx, eax
    push    2
    pop     rax
    shl     eax, cl
    dec     eax
    xchg    eax, r11d
    mov     eax, [rdi+2]
    xor     ecx, ecx
.wl:
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
    lea     ecx, [r11+1]
    mov     edi, G_HT
    rep stosq
    stc
    rcr     eax, 1
    xchg    eax, ebp
    mov     cl, 31
.il:bt      [r8], r14d
    adc     r15d, r15d
    inc     r14d
    loop    .il
.body:
    push    10
    pop     rax
    lea     rdi, [rsp+24]
    stosd
    stosd
    lea     r12d, [r13-1]
.mdl:
    mov     eax, [rsp+r12*4+200]
    mov     dl, al
    mov     esi, [rsp]
    xor     edi, edi
    mov     ecx, r9d
    jrcxz   .hash_finish

    dec     ecx
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
.hash_finish:
    imul    eax, eax, HMUL
    add     al, dil
    dec     eax
    jmp     short .cl_next

.cl_hash:
    xor     al, [rsi]
    imul    eax, eax, HMUL
    add     al, [rsi]
    dec     eax
.cl_next:
    dec     esi
    add     dl, dl
    jc      .cl_hash
    jnz     .cl_next

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

.rn:jmp     short .re
.rl:add     ebx, ebx
    add     ebp, ebp
    bt      [r8], r14d
    adc     r15d, r15d
    inc     r14d
.re:test    ebp, ebp
    jns     .rl
.rd:push    1
    pop     rdi
    sub     edi, esi
    lea     ecx, [r13-1]
.ul:mov     eax, [rsp+32+rcx*4]
    inc     byte [rax+4+rdi]
    shr     byte [rax+4+rsi], 1
    jnz     .nh
    rcl     byte [rax+4+rsi], 1
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
    cmp     r9d, r10d
    jl      .body

.dn:jmp     START_LOCATION

payload_compressed:
    incbin  './build/4kc.paq'

filesize equ $ - ehdr
