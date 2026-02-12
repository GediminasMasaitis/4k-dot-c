bits 64
org 0x300000

%ifndef START_LOCATION
%error "START_LOCATION must be defined"
%endif

%define PAYLOAD_DEST 0x400000
%define G_HT         0x800000
%define HMUL         111
%define FSZ          576

; ===================== ELF Header (64 bytes) =====================
ehdr:
    db 0x7F, "ELF", 2, 1, 1, 0     ; e_ident[0..7]
    dq 0                             ; e_ident[8..15] padding
    dw 2                             ; e_type  = ET_EXEC
    dw 0x3E                          ; e_machine = x86_64
    dd 1                             ; e_version
    dq _start                        ; e_entry
    dq phdr - ehdr                   ; e_phoff = 56 (overlaps)
    dq 0                             ; e_shoff
    dd 0                             ; e_flags
    dw 64                            ; e_ehsize
    dw 56                            ; e_phentsize

; ============= Program Header (overlapping at offset 56) =========
phdr:
    dd 1                             ; p_type  = PT_LOAD  (e_phnum=1)
    dd 7                             ; p_flags = RWX
    dq 0                             ; p_offset
    dq 0x300000                      ; p_vaddr
    dq 0x300000                      ; p_paddr
    dq filesize                      ; p_filesz
    dq 0x10000000                    ; p_memsz
    dq 0x1000                        ; p_align

; ========================= Entry Point ===========================
_start:
    mov     edi, payload_compressed
    mov     esi, PAYLOAD_DEST
    push    START_LOCATION
    ; fall through

; ================== Crinkler Decompressor ========================
decompress4kc:
    push    rbp
    push    rbx
    push    r14
    push    r15
    sub     rsp, FSZ
    mov     [rsp], rsi                  ; save output ptr

    ; --- Parse header ---
    mov     eax, [rdi]                  ; output_size
    movzx   r9d, byte [rdi+9]          ; base_prob
    movzx   ecx, byte [rdi+10]         ; hashbits
    movzx   edx, byte [rdi+8]          ; num_models
    mov     [rsp+32], edx

    ; hashmask = (1 << (hashbits-1)) - 1
    dec     ecx
    push    1
    pop     rsi
    shl     esi, cl
    dec     esi
    mov     [rsp+8], esi

    ; bitlength = output_size * 8 + 1
    shl     eax, 3
    inc     eax
    mov     [rsp+20], eax

    ; tinymask = NextPowerOf2(bitlength * num_models) - 1
    imul    eax, edx
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
    mov     [rsp+12], eax

    ; --- Parse weightmask -> weights[], cmasks[] ---
    mov     eax, [rdi+4]               ; weightmask
    xor     ecx, ecx
    xor     edx, edx
.wl:test    eax, eax
    jz      .wd
    cmp     ecx, [rsp+32]
    jge     .wd
    ; Count leading 1-bits as weight, 0-bit is separator
    ; add eax,eax shifts left and sets CF = old bit 31
.wo:add     eax, eax                   ; shift, CF = old high bit  (-6 vs bt+separate shift)
    jnc     .wz                        ; 0-bit = separator
    inc     edx
    jmp     .wo
.wz:mov     [rsp+48+rcx*4], edx        ; weights[m]
    movzx   esi, byte [rdi+12+rcx]     ; context mask byte
    mov     ebp, eax
    mov     bpl, sil                   ; low byte = ctx mask  (-5 vs and+or)
    mov     [rsp+176+rcx*4], ebp       ; cmasks[m]
    inc     ecx
    jmp     .wl

    ; --- Set up compressed data pointer & hash table ---
.wd:movzx   ecx, byte [rdi+8]
    lea     r8, [rdi+rcx+12]           ; r8 = comp base

    ; Clear hash table at G_HT
    mov     eax, [rsp+12]              ; tinymask
    inc     eax
    add     eax, eax                   ; dword count = (tinymask+1)*2
    xchg    ecx, eax                   ; ecx = count, eax = junk  (-1 vs lea)
    mov     edi, G_HT
    mov     [rsp+24], rdi              ; save ht base
    xor     eax, eax
    rep stosd

    ; --- Init arithmetic decoder ---
    xor     r14d, r14d                 ; stream_pos = 0
    xor     r15d, r15d                 ; code = 0
    mov     ebp, 0x80000000            ; range
    xor     ebx, ebx                   ; low = 0

    ; Read first 31 bits (use loop instruction)  (-1 vs cmp+jl)
    push    31
    pop     rcx
