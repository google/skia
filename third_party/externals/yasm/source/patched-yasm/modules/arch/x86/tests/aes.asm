[bits 64]
aesenc xmm1, xmm2
aesenc xmm1, [rax]
aesenc xmm1, dqword [rax]
aesenc xmm10, xmm12
aesenc xmm10, [rax+r15*4]
aesenc xmm10, [r14+r15*4]
vaesenc xmm1, xmm2
vaesenc xmm1, [rax]
vaesenc xmm1, dqword [rax]
vaesenc xmm1, xmm2, xmm3
vaesenc xmm1, xmm2, [rax]
vaesenc xmm1, xmm2, dqword [rax]

aesenclast xmm1, xmm2
aesenclast xmm1, [rax]
aesenclast xmm1, dqword [rax]
vaesenclast xmm1, xmm2
vaesenclast xmm1, [rax]
vaesenclast xmm1, dqword [rax]
vaesenclast xmm1, xmm2, xmm3
vaesenclast xmm1, xmm2, [rax]
vaesenclast xmm1, xmm2, dqword [rax]

aesdec xmm1, xmm2
aesdec xmm1, [rax]
aesdec xmm1, dqword [rax]
vaesdec xmm1, xmm2
vaesdec xmm1, [rax]
vaesdec xmm1, dqword [rax]
vaesdec xmm1, xmm2, xmm3
vaesdec xmm1, xmm2, [rax]
vaesdec xmm1, xmm2, dqword [rax]

aesdeclast xmm1, xmm2
aesdeclast xmm1, [rax]
aesdeclast xmm1, dqword [rax]
vaesdeclast xmm1, xmm2
vaesdeclast xmm1, [rax]
vaesdeclast xmm1, dqword [rax]
vaesdeclast xmm1, xmm2, xmm3
vaesdeclast xmm1, xmm2, [rax]
vaesdeclast xmm1, xmm2, dqword [rax]

aesimc xmm1, xmm2
aesimc xmm1, [rax]
aesimc xmm1, dqword [rax]
vaesimc xmm1, xmm2
vaesimc xmm1, [rax]
vaesimc xmm1, dqword [rax]
; no 3-operand form

aeskeygenassist xmm1, xmm2, 5
aeskeygenassist xmm1, [rax], byte 5
aeskeygenassist xmm1, dqword [rax], 5
vaeskeygenassist xmm1, xmm2, 5
vaeskeygenassist xmm1, [rax], byte 5
vaeskeygenassist xmm1, dqword [rax], 5

pclmulqdq xmm1, xmm2, 5
pclmulqdq xmm1, [rax], byte 5
pclmulqdq xmm1, dqword [rax], 5

; pclmulqdq variants
pclmullqlqdq xmm1, xmm2
pclmullqlqdq xmm1, [rax]
pclmullqlqdq xmm1, dqword [rax]

pclmulhqlqdq xmm1, xmm2
pclmulhqlqdq xmm1, [rax]
pclmulhqlqdq xmm1, dqword [rax]

pclmullqhqdq xmm1, xmm2
pclmullqhqdq xmm1, [rax]
pclmullqhqdq xmm1, dqword [rax]

pclmulhqhqdq xmm1, xmm2
pclmulhqhqdq xmm1, [rax]
pclmulhqhqdq xmm1, dqword [rax]

