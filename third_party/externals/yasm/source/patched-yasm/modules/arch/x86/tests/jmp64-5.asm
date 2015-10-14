[bits 64]
l1:
mov dword [l2], l2
jc l3
times 0x10001 db 0x0
l3:
l2 equ $-l1
