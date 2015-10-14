movsl
movsq

smovb
smovw
smovl
smovq

scasl
scasq

sscab
sscaw
sscal
sscaq

lgdtq 0
lidtq 0
lldtw 0

ltrw 0

sgdtq 0
sidtq 0
sldtw %ax
sldtl %eax
sldtq %rax
smsww %ax
smswl %eax
smswq %rax

cvtsi2ssl %eax, %xmm0
cvtsi2ssq %rax, %xmm0
cvtss2sil %xmm0, %eax
cvtss2siq %xmm0, %rax
cvttss2sil %xmm0, %eax
cvttss2siq %xmm0, %rax

movmskpsl %xmm0, %eax
movmskpsq %xmm0, %rax

pextrwl $5, %mm0, %eax
pextrwq $10, %mm1, %rax
pextrwl $5, %xmm0, %eax
pextrwq $10, %xmm1, %rax

pinsrwl $5, %eax, %mm0
pinsrwq $10, %rax, %mm1
pinsrwl $5, %eax, %xmm0
pinsrwq $10, %rax, %xmm1

pmovmskbl %mm0, %eax
pmovmskbq %mm0, %rax
pmovmskbl %xmm0, %eax
pmovmskbq %xmm0, %rax

cvtsi2sdl %eax, %xmm0
cvtsi2sdq %rax, %xmm0

cvttsd2sil %xmm0, %eax
cvttsd2siq %xmm0, %rax

fistps 0
fistpl 0
fistpq 0
fistpll 0
