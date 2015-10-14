jmp 1234:5678		; YASM: far jump
jmp near 1234:5678	; YASM: near jump; NASM: mismatch in operand sizes
jmp far 1234:5678	; YASM: far jump; NASM: mismatch in operand sizes
;dw seg (1234:5678)
far1 equ 1234:5678
jmp far1		; both: near jump
jmp near far1		; both: near jump
jmp far far1		; YASM: far jump; NASM: value referenced by FAR is not relocatable
dw seg far1
jmp far2		; both: near jump
jmp near far2		; both: near jump
jmp far far2		; YASM: far jump; NASM: value referenced by FAR is not relocatable
dw seg far2
far2 equ 1234:5678
;mov ax, [1234:5678]	; YASM: invalid segment in effective address; NASM: invalid segment override
;mov ax, 1234:5678	; YASM: immediate does not support segment; NASM: invalid combination of opcode and operands
