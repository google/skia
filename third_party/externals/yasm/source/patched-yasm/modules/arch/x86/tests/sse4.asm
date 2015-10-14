[bits 64]
blendpd xmm1, xmm2, 5
blendpd xmm1, [0], 5

blendps xmm1, xmm2, 5
blendps xmm1, [0], 5

blendvpd xmm1, xmm2
blendvpd xmm1, xmm2, xmm0
blendvpd xmm1, [0]
blendvpd xmm1, [0], xmm0

blendvps xmm1, xmm2
blendvps xmm1, xmm2, xmm0
blendvps xmm1, [0]
blendvps xmm1, [0], xmm0

crc32 eax, bl
crc32 eax, bh
crc32 eax, r9b
crc32 eax, byte [0]
crc32 eax, bx
crc32 eax, word [0]
crc32 eax, ebx
crc32 eax, dword [0]

crc32 r8d, bl
;crc32 r8d, bh			; error
crc32 r8d, r9b
crc32 r8d, byte [0]
crc32 r8d, bx
crc32 r8d, word [0]
crc32 r8d, ebx
crc32 r8d, dword [0]

crc32 rax, bl
;crc32 rax, bh			; error
crc32 rax, r9b
crc32 rax, byte [0]
crc32 rax, rbx
crc32 rax, qword [0]

dppd xmm1, xmm2, 5
dppd xmm1, [0], 5

dpps xmm1, xmm2, 5
dpps xmm1, [0], 5

extractps eax, xmm1, 5
extractps [0], xmm1, 5
extractps dword [0], xmm1, 5
extractps r8d, xmm1, 5
extractps rax, xmm1, 5

insertps xmm1, xmm2, 5
insertps xmm1, [0], 5
insertps xmm1, dword [0], 5

movntdqa xmm1, [0]
movntdqa xmm1, dqword [0]

mpsadbw xmm1, xmm2, 5
mpsadbw xmm1, [0], 5

packusdw xmm1, xmm2
packusdw xmm1, [0]

pblendvb xmm1, xmm2, xmm0
pblendvb xmm1, [0], xmm0
pblendvb xmm1, xmm2
pblendvb xmm1, [0]

pblendw xmm1, xmm2, 5
pblendw xmm1, [0], 5

pcmpeqq xmm1, xmm2
pcmpeqq xmm1, [0]

pcmpestri xmm1, xmm2, 5
pcmpestri xmm1, [0], 5

pcmpestrm xmm1, xmm2, 5
pcmpestrm xmm1, [0], 5

pcmpistri xmm1, xmm2, 5
pcmpistri xmm1, [0], 5

pcmpistrm xmm1, xmm2, 5
pcmpistrm xmm1, [0], 5

pcmpgtq xmm1, xmm2
pcmpgtq xmm1, [0]

pextrb eax, xmm1, 5
pextrb rax, xmm1, 5
pextrb [0], xmm1, 5
pextrb byte [0], xmm1, 5

pextrd eax, xmm1, 5
pextrd [0], xmm1, 5
pextrd dword [0], xmm1, 5
pextrq rax, xmm1, 5
pextrq qword [0], xmm1, 5

; To get the SSE4 versions we need to disable the SSE2 versions
cpu nosse2
pextrw eax, xmm1, 5
pextrw [0], xmm1, 5
pextrw word [0], xmm1, 5
pextrw rax, xmm1, 5

phminposuw xmm1, xmm2
phminposuw xmm1, [0]

pinsrb xmm1, eax, 5
pinsrb xmm1, [0], 5
pinsrb xmm1, byte [0], 5

pinsrd xmm1, eax, 5
pinsrd xmm1, [0], 5
pinsrd xmm1, dword [0], 5

pinsrq xmm1, rax, 5
pinsrq xmm1, [0], 5
pinsrq xmm1, qword [0], 5

pmaxsb xmm1, xmm2
pmaxsb xmm1, [0]

pmaxsd xmm1, xmm2
pmaxsd xmm1, [0]

pmaxud xmm1, xmm2
pmaxud xmm1, [0]

pmaxuw xmm1, xmm2
pmaxuw xmm1, [0]

pminsb xmm1, xmm2
pminsb xmm1, [0]

pminsd xmm1, xmm2
pminsd xmm1, [0]

pminud xmm1, xmm2
pminud xmm1, [0]

pminuw xmm1, xmm2
pminuw xmm1, [0]

pmovsxbw xmm1, xmm2
pmovsxbw xmm1, [0]
pmovsxbw xmm1, qword [0]

pmovsxbd xmm1, xmm2
pmovsxbd xmm1, [0]
pmovsxbd xmm1, dword [0]

pmovsxbq xmm1, xmm2
pmovsxbq xmm1, [0]
pmovsxbq xmm1, word [0]

pmovsxwd xmm1, xmm2
pmovsxwd xmm1, [0]
pmovsxwd xmm1, qword [0]

pmovsxwq xmm1, xmm2
pmovsxwq xmm1, [0]
pmovsxwq xmm1, dword [0]

pmovsxdq xmm1, xmm2
pmovsxdq xmm1, [0]
pmovsxdq xmm1, qword [0]

pmovzxbw xmm1, xmm2
pmovzxbw xmm1, [0]
pmovzxbw xmm1, qword [0]

pmovzxbd xmm1, xmm2
pmovzxbd xmm1, [0]
pmovzxbd xmm1, dword [0]

pmovzxbq xmm1, xmm2
pmovzxbq xmm1, [0]
pmovzxbq xmm1, word [0]

pmovzxwd xmm1, xmm2
pmovzxwd xmm1, [0]
pmovzxwd xmm1, qword [0]

pmovzxwq xmm1, xmm2
pmovzxwq xmm1, [0]
pmovzxwq xmm1, dword [0]

pmovzxdq xmm1, xmm2
pmovzxdq xmm1, [0]
pmovzxdq xmm1, qword [0]

pmuldq xmm1, xmm2
pmuldq xmm1, [0]

pmulld xmm1, xmm2
pmulld xmm1, [0]

popcnt ax, bx
popcnt ax, [0]
popcnt ebx, ecx
popcnt ebx, [0]
popcnt rcx, rdx
popcnt rcx, [0]

ptest xmm1, xmm2
ptest xmm1, [0]

roundpd xmm1, xmm2, 5
roundpd xmm1, [0], 5

roundps xmm1, xmm2, 5
roundps xmm1, [0], 5

roundsd xmm1, xmm2, 5
roundsd xmm1, [0], 5

roundss xmm1, xmm2, 5
roundss xmm1, [0], 5

