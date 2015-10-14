[bits 32]
off	equ	-4
pos	equ	4

mov	[ebp+off], eax
mov	[ebp+pos], eax
mov	[ebp-off], eax
mov	[ebp-pos], eax
