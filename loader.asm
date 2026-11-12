; Derived from Crinkler by Rune L. H. Stubbe and Aske Simon Christensen
; Original: https://github.com/runestubbe/Crinkler (zlib license)
; Rewritten and ported to Linux ELF x86-64

bits 64
org 0x3E0000

%ifndef START_LOCATION
%error "START_LOCATION must be defined"
%endif

%ifndef IMAGE_SIZE
%error "IMAGE_SIZE (uncompressed image size in bytes) must be defined"
%endif

%ifndef DIRECT_BITS
  %define DIRECT_BITS 24
%endif

%define PAQ './build/4kc.paq'

%macro AT 1
  %if ($-ehdr) > %1
    %error "loader layout overflow before offset %1"
  %endif
  times %1-($-ehdr) db 0
%endmacro

ehdr:
    db      0x7F, "ELF"
_entry:
    bts     ebx, 22
    inc     ebp
top:
    push    20
    pop     r10
    push    rcx
    AT 0x0F
    mov     esi, 0x3E0002
    push    40
    jmp     short rest
    AT 0x18
    dq      _entry
    dq      phdr - ehdr
one:
    xchg    eax, ebp
wr: rcl     byte [rbx], 1
    jnc     ul
    db      0x8D, 0x9B
phdr:
    dd      1
    jmp     short wr
    db      0
    db      0x38, 0
    db      1, 0
    times   5 db 0
    dq      0x3E0100
ul: pop     rcx
    jrcxz   top
    add     ecx, edi
    nop
    AT 0x4D
    db      0x49, 0xB8
    dq      0xF00
upd:
    inc     byte [rcx]
    xchg    eax, ecx
    db      0x35
    dd      1
    shr     byte [rax], 1
    jnz     ul
    rcl     byte [rax], 1
    jmp     short ul
rl: add     ebp, ebp
    bt      [rsi], r14d
    adc     r15d, r15d
    inc     r14d
re: test    ebp, ebp
    jns     rl
    xchg    eax, r10d
    mul     ebp
    div     r11d
    cmp     r15d, eax
    sbb     edi, edi
    jc      one
    sub     r15d, eax
    sub     ebp, eax
    jmp     short wr
rest:
    pop     r11
    mov     sil, masks - ehdr
    db      0xBA
    incbin  PAQ, 2, 4
    cmp     bx, IMAGE_SIZE & 0xFFFF
    jae     START_LOCATION
incw:
    inc     ch
wl: add     edx, edx
    jc      incw
    jz      re
    mov     eax, edx
    lodsb
    mov     cl, al
    mov     edi, ebx
hash:
    crc32   eax, byte [rdi]
next:
    dec     edi
    add     cl, cl
    jc      hash
    jnz     next
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
    jz      bo
    test    edi, edi
    jnz     nb
bo: add     ecx, 2
nb: add     eax, edi
    shl     eax, cl
    add     r11d, eax
    shl     edi, cl
    add     r10d, edi
    jmp     short wl

masks:
    incbin  PAQ, 6
