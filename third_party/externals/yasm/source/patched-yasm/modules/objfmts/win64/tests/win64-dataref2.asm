bits 64

section .text

bar:

mov eax, [foo wrt rip]
shl dword [foo wrt rip], 5
cmp dword [foo wrt rip], 16
cmp word [foo wrt rip], 10000
cmp dword [foo wrt rip], 10000000
je bar

section .data

foo:
dd 5
