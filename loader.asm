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

; Stack layout for up to 21 models (284 bytes):
;   [rsp+0]   output ptr (from push)
;   [rsp+28]  pr1               (pr0 is held in r11d)
;   [rsp+32]  ent[21]     (84 bytes, disp8, 4B stride)
;   [rsp+116] wc[21]      (168 bytes, disp8, 8B stride: weight@+0, cmask@+4)
; Registers: r9=output_ptr, r10=output_end, r11=pr0, r13=num_models
;            r8=compressed_ptr, r14=bitpos, r15=code, ebp=range, ebx=pr1

ehdr:
    db      0x7F, "ELF", 2, 1, 1, 0
_entry:
    mov     edi, payload_compressed
    xor     ebx, ebx
    db      0xB8
    dw      2
    dw      0x3E
    movzx   r10d, word [rdi]
    dq      _entry
    dq      0x31
_c:
    enter   276, 0
    mov     eax, [rdi+2]
    jmp     short _d
phdr:
    db      1, 0, 0, 0
    db      7
    db      0x38, 0
    db      1
    times   8 db 0
    dq      0x300000
_d:
    mov     esi, PAYLOAD_DEST
    push    rsi
    jmp     short _e
    dq      filesize
_e:
    db      0x3E, 0x44, 0x8D, 0x8E
    dd      0
decompress4kc:
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
.wd:mov     r13d, ecx
    lea     r8, [rdi+r13+7]
    push    1
    pop     rbp
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
    mov     edx, r13d
.mdl:
    mov     eax, [rsp+rdx*8+112]
    mov     cl, al
    mov     esi, r9d
.hash:
    crc32   eax, byte [rsi]
.next:
    dec     esi
    add     cl, cl
    jc      .hash
    jnz     .next

.hr:shl     eax, 32 - DIRECT_BITS
    shr     eax, 31 - DIRECT_BITS
    lea     ecx, [rax+0x800000]
.po:mov     [rsp+28+rdx*4], ecx
    movzx   eax, byte [rcx]
    movzx   edi, byte [rcx+1]
    mov     ecx, [rsp+rdx*8+108]
    test    al, al
    jz      .bo
    test    edi, edi
    jnz     .nb
.bo:add     ecx, 2
.nb:shl     eax, cl
    add     r11d, eax
    shl     edi, cl
    add     ebx, edi
    dec     edx
    jnz     .mdl
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
.bt:jmp     short .body
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
    jnz     .bt

.dn:jmp     START_LOCATION

payload_compressed:
    incbin  './build/4kc.paq'

filesize equ $ - ehdr
