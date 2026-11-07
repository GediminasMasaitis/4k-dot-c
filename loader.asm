; Derived from Crinkler by Rune L. H. Stubbe and Aske Simon Christensen
; Original: https://github.com/runestubbe/Crinkler (zlib license)
; Rewritten and ported to Linux ELF x86-64

bits 64
org 0x3E0000

%ifndef START_LOCATION
%error "START_LOCATION must be defined"
%endif

%ifndef DIRECT_BITS
  %define DIRECT_BITS 24
%endif

%define PAQ './build/4kc.paq'

ehdr:
    db      0x7F, "ELF"
_entry:
    bts     r9d, 22
    db      0x66, 0x41, 0xBA
    incbin  PAQ, 0, 2
    push    rax
    db      0xBE
    dw      2
    dw      0x3E
    inc     ebp
    jmp     short _c
    dq      _entry
    dq      0x31
_c:
    push    START_LOCATION
    jmp     short decompress4kc.top
_done:
    ret
    times   0x31-($-ehdr) db 0
phdr:
    db      1, 0, 0, 0
    db      7
    db      0x38, 0
    db      1
    times   8 db 0
    dq      0x3E0000
decompress4kc:
.adv:
    inc     r9d
    jmp     short .wr
.one:
    xchg    eax, ebp
    jmp     short .wr
    times   0x51-($-ehdr) db 0
    dq      (filesize + 4095) & ~4095
.incw:
    inc     ch
    jmp     short .wl
    db      1, 0, 0, 0
.rl:
    add     ebp, ebp
    bt      [rsi], r14d
    adc     r15d, r15d
    inc     r14d
.re:test    ebp, ebp
    jns     .rl
    mov     eax, ebp
    mul     ebx
    add     ebx, r11d
    div     ebx
    cmp     r15d, eax
    sbb     edi, edi
    jc      .one
    sub     r15d, eax
    sub     ebp, eax
.wr:rcl     byte [r9], 1
    jc      .adv
.ul:pop     rcx
    jrcxz   .ud
    add     ecx, edi
    inc     byte [rcx]
    xor     ecx, 1
    shr     byte [rcx], 1
    jnz     .ul
    rcl     byte [rcx], 1
    jmp     short .ul
.ud:dec     r10d
    jz      _done
.top:
    push    10
    pop     rbx
    mov     r11d, ebx
    mov     si, masks - ehdr
    db      0xBA
    incbin  PAQ, 2, 4
    push    rcx
.wl:add     edx, edx
    jz      .re
    jc      .incw
    mov     eax, edx
    lodsb
    mov     cl, al
    mov     edi, r9d
.hash:
    crc32   eax, byte [rdi]
.next:
    dec     edi
    add     cl, cl
    jc      .hash
    jnz     .next
%if DIRECT_BITS < 30
    shr     eax, 30 - DIRECT_BITS
%endif
    or      al, 3
    ror     eax, 1
    push    rax
    movzx   edi, byte [rax-1]
    movzx   eax, byte [rax]
    mov     cl, ch
    test    eax, eax
    jz      .bo
    test    edi, edi
    jnz     .nb
.bo:add     ecx, 2
.nb:shl     eax, cl
    add     r11d, eax
    shl     edi, cl
    add     ebx, edi
    jmp     short .wl

masks:
    incbin  PAQ, 6

filesize equ $ - ehdr