.il:bt      [r8], r14d
    adc     r15d, r15d
    inc     r14d
    loop    .il

    mov     [rsp+16], eax              ; bpos = 0 (eax=0 from rep stosd)

; ======================== MAIN DECODE LOOP =======================
.ml:mov     ecx, [rsp+16]
    cmp     ecx, [rsp+20]
    jge     .dn
    mov     [rsp+36], r9d              ; pr0 = base_prob
    mov     [rsp+40], r9d              ; pr1 = base_prob
    xor     ecx, ecx

; --- Model loop ---
.mdl:
    cmp     ecx, [rsp+32]
    jge     .mdd
    mov     [rsp+44], ecx
    mov     eax, [rsp+176+rcx*4]       ; h = cmask[m]
    movzx   edx, al                    ; ctx byte
    mov     ecx, [rsp+16]
    jecxz   .hsb

    ; --- Context hash (bpos > 0) ---
    dec     ecx
    mov     rsi, [rsp]
    mov     edi, ecx
    shr     edi, 3
    add     rsi, rdi
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
    dec     esi                        ; 32-bit dec is safe (addr < 4GB)  (-1 vs dec rsi)
    test    dl, 0x80
    jz      .cs
    xor     al, [rsi]
    imul    eax, eax, HMUL
    add     al, [rsi]
    dec     eax
.cs:add     dl, dl
    jmp     .cl

    ; --- Start-bit hash (bpos == 0) ---
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

    ; --- Hash reduce + linear probe ---
.hr:and     eax, [rsp+8]
    mov     edi, eax
    and     eax, [rsp+12]
    mov     rsi, [rsp+24]
.pb:lea     rcx, [rsi+rax*8]
    cmp     byte [rcx+6], 0
    jne     .pu
    mov     [rcx], edi
    inc     byte [rcx+6]
    jmp     .po
.pu:cmp     [rcx], edi
    je      .po
    inc     eax
    and     eax, [rsp+12]
    jmp     .pb

    ; --- Accumulate prediction ---
.po:mov     edx, [rsp+44]
    mov     [rsp+304+rdx*8], rcx
    movzx   eax, byte [rcx+4]
    movzx   edi, byte [rcx+5]
    mov     ecx, [rsp+48+rdx*4]
    test    al, al
    jz      .bo
    test    edi, edi
    jnz     .nb
.bo:add     ecx, 2
.nb:shl     eax, cl
    add     [rsp+36], eax
    shl     edi, cl
    add     [rsp+40], edi

    mov     ecx, [rsp+44]
    inc     ecx
    jmp     .mdl

; --- Arithmetic decode ---
.mdd:
    mov     eax, ebp
    mov     ecx, [rsp+40]
    mul     ecx
    mov     esi, [rsp+36]
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

    ; --- Renormalize ---
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
    xor     ecx, ecx
.ul:cmp     ecx, [rsp+32]
    jge     .ud
    mov     rax, [rsp+304+rcx*8]
    inc     byte [rax+4+rdi]
    cmp     byte [rax+4+rsi], 1
    jbe     .nh
    shr     byte [rax+4+rsi], 1
.nh:inc     ecx
    jmp     .ul

    ; --- Write output bit ---
.ud:test    edi, edi
    jz      .nw
    mov     ecx, [rsp+16]
    dec     ecx
    js      .nw
    xor     ecx, 7
    mov     rdx, [rsp]
    bts     [rdx], ecx

.nw:inc     dword [rsp+16]
    jmp     .ml

; --- Done ---
.dn:add     rsp, FSZ
    pop     r15
    pop     r14
    pop     rbx
    pop     rbp
    ret

; ========================= Payload ===============================
payload_compressed:
    incbin  './build/4kc.4kc'

filesize equ $ - ehdr
