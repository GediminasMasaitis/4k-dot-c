; Derived from Crinkler by Rune L. H. Stubbe and Aske Simon Christensen
; Original: https://github.com/runestubbe/Crinkler (zlib license)
; Rewritten and ported to Linux ELF x86-64

bits 64
org 0x300000

%ifndef START_LOCATION
%error "START_LOCATION must be defined"
%endif

%ifndef DIRECT_BITS
  %define DIRECT_BITS 24
%endif

%define PAYLOAD_DEST 0x400000
%define HMUL         111

; Stack layout for up to 21 models (284 bytes):
;   [rsp+0]   output ptr (from push)
;   [rsp+28]  pr1               (pr0 is held in r11d)
;   [rsp+32]  ent[21]     (84 bytes, disp8, 4B stride)
;   [rsp+116] wc[21]      (168 bytes, disp8, 8B stride: weight@+0, cmask@+4)
; Registers: r9=output_ptr, r10=output_end, r11=pr0, r13=num_models
;            r8=compressed_ptr, r14=bitpos, r15=code, ebp=range, ebx=pr1

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
    dq      0x580000000

init_header:
    push    rsi
    mov     r13b, [rdi+6]

decompress4kc:
    mov     eax, [rdi+2]
.wl:
.wo:add     eax, eax
    jz      .wd
    jnc     .wz
    inc     edx
    jmp     .wo
.wz:mov     [rsp+116+rcx*8], edx
    mov     ebx, eax
    mov     bl, [rdi+7+rcx]
    mov     [rsp+120+rcx*8], ebx
    inc     ecx
    jmp     .wl
.wd:lea     r8, [rdi+r13+7]
    push    1
    pop     rbp
    mov     r9d, esi
.body:
    jmp     short .re
.rl:add     ebp, ebp
    bt      [r8], r14d
    adc     r15d, r15d
    inc     r14d
.re:test    ebp, ebp
    jns     .rl
    push    10
    pop     rbx
    mov     r11d, ebx
    lea     r12d, [r13-1]
.mdl:
    mov     eax, [rsp+r12*8+120]
    mov     dl, al
    mov     esi, r9d
.hash:
    xor     al, [rsi]
    imul    eax, eax, HMUL
    add     al, [rsi]
    dec     eax
.next:
    dec     esi
    add     dl, dl
    jc      .hash
    jnz     .next

.hr:shl     eax, 32 - DIRECT_BITS
    shr     eax, 31 - DIRECT_BITS
    pop     rdx
    push    rdx
    lea     ecx, [rax+rdx*2]
.po:mov     [rsp+32+r12*4], ecx
    movzx   eax, byte [rcx]
    movzx   edi, byte [rcx+1]
    mov     ecx, [rsp+r12*8+116]
    test    al, al
    jz      .bo
    test    edi, edi
    jnz     .nb
.bo:add     ecx, 2
.nb:shl     eax, cl
    add     r11d, eax
    shl     edi, cl
    add     ebx, edi
    dec     r12d
    jns     .mdl
.mdd:
    mov     eax, ebp
    mul     ebx
    add     ebx, r11d
    div     ebx
    cmp     r15d, eax
    sbb     edi, edi
    jae     .ui
    xchg    eax, ebp
    jmp     .rd
.ui:sub     r15d, eax
    sub     ebp, eax
.rd:lea     esi, [rdi+1]
    neg     edi
    mov     ecx, r13d
.ul:mov     eax, [rsp+28+rcx*4]
    inc     byte [rax+rdi]
    shr     byte [rax+rsi], 1
    jnz     .nh
    rcl     byte [rax+rsi], 1
.nh:loop    .ul
    shr     edi, 1
.wr:rcl     byte [r9], 1
    jnc     .nw
    inc     r9d
    jmp     short .wr
.nw:dec     r10d
    jnz     .body

.dn:jmp     START_LOCATION

payload_compressed:
    incbin  './build/4kc.paq'

filesize equ $ - ehdr
