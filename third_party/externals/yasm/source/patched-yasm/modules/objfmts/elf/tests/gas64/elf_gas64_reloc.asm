.comm _ZEROVAR, 32, 16
.comm _VAR, 16, 16
.data
.org 0
_ZEROVAR:
.org 0xa0
.globl _VAR
.type _VAR, @object
.size _VAR, 16
_VAR:
.4byte 0
.4byte 0
.4byte 0
.4byte 0
.org 0xc0
_VAR2:
.org 0xe0
.globl _VAR3
_VAR3:

.text
movq $0, %rax
movq _VAR, %rax
movq %rax, _VAR(%rip)
movq _VAR+8(%rip), %rcx
movlpd _VAR(%rip), %xmm1

movq _VAR2, %rax
movq %rax, _VAR2(%rip)
movq _VAR2+8(%rip), %rcx
movlpd _VAR2(%rip), %xmm1

movq _VAR3, %rax
movq %rax, _VAR3(%rip)
movq _VAR3+8(%rip), %rcx
movlpd _VAR3(%rip), %xmm1

