; Derived from Crinkler by Rune L. H. Stubbe and Aske Simon Christensen
; Original: https://github.com/runestubbe/Crinkler (zlib license)
; Rewritten and ported to Linux ELF x86-64

bits 64
org 0x300000

%ifndef DIRECT_BITS
  %define DIRECT_BITS 24
%endif

%define PAYLOAD_DEST 0x400000

; Stack layout for up to 21 models (284 bytes):
;   [rsp+0]   PAYLOAD_DEST (pushed at .wd), popped by the final ret;
;             _start must be linked at the very start of the payload
;   [rsp+24]  ent[21]     (84 bytes, disp8, 4B stride)
;   [rsp+108] wc[21]      (168 bytes, disp8, 8B stride: weight@+0, cmask@+4)
;   (.wl runs with the range seed pushed where the body keeps PAYLOAD_DEST,
;    so both phases use the same displacements)
; Registers: r9=output_ptr, r10=output_end, r11=pr0, r13=num_models
;            r8=compressed_ptr, r14=bitpos, r15=code, ebp=range, ebx=pr1

ehdr:
    db      0x7F, "ELF"
_entry:
    mov     edi, payload_compressed
    mov     r9d, PAYLOAD_DEST
    db      0xB8
    dw      2
    dw      0x3E
    enter   284, 0
    dq      _entry
    dq      0x31
_c:
    movzx   r10d, word [rdi]
    mov     ebp, [rdi+2]    ; wmask decodes in ebp so rax keeps 0x3e0002 (a
                            ; mapped RWX pointer): the phdr islands below then
                            ; execute as harmless add [rax], reg
    jmp     short _d
phdr:
    db      1, 0, 0, 0
    db      7
    db      0x38, 0
    db      1
    times   8 db 0
    dq      0x300000
_d:
decompress4kc:
    push    1               ; range seed, popped at .wd
.wl:
.wo:add     ebp, ebp
    jz      .wd
    jnc     .wz
    ; p_filesz, page-rounded: bytes 00 X0 00.. execute as add [rax], reg,
    ; so the weight-increment path falls straight through the field
    dq      (filesize + 4095) & ~4095
    inc     edx
    jmp     .wo
    db      1, 0, 0, 0
.wz:mov     [rsp+108+rcx*8], edx
    mov     ebx, ebp
    mov     bl, [rdi+6+rcx]
    mov     [rsp+112+rcx*8], ebx
    inc     ecx
    jmp     .wl
.wd:mov     r13d, ecx
    lea     r8, [rdi+r13+6]
    pop     rbp
    push    r9              ; payload entry, consumed by the final ret
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
    mov     eax, [rsp+rdx*8+104]
    mov     cl, al
    mov     esi, r9d
.hash:
    crc32   eax, byte [rsi]
.next:
    dec     esi
    add     cl, cl
    jc      .hash
    jnz     .next

; Slot index = top DIRECT_BITS bits of the hash; table base = 0x80000000 so
; the base bit is OR'd in for free (max addr stays < 4 GB for <= 30 bits).
%if DIRECT_BITS == 30
.hr:and     al, 0xFD                     ; clear bit 1: keep slot 2-byte aligned
    stc
    rcr     eax, 1                       ; addr = 0x80000000 | ((hash & ~2) >> 1)
%else
.hr:shr     eax, 31 - DIRECT_BITS
    and     al, 0xFE
    bts     eax, 31
%endif
.po:mov     [rsp+20+rdx*4], eax
    movzx   esi, byte [rax]
    movzx   edi, byte [rax+1]
    mov     ecx, [rsp+rdx*8+100]
    test    esi, esi
    jz      .bo
    test    edi, edi
    jnz     .nb
.bo:add     ecx, 2
.nb:shl     esi, cl
    add     r11d, esi
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
.ul:mov     eax, [rsp+20+rcx*4]
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

.dn:ret                     ; pops PAYLOAD_DEST pushed at .wd

payload_compressed:
    incbin  './build/4kc.paq'

filesize equ $ - ehdr
