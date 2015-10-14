[bits 32]
movntps [0], xmm4
movntps dqword [0], xmm5
movntq [8], mm6
movntq qword [8], mm7
movss xmm0, [0]
movss xmm1, dword [8]
movss [0], xmm2
movss dword [8], xmm3
pcmpeqb xmm3, xmm4
pcmpgtw mm0, mm2
