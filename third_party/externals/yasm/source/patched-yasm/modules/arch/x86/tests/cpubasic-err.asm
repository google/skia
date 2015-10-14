[cpu 8086]
pause

shl ax, 2

cpu 386
fninit

cpu 386 fpu
fninit

cpu 8086

shl ax, 1
shl ax, 2
movsd
push 0
