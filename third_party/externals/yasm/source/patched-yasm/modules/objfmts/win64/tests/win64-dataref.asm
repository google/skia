BITS 64

global		x86ident
global		__savident
extern		foobar		; :proc
extern		foobar2		; :abs
extern		foobar3		; :qword
extern		foobar4		; :byte

[SECTION .data]
__savident	dd 0              
savidentptr	dd __savident
savidentptr2	dq __savident
x86identptr	dd x86ident
x86identptr2	dq x86ident
foobarptr	dd foobar
foobarptr2	dq foobar
foobar2ptr	dd foobar2
foobar2ptr2	dq foobar2
foobar3ptr	dd foobar3
foobar3ptr2	dq foobar3
xptr		dd x
xptr2		dq x

[SECTION .bss]
x		resq	1
y		resq	1

[SECTION .text]
x86ident:
		; extern with :proc
		; This instruction generates a different relocation than
		; MASM does at present.
		mov	ebx, foobar		; WTF ML64.. this had []
		mov	rcx, qword foobar
		lea	rdx, [foobar wrt rip]
		mov	rax, [foobar+rcx]
		mov	rax, qword foobar
		mov	rbx, qword foobar
		movzx	rax, byte [foobar wrt rip]
		movzx	rax, byte [foobar+rax]

		; local "proc"
		; See note above
		mov	ebx, trap
		mov	rcx, qword trap
		; MASM generates a REL32 reloc for this even though it's in
		; the same section.  I don't know why, as the call instruction
		; below doesn't cause a reloc, so the linker can't be moving
		; functions around within an object!
		lea	rdx, [trap wrt rip]
		mov	rax, [trap+rcx]
		mov	rax, qword trap
		mov	rbx, qword trap
		; MASM generates a REL32 reloc for this even though it's in
		; the same section.  I don't know why, as the call instruction
		; below doesn't cause a reloc, so the linker can't be moving
		; functions around within an object!
		movzx	rax, byte [trap wrt rip]
		movzx	rax, byte [trap+rax]

		; with :abs
		;mov	ebx,[foobar2]
		;mov	rcx,offset foobar2
		;lea	rdx, foobar2
		;mov	rax, qword ptr foobar2[rcx]
		;mov	rax, foobar2
		;mov	rbx, foobar2
		;movzx	rax, byte ptr foobar2
		;movzx	rax, byte ptr foobar2[rax]

		; with :qword
		; See note above
		mov	ebx, foobar3
		mov	ebx, [foobar3 wrt rip]
		mov	rcx, qword foobar3
		lea	rdx, [foobar3 wrt rip]
		mov	rax, [foobar3+rcx]
		mov	rax, [foobar3 wrt rip]
		mov	rbx, [foobar3 wrt rip]
		movzx	rax, byte [foobar3 wrt rip]
		movzx	rax, byte [foobar3+rax]

		; local var (dword)
		; See note above
		mov	ebx, __savident
		mov	ebx,[__savident wrt rip]
		mov	rcx, qword __savident
		lea	rdx, [__savident wrt rip]
		mov	rax, [__savident+rcx]
		mov	rax, [__savident wrt rip]
		mov	rbx, [__savident wrt rip]
		movzx	rax, byte [__savident wrt rip]
		movzx	rax, byte [__savident+rax]

		; local var (qword)
		; See note above
		mov	ebx, savidentptr2
		mov	ebx, [savidentptr2 wrt rip]
		mov	rcx, qword savidentptr2
		lea	rdx, [savidentptr2 wrt rip]
		mov	rax, [savidentptr2+rcx]
		mov	rax, [savidentptr2 wrt rip]
		mov	rbx, [savidentptr2 wrt rip]
		movzx	rax, byte [savidentptr2 wrt rip]
		movzx	rax, byte [savidentptr2+rax]

		; bss local var (qword)
		; See note above
		mov	ebx, y
		mov	ebx, [y wrt rip]
		mov	rcx, qword y
		lea	rdx, [y wrt rip]
		mov	rax, [y+rcx]
		mov	rax, [y wrt rip]
		mov	rbx, [y wrt rip]
		movzx	rax, byte [y wrt rip]
		movzx	rax, byte [y+rax]

		call	foobar

		call	trap

		ret

trap:		sub	rsp, 256
		int3
		add	rsp, 256
.end

[SECTION .pdata]
dd	trap
dd	trap.end wrt trap
dd	$$xdatasym

[SECTION .xdata]
$$xdatasym:
db	1, 7, 2, 0, 7, 1, 0x20, 0

[SECTION _FOO]
foo_foobar3ptr	dd foobar3
foo_foobar3ptr2	dq foobar3
		mov	ebx, [foobar3 wrt rip]
		mov	rcx, qword foobar3
		lea	rdx, [foobar3 wrt rip]
		mov	rax, [foobar3+rcx]
		mov	rax, [foobar3 wrt rip]
		mov	rbx, [foobar3 wrt rip]
		movzx	rax, byte [foobar3 wrt rip]
		movzx	rax, byte [foobar3+rax]

