[bits 16]
mov al, 0
mov byte al, 0
mov al, byte 0
mov byte al, byte 0
;mov al, word 0
mov byte [0], 0
mov [0], word 0
mov dword [0], dword 0
;mov [0], 0
mov eax, 0
mov dword eax, 0
mov eax, dword 0
;mov eax, word 0
mov dword eax, dword 0
mov bx, 1h
mov cr0, eax
mov cr2, ebx
mov cr4, edx
mov ecx, cr4
mov dr3, edx
mov eax, dr7

mov [0], al
mov [0], bl
mov [1], al
mov [1], bl
mov ecx, edx
movsx ax, [ecx]
;movzx eax, [edx]
movzx ebx, word [eax]
movzx ecx, byte [ebx]
fnstenv [es:ecx+5]
nop

push cs
push word cs
push dword cs ; NASM unsupported
push ds
push es
push fs
push gs
pop ds
pop es
pop fs
pop gs
xchg al, bl
xchg al, [0]
xchg [0], al
xchg ax, bx
xchg cx, ax
xchg [0], ax
xchg [0], cx
xchg cx, [0]
xchg eax, edx
xchg ebx, eax
xchg ecx, ebx
xchg [0], ecx
xchg eax, [0]
in al, 55
in ax, 99
in eax, 100
in al, dx
in ax, dx
in eax, dx
out 55, al
out 66, ax
out 77, eax
out dx, al
out dx, ax
out dx, eax
lea bx, [5]
lea ebx, [32]
lds si, [0]
lds ax, [1]
;lds ax, dword [1]
les di, [5]
lds eax, [7]
les ebx, [9]
lss esp, [11]
lfs ecx, [13]
lgs edx, [15]
;; TODO: add arith stuff
imul eax, 4
aad
aam
aad 5
aam 10
shl al, 5
shl bl, 1
shl cl, cl
shr ax, 5
shr bx, 1
shr cx, cl
shld ax, bx, 5
shrd cx, dx, cl
shld ecx, edx, 10
shld eax, ebx, cl
retn
retf
retn 8
retf 16
enter 10, 12
setc al
setc [0]
;; TODO: add bit manip
int 10
;; TODO: add bound
;; TODO: add protection control
fld dword [0]
fld qword [4]
fld tword [16]
fld st2
fstp dword [0]
fstp st4
fild word [0]
fild dword [4]
fild qword [8]
fbld [100]
fbld tword [10]
fst dword [1]
fst qword [8]
fst st1
fxch
fxch st1
fxch st0, st2
fxch st2, st0
fcom dword [0]
fcom qword [8]
fcom st1
fcom st0, st0
fucom st7
fucomp st0, st5
fadd dword [10]
fadd qword [5]
fadd st0
fadd st0, st5
fadd to st7
fadd st6, st0
faddp ;NASM unsupported
faddp st2
faddp st5, st0
fiadd word [10]
fisub dword [4]
fldcw [0]
fnstcw [4]
fstcw word [4]
fnstsw [8]
fnstsw ax
fstsw word [0]
fstsw ax
ffree st1
ffreep st0 ;NASM unsupported
jc short label
jc label
label:
jz label
jz near label
loop label
jcxz label
jecxz label
call label
call [label]
call dword [label]
;jmp label
jmp short label
jmp near label
jmp far [label]
jmp far dword [label]
call far word [label]
loop short label
jcxz short label
jecxz short label
[bits 16]
push si
push esi
[bits 32]
push esi
