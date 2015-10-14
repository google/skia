movd %mm0, %eax
movd %eax, %mm0
movd %mm0, 0
movd 0, %mm(0)

movd %xmm0, %eax
movd %eax, %xmm0
movd %xmm0, 0
movd 0, %xmm0

movq %xmm0, 0
movq 0, %xmm0
movq %xmm1, %xmm0
movq %xmm0, %xmm1

movq %mm0, 0
movq 0, %mm0
movq %mm1, %mm0
movq %mm0, %mm1

