[bits 64]
l1:
mov dword [l2], l2
jc l3
db 0x0
l3:
l2 equ $-l1
