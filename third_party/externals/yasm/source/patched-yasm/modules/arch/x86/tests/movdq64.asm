[bits 64]
movd eax, mm0
movd mm0, eax
movd rax, mm0
movd mm0, rax
movd [0], mm0
movd mm0, [0]

movd eax, xmm0
movd xmm0, eax
movd rax, xmm0
movd xmm0, rax
movd [0], xmm0
movd xmm0, [0]

movq [0], xmm0
movq xmm0, [0]
movq xmm0, xmm1
movq xmm1, xmm0

movq [0], mm0
movq mm0, [0]
movq mm0, mm1
movq mm1, mm0

movq rax, xmm0
movq xmm0, rax

movq rax, mm0
movq mm0, rax

