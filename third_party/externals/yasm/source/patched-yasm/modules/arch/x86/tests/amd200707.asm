bits 64
extrq xmm0, 5, 4
extrq xmm6, 0, 7
extrq xmm2, xmm3
insertq xmm0, xmm1, 5, 4
insertq xmm5, xmm6, 0, 7
insertq xmm2, xmm3
movntsd [0], xmm1
movntsd qword [0], xmm5
movntss [0], xmm3
movntss dword [0], xmm7

lzcnt ax, bx
lzcnt cx, word [0]
lzcnt dx, [0]
lzcnt eax, ebx
lzcnt ecx, dword [0]
lzcnt edx, [0]
lzcnt rax, rbx
lzcnt rcx, qword [0]
lzcnt rdx, [0]

popcnt ax, bx
popcnt cx, word [0]
popcnt dx, [0]
popcnt eax, ebx
popcnt ecx, dword [0]
popcnt edx, [0]
popcnt rax, rbx
popcnt rcx, qword [0]
popcnt rdx, [0]
