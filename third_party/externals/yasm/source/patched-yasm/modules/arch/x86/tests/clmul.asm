[bits 64]

pclmulqdq xmm1, xmm2, 5
pclmulqdq xmm1, [rax], byte 5
pclmulqdq xmm1, dqword [rax], 5
vpclmulqdq xmm1, xmm2, 0x10
vpclmulqdq xmm1, dqword [rbx], 0x10
vpclmulqdq xmm0, xmm1, xmm2, 0x10
vpclmulqdq xmm0, xmm1, dqword [rbx], 0x10

pclmullqlqdq xmm1, xmm2
pclmullqlqdq xmm1, [rax]
pclmullqlqdq xmm1, dqword [rax]
vpclmullqlqdq xmm1, xmm2
vpclmullqlqdq xmm1, dqword[rbx]
vpclmullqlqdq xmm0, xmm1, xmm2
vpclmullqlqdq xmm0, xmm1, dqword[rbx]

pclmulhqlqdq xmm1, xmm2
pclmulhqlqdq xmm1, [rax]
pclmulhqlqdq xmm1, dqword [rax]
vpclmulhqlqdq xmm1, xmm2
vpclmulhqlqdq xmm1, dqword[rbx]
vpclmulhqlqdq xmm0, xmm1, xmm2
vpclmulhqlqdq xmm0, xmm1, dqword[rbx]

pclmullqhqdq xmm1, xmm2
pclmullqhqdq xmm1, [rax]
pclmullqhqdq xmm1, dqword [rax]
vpclmullqhqdq xmm1, xmm2
vpclmullqhqdq xmm1, dqword[rbx]
vpclmullqhqdq xmm0, xmm1, xmm2
vpclmullqhqdq xmm0, xmm1, dqword[rbx]

pclmulhqhqdq xmm1, xmm2
pclmulhqhqdq xmm1, [rax]
pclmulhqhqdq xmm1, dqword [rax]
vpclmulhqhqdq xmm1, xmm2
vpclmulhqhqdq xmm1, dqword[rbx]
vpclmulhqhqdq xmm0, xmm1, xmm2
vpclmulhqhqdq xmm0, xmm1, dqword[rbx]

