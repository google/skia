movd %mm(0), %eax
movd %eax, %mm(0)
movd %mm(0), %rax
movd %rax, %mm(0)
movd %mm(0), 0
movd 0, %mm(0)

movd %xmm(0), %eax
movd %eax, %xmm(0)
movd %xmm(0), %rax
movd %rax, %xmm(0)
movd %xmm(0), 0
movd 0, %xmm(0)

movq %xmm(0), 0
movq 0, %xmm(0)
movq %xmm(1), %xmm(0)
movq %xmm(0), %xmm(1)

movq %mm(0), 0
movq 0, %mm(0)
movq %mm(1), %mm(0)
movq %mm(0), %mm(1)

movq %xmm(0), %rax
movq %rax, %xmm(0)

movq %mm(0), %rax
movq %rax, %mm(0)

