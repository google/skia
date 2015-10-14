global label
absolute 5000h
label:

section .text
global absval
absval equ 1000h

jmp absval
jmp label
