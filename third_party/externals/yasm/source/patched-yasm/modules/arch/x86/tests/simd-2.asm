[bits 32]
pextrw ebx, mm5, 0		; 0F C5 DD 00
pextrw ecx, xmm0, 1		; 66 0F C5 C8 01

pinsrw mm3, esi, 5		; 0F C4 DE 05
pinsrw mm3, [0], 4		; 0F C4 1D 00 00 00 00 04

pinsrw xmm1, eax, 3		; 66 0F C4 C8 03
pinsrw xmm1, [0], 2		; 66 0F C4 0D 00 00 00 00 02

movmskpd edx, xmm7		; 66 0F 50 D7
movmskps eax, xmm1		; 0F 50 C1

pmovmskb edi, mm0		; 0F D7 F8
pmovmskb esi, xmm1		; 66 0F D7 F1

cvtdq2pd xmm5, xmm4		; F3 0F E6 EC
cvtdq2pd xmm3, [0]		; F3 0F E6 1D 00 00 00 00
cvtdq2pd xmm2, qword [0]	; F3 0F E6 15 00 00 00 00

cvtdq2ps xmm1, xmm2		; 0F 5B CA
cvtdq2ps xmm4, [0]		; 0F 5B 25 00 00 00 00
cvtdq2ps xmm5, dqword [0]	; 0F 5B 2D 00 00 00 00

cvtpd2dq xmm0, xmm1		; F2 0F E6 C1
cvtpd2dq xmm2, [0]		; F2 0F E6 15 00 00 00 00
cvtpd2dq xmm3, dqword [0]	; F2 0F E6 1D 00 00 00 00

cvtpd2pi mm4, xmm5		; 66 0F 2D E5
cvtpd2pi mm6, [0]		; 66 0F 2D 35 00 00 00 00
cvtpd2pi mm7, dqword [0]	; 66 0F 2D 3D 00 00 00 00

cvtpd2ps xmm1, xmm2		; 66 0F 5A CA
cvtpd2ps xmm3, [0]		; 66 0F 5A 1D 00 00 00 00
cvtpd2ps xmm4, dqword [0]	; 66 0F 5A 25 00 00 00 00

cvtpi2pd xmm5, mm6		; 66 0F 2A EE
cvtpi2pd xmm7, [0]		; 66 0F 2A 3D 00 00 00 00
cvtpi2pd xmm0, qword [0]	; 66 0F 2A 05 00 00 00 00

cvtpi2ps xmm2, mm3		; 0F 2A D3
cvtpi2ps xmm4, [0]		; 0F 2A 25 00 00 00 00
cvtpi2ps xmm5, qword [0]	; 0F 2A 2D 00 00 00 00

cvtps2dq xmm6, xmm7		; 66 0F 5B F7
cvtps2dq xmm0, [0]		; 66 0F 5B 05 00 00 00 00
cvtps2dq xmm1, dqword [0]	; 66 0F 5B 0D 00 00 00 00

cvtps2pd xmm2, xmm3		; 0F 5A D3
cvtps2pd xmm4, [0]		; 0F 5A 25 00 00 00 00
cvtps2pd xmm5, qword [0]	; 0F 5A 2D 00 00 00 00

cvtps2pi mm6, xmm7		; 0F 2D F7
cvtps2pi mm0, [0]		; 0F 2D 05 00 00 00 00
cvtps2pi mm1, qword [0]		; 0F 2D 0D 00 00 00 00

cvtsd2si edx, xmm0		; F2 0F 2D D0
cvtsd2si eax, [0]		; F2 0F 2D 05 00 00 00 00
cvtsd2si ebx, qword [0]		; F2 0F 2D 1D 00 00 00 00

cvtsd2ss xmm1, xmm2		; F2 0F 5A CA
cvtsd2ss xmm3, [0]		; F2 0F 5A 1D 00 00 00 00
cvtsd2ss xmm4, qword [0]	; F2 0F 5A 25 00 00 00 00

cvtsi2sd xmm5, eax		; F2 0F 2A E8
cvtsi2sd xmm6, [0]		; F2 0F 2A 35 00 00 00 00
cvtsi2sd xmm7, dword [0]	; F2 0F 2A 3D 00 00 00 00

cvtsi2ss xmm0, edx		; F3 0F 2A C2
cvtsi2ss xmm1, [0]		; F3 0F 2A 0D 00 00 00 00
cvtsi2ss xmm2, dword [0]	; F3 0F 2A 15 00 00 00 00

cvtss2sd xmm3, xmm4		; F3 0F 5A DC
cvtss2sd xmm5, [0]		; F3 0F 5A 2D 00 00 00 00
cvtss2sd xmm6, dword [0]	; F3 0F 5A 35 00 00 00 00

cvtss2si ebx, xmm7		; F3 0F 2D DF
cvtss2si ecx, [0]		; F3 0F 2D 0D 00 00 00 00
cvtss2si eax, dword [0]		; F3 0F 2D 05 00 00 00 00

cvttpd2pi mm0, xmm1		; 66 0F 2C C1
cvttpd2pi mm2, [0]		; 66 0F 2C 15 00 00 00 00
cvttpd2pi mm3, dqword [0]	; 66 0F 2C 1D 00 00 00 00

cvttpd2dq xmm4, xmm5		; 66 0F E6 E5
cvttpd2dq xmm6, [0]		; 66 0F E6 35 00 00 00 00
cvttpd2dq xmm7, dqword [0]	; 66 0F E6 3D 00 00 00 00

cvttps2dq xmm0, xmm1		; F3 0F 5B C1
cvttps2dq xmm2, [0]		; F3 0F 5B 15 00 00 00 00
cvttps2dq xmm3, dqword [0]	; F3 0F 5B 1D 00 00 00 00

cvttps2pi mm4, xmm5		; 0F 2C E5
cvttps2pi mm6, [0]		; 0F 2C 35 00 00 00 00
cvttps2pi mm7, qword [0]	; 0F 2C 3D 00 00 00 00

cvttsd2si ecx, xmm0		; F2 0F 2C C8
cvttsd2si ebx, [0]		; F2 0F 2C 1D 00 00 00 00
cvttsd2si edi, qword [0]	; F2 0F 2C 3D 00 00 00 00

cvttss2si esi, xmm3		; F3 0F 2C F3
cvttss2si ebp, [0]		; F3 0F 2C 2D 00 00 00 00
cvttss2si eax, dword [0]	; F3 0F 2C 05 00 00 00 00

