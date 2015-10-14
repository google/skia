extern value
mov ax, seg value
mov ds, ax

mov ax, seg local
mov es, ax

section .data

local:
