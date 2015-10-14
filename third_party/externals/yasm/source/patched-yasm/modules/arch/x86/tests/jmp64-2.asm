[bits 64]
l1:
mov dword [l2], l2
jc short l3
l3:
l2 equ $-l1
