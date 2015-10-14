[map all]
section .text
 mov ax, bx

section .foo nobits follows=.text
 resb 4

section .bss
 resb 4
