; Exhaustive test of AVX instructions
; Also includes based-upon SSE instructions for comparison
;
;  Copyright (C) 2008  Peter Johnson
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
; 1. Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.
;

[bits 64]
addpd xmm1, xmm2
addpd xmm1, [rax]
addpd xmm1, dqword [rax]
addpd xmm10, xmm12
addpd xmm10, [rax+r15*4]
addpd xmm10, [r14+r15*4]

vaddpd xmm1, xmm2
vaddpd xmm1, [rax]
vaddpd xmm1, dqword [rax]
vaddpd xmm10, xmm12
vaddpd xmm10, [rax+r15*4]
vaddpd xmm10, [r14+r15*4]

vaddpd xmm1, xmm2, xmm3
vaddpd xmm1, xmm2, [rax]
vaddpd xmm1, xmm2, dqword [rax]
vaddpd xmm10, xmm12, xmm13
vaddpd xmm10, xmm12, [rax+r15*4]
vaddpd xmm10, xmm12, [r14+r15*4]

vaddpd ymm1, ymm2, ymm3
vaddpd ymm1, ymm2, [rax]
vaddpd ymm1, ymm2, yword [rax]
vaddpd ymm10, ymm12, ymm13
vaddpd ymm10, ymm12, [rax+r15*4]
vaddpd ymm10, ymm12, [r14+r15*4]

; Further instructions won't test high 8 registers (validated above)
addps xmm1, xmm2
addps xmm1, [rax]
addps xmm1, dqword [rax]
vaddps xmm1, xmm2
vaddps xmm1, [rax]
vaddps xmm1, dqword [rax]
vaddps xmm1, xmm2, xmm3
vaddps xmm1, xmm2, [rax]
vaddps xmm1, xmm2, dqword [rax]
vaddps ymm1, ymm2, ymm3
vaddps ymm1, ymm2, [rax]
vaddps ymm1, ymm2, yword [rax]

addsd xmm1, xmm2
addsd xmm1, [rax]
addsd xmm1, qword [rax]
vaddsd xmm1, xmm2
vaddsd xmm1, [rax]
vaddsd xmm1, qword [rax]
vaddsd xmm1, xmm2, xmm3
vaddsd xmm1, xmm2, [rax]
vaddsd xmm1, xmm2, qword [rax]

addss xmm1, xmm2
addss xmm1, [rax]
addss xmm1, dword [rax]
vaddss xmm1, xmm2
vaddss xmm1, [rax]
vaddss xmm1, dword [rax]
vaddss xmm1, xmm2, xmm3
vaddss xmm1, xmm2, [rax]
vaddss xmm1, xmm2, dword [rax]

addsubpd xmm1, xmm2
addsubpd xmm1, [rax]
addsubpd xmm1, dqword [rax]
vaddsubpd xmm1, xmm2
vaddsubpd xmm1, [rax]
vaddsubpd xmm1, dqword [rax]
vaddsubpd xmm1, xmm2, xmm3
vaddsubpd xmm1, xmm2, [rax]
vaddsubpd xmm1, xmm2, dqword [rax]
vaddsubpd ymm1, ymm2, ymm3
vaddsubpd ymm1, ymm2, [rax]
vaddsubpd ymm1, ymm2, yword [rax]

addsubps xmm1, xmm2
addsubps xmm1, [rax]
addsubps xmm1, dqword [rax]
vaddsubps xmm1, xmm2
vaddsubps xmm1, [rax]
vaddsubps xmm1, dqword [rax]
vaddsubps xmm1, xmm2, xmm3
vaddsubps xmm1, xmm2, [rax]
vaddsubps xmm1, xmm2, dqword [rax]
vaddsubps ymm1, ymm2, ymm3
vaddsubps ymm1, ymm2, [rax]
vaddsubps ymm1, ymm2, yword [rax]

andpd xmm1, xmm2
andpd xmm1, [rax]
andpd xmm1, dqword [rax]
vandpd xmm1, xmm2
vandpd xmm1, [rax]
vandpd xmm1, dqword [rax]
vandpd xmm1, xmm2, xmm3
vandpd xmm1, xmm2, [rax]
vandpd xmm1, xmm2, dqword [rax]
vandpd ymm1, ymm2, ymm3
vandpd ymm1, ymm2, [rax]
vandpd ymm1, ymm2, yword [rax]

andps xmm1, xmm2
andps xmm1, [rax]
andps xmm1, dqword [rax]
vandps xmm1, xmm2
vandps xmm1, [rax]
vandps xmm1, dqword [rax]
vandps xmm1, xmm2, xmm3
vandps xmm1, xmm2, [rax]
vandps xmm1, xmm2, dqword [rax]
vandps ymm1, ymm2, ymm3
vandps ymm1, ymm2, [rax]
vandps ymm1, ymm2, yword [rax]

andnpd xmm1, xmm2
andnpd xmm1, [rax]
andnpd xmm1, dqword [rax]
vandnpd xmm1, xmm2
vandnpd xmm1, [rax]
vandnpd xmm1, dqword [rax]
vandnpd xmm1, xmm2, xmm3
vandnpd xmm1, xmm2, [rax]
vandnpd xmm1, xmm2, dqword [rax]
vandnpd ymm1, ymm2, ymm3
vandnpd ymm1, ymm2, [rax]
vandnpd ymm1, ymm2, yword [rax]

andnps xmm1, xmm2
andnps xmm1, [rax]
andnps xmm1, dqword [rax]
vandnps xmm1, xmm2
vandnps xmm1, [rax]
vandnps xmm1, dqword [rax]
vandnps xmm1, xmm2, xmm3
vandnps xmm1, xmm2, [rax]
vandnps xmm1, xmm2, dqword [rax]
vandnps ymm1, ymm2, ymm3
vandnps ymm1, ymm2, [rax]
vandnps ymm1, ymm2, yword [rax]

blendpd xmm1, xmm2, 5
blendpd xmm1, [rax], byte 5
blendpd xmm1, dqword [rax], 5
vblendpd xmm1, xmm2, 5
vblendpd xmm1, [rax], byte 5
vblendpd xmm1, dqword [rax], 5
vblendpd xmm1, xmm2, xmm3, 5
vblendpd xmm1, xmm2, [rax], byte 5
vblendpd xmm1, xmm2, dqword [rax], 5
vblendpd ymm1, ymm2, ymm3, 5
vblendpd ymm1, ymm2, [rax], byte 5
vblendpd ymm1, ymm2, yword [rax], 5

blendps xmm1, xmm2, 5
blendps xmm1, [rax], byte 5
blendps xmm1, dqword [rax], 5
vblendps xmm1, xmm2, 5
vblendps xmm1, [rax], byte 5
vblendps xmm1, dqword [rax], 5
vblendps xmm1, xmm2, xmm3, 5
vblendps xmm1, xmm2, [rax], byte 5
vblendps xmm1, xmm2, dqword [rax], 5
vblendps ymm1, ymm2, ymm3, 5
vblendps ymm1, ymm2, [rax], byte 5
vblendps ymm1, ymm2, yword [rax], 5

; blendvpd doesn't have vex-encoded version of implicit xmm0
blendvpd xmm1, xmm3
blendvpd xmm1, [rax]
blendvpd xmm1, dqword [rax]
blendvpd xmm1, xmm3, xmm0
blendvpd xmm1, [rax], xmm0
blendvpd xmm1, dqword [rax], xmm0
vblendvpd xmm1, xmm2, xmm3, xmm4
vblendvpd xmm1, xmm2, [rax], xmm4
vblendvpd xmm1, xmm2, dqword [rax], xmm4
vblendvpd ymm1, ymm2, ymm3, ymm4
vblendvpd ymm1, ymm2, [rax], ymm4
vblendvpd ymm1, ymm2, yword [rax], ymm4

; blendvps doesn't have vex-encoded version of implicit xmm0
blendvps xmm1, xmm3
blendvps xmm1, [rax]
blendvps xmm1, dqword [rax]
blendvps xmm1, xmm3, xmm0
blendvps xmm1, [rax], xmm0
blendvps xmm1, dqword [rax], xmm0
vblendvps xmm1, xmm2, xmm3, xmm4
vblendvps xmm1, xmm2, [rax], xmm4
vblendvps xmm1, xmm2, dqword [rax], xmm4
vblendvps ymm1, ymm2, ymm3, ymm4
vblendvps ymm1, ymm2, [rax], ymm4
vblendvps ymm1, ymm2, yword [rax], ymm4

vbroadcastss xmm1, [rax]
vbroadcastss xmm1, dword [rax]
vbroadcastss ymm1, [rax]
vbroadcastss ymm1, dword [rax]

vbroadcastsd ymm1, [rax]
vbroadcastsd ymm1, qword [rax]

vbroadcastf128 ymm1, [rax]
vbroadcastf128 ymm1, dqword [rax]

cmppd xmm1, xmm2, 5
cmppd xmm1, [rax], byte 5
cmppd xmm1, dqword [rax], 5
vcmppd xmm1, xmm2, 5
vcmppd xmm1, [rax], byte 5
vcmppd xmm1, dqword [rax], 5
vcmppd xmm1, xmm2, xmm3, 5
vcmppd xmm1, xmm2, [rax], byte 5
vcmppd xmm1, xmm2, dqword [rax], 5
vcmppd ymm1, ymm2, ymm3, 5
vcmppd ymm1, ymm2, [rax], byte 5
vcmppd ymm1, ymm2, yword [rax], 5

cmpps xmm1, xmm2, 5
cmpps xmm1, [rax], byte 5
cmpps xmm1, dqword [rax], 5
vcmpps xmm1, xmm2, 5
vcmpps xmm1, [rax], byte 5
vcmpps xmm1, dqword [rax], 5
vcmpps xmm1, xmm2, xmm3, 5
vcmpps xmm1, xmm2, [rax], byte 5
vcmpps xmm1, xmm2, dqword [rax], 5
vcmpps ymm1, ymm2, ymm3, 5
vcmpps ymm1, ymm2, [rax], byte 5
vcmpps ymm1, ymm2, yword [rax], 5

cmpsd xmm1, xmm2, 5
cmpsd xmm1, [rax], byte 5
cmpsd xmm1, qword [rax], 5
vcmpsd xmm1, xmm2, 5
vcmpsd xmm1, [rax], byte 5
vcmpsd xmm1, qword [rax], 5
vcmpsd xmm1, xmm2, xmm3, 5
vcmpsd xmm1, xmm2, [rax], byte 5
vcmpsd xmm1, xmm2, qword [rax], 5

cmpss xmm1, xmm2, 5
cmpss xmm1, [rax], byte 5
cmpss xmm1, dword [rax], 5
vcmpss xmm1, xmm2, 5
vcmpss xmm1, [rax], byte 5
vcmpss xmm1, dword [rax], 5
vcmpss xmm1, xmm2, xmm3, 5
vcmpss xmm1, xmm2, [rax], byte 5
vcmpss xmm1, xmm2, dword [rax], 5

comisd xmm1, xmm2
comisd xmm1, [rax]
comisd xmm1, qword [rax]
vcomisd xmm1, xmm2
vcomisd xmm1, [rax]
vcomisd xmm1, qword [rax]

comiss xmm1, xmm2
comiss xmm1, [rax]
comiss xmm1, dword [rax]
vcomiss xmm1, xmm2
vcomiss xmm1, [rax]
vcomiss xmm1, dword [rax]

cvtdq2pd xmm1, xmm2
cvtdq2pd xmm1, [rax]
cvtdq2pd xmm1, qword [rax]
vcvtdq2pd xmm1, xmm2
vcvtdq2pd xmm1, [rax]
vcvtdq2pd xmm1, qword [rax]
vcvtdq2pd ymm1, xmm2
vcvtdq2pd ymm1, [rax]
vcvtdq2pd ymm1, dqword [rax]

cvtdq2ps xmm1, xmm2
cvtdq2ps xmm1, [rax]
cvtdq2ps xmm1, dqword [rax]
vcvtdq2ps xmm1, xmm2
vcvtdq2ps xmm1, [rax]
vcvtdq2ps xmm1, dqword [rax]
vcvtdq2ps ymm1, ymm2
vcvtdq2ps ymm1, [rax]
vcvtdq2ps ymm1, yword [rax]

; These require memory operand size to be specified (in AVX version)
cvtpd2dq xmm1, xmm2
cvtpd2dq xmm1, [rax]
cvtpd2dq xmm1, dqword [rax]
vcvtpd2dq xmm1, xmm2
vcvtpd2dq xmm1, dqword [rax]
vcvtpd2dq xmm1, ymm2
vcvtpd2dq xmm1, yword [rax]

cvtpd2ps xmm1, xmm2
cvtpd2ps xmm1, [rax]
cvtpd2ps xmm1, dqword [rax]
vcvtpd2ps xmm1, xmm2
vcvtpd2ps xmm1, dqword [rax]
vcvtpd2ps xmm1, ymm2
vcvtpd2ps xmm1, yword [rax]

cvtps2dq xmm1, xmm2
cvtps2dq xmm1, [rax]
cvtps2dq xmm1, dqword [rax]
vcvtps2dq xmm1, xmm2
vcvtps2dq xmm1, [rax]
vcvtps2dq xmm1, dqword [rax]
vcvtps2dq ymm1, ymm2
vcvtps2dq ymm1, [rax]
vcvtps2dq ymm1, yword [rax]

cvtps2pd xmm1, xmm2
cvtps2pd xmm1, [rax]
cvtps2pd xmm1, qword [rax]
vcvtps2pd xmm1, xmm2
vcvtps2pd xmm1, [rax]
vcvtps2pd xmm1, qword [rax]
vcvtps2pd ymm1, xmm2
vcvtps2pd ymm1, [rax]
vcvtps2pd ymm1, dqword [rax]

cvtsd2si eax, xmm2
cvtsd2si eax, [rax]
cvtsd2si eax, qword [rax]
vcvtsd2si eax, xmm2
vcvtsd2si eax, [rax]
vcvtsd2si eax, qword [rax]
cvtsd2si rax, xmm2
cvtsd2si rax, [rax]
cvtsd2si rax, qword [rax]
vcvtsd2si rax, xmm2
vcvtsd2si rax, [rax]
vcvtsd2si rax, qword [rax]

cvtsd2ss xmm1, xmm2
cvtsd2ss xmm1, [rax]
cvtsd2ss xmm1, qword [rax]
vcvtsd2ss xmm1, xmm2
vcvtsd2ss xmm1, [rax]
vcvtsd2ss xmm1, qword [rax]
vcvtsd2ss xmm1, xmm2, xmm3
vcvtsd2ss xmm1, xmm2, [rax]
vcvtsd2ss xmm1, xmm2, qword [rax]

; unsized not valid
cvtsi2sd xmm1, eax
cvtsi2sd xmm1, dword [rax]
vcvtsi2sd xmm1, eax
vcvtsi2sd xmm1, dword [rax]
vcvtsi2sd xmm1, xmm2, eax
vcvtsi2sd xmm1, xmm2, dword [rax]
cvtsi2sd xmm1, rax
cvtsi2sd xmm1, qword [rax]
vcvtsi2sd xmm1, rax
vcvtsi2sd xmm1, qword [rax]
vcvtsi2sd xmm1, xmm2, rax
vcvtsi2sd xmm1, xmm2, qword [rax]

cvtsi2ss xmm1, eax
cvtsi2ss xmm1, dword [rax]
vcvtsi2ss xmm1, eax
vcvtsi2ss xmm1, dword [rax]
vcvtsi2ss xmm1, xmm2, eax
vcvtsi2ss xmm1, xmm2, dword [rax]
cvtsi2ss xmm1, rax
cvtsi2ss xmm1, qword [rax]
vcvtsi2ss xmm1, rax
vcvtsi2ss xmm1, qword [rax]
vcvtsi2ss xmm1, xmm2, rax
vcvtsi2ss xmm1, xmm2, qword [rax]

cvtss2sd xmm1, xmm2
cvtss2sd xmm1, [rax]
cvtss2sd xmm1, dword [rax]
vcvtss2sd xmm1, xmm2
vcvtss2sd xmm1, [rax]
vcvtss2sd xmm1, dword [rax]
vcvtss2sd xmm1, xmm2, xmm3
vcvtss2sd xmm1, xmm2, [rax]
vcvtss2sd xmm1, xmm2, dword [rax]

cvtss2si eax, xmm2
cvtss2si eax, [rax]
cvtss2si eax, dword [rax]
vcvtss2si eax, xmm2
vcvtss2si eax, [rax]
vcvtss2si eax, dword [rax]
cvtss2si rax, xmm2
cvtss2si rax, [rax]
cvtss2si rax, dword [rax]
vcvtss2si rax, xmm2
vcvtss2si rax, [rax]
vcvtss2si rax, dword [rax]

; These require memory operand size to be specified (in AVX version)
cvttpd2dq xmm1, xmm2
cvttpd2dq xmm1, [rax]
cvttpd2dq xmm1, dqword [rax]
vcvttpd2dq xmm1, xmm2
vcvttpd2dq xmm1, dqword [rax]
vcvttpd2dq xmm1, ymm2
vcvttpd2dq xmm1, yword [rax]

cvttps2dq xmm1, xmm2
cvttps2dq xmm1, [rax]
cvttps2dq xmm1, dqword [rax]
vcvttps2dq xmm1, xmm2
vcvttps2dq xmm1, [rax]
vcvttps2dq xmm1, dqword [rax]
vcvttps2dq ymm1, ymm2
vcvttps2dq ymm1, [rax]
vcvttps2dq ymm1, yword [rax]

cvttsd2si eax, xmm2
cvttsd2si eax, [rax]
cvttsd2si eax, qword [rax]
vcvttsd2si eax, xmm2
vcvttsd2si eax, [rax]
vcvttsd2si eax, qword [rax]
cvttsd2si rax, xmm2
cvttsd2si rax, [rax]
cvttsd2si rax, qword [rax]
vcvttsd2si rax, xmm2
vcvttsd2si rax, [rax]
vcvttsd2si rax, qword [rax]

cvttss2si eax, xmm2
cvttss2si eax, [rax]
cvttss2si eax, dword [rax]
vcvttss2si eax, xmm2
vcvttss2si eax, [rax]
vcvttss2si eax, dword [rax]
cvttss2si rax, xmm2
cvttss2si rax, [rax]
cvttss2si rax, dword [rax]
vcvttss2si rax, xmm2
vcvttss2si rax, [rax]
vcvttss2si rax, dword [rax]

divpd xmm1, xmm2
divpd xmm1, [rax]
divpd xmm1, dqword [rax]
vdivpd xmm1, xmm2
vdivpd xmm1, [rax]
vdivpd xmm1, dqword [rax]
vdivpd xmm1, xmm2, xmm3
vdivpd xmm1, xmm2, [rax]
vdivpd xmm1, xmm2, dqword [rax]
vdivpd ymm1, ymm2, ymm3
vdivpd ymm1, ymm2, [rax]
vdivpd ymm1, ymm2, yword [rax]

divps xmm1, xmm2
divps xmm1, [rax]
divps xmm1, dqword [rax]
vdivps xmm1, xmm2
vdivps xmm1, [rax]
vdivps xmm1, dqword [rax]
vdivps xmm1, xmm2, xmm3
vdivps xmm1, xmm2, [rax]
vdivps xmm1, xmm2, dqword [rax]
vdivps ymm1, ymm2, ymm3
vdivps ymm1, ymm2, [rax]
vdivps ymm1, ymm2, yword [rax]

divsd xmm1, xmm2
divsd xmm1, [rax]
divsd xmm1, qword [rax]
vdivsd xmm1, xmm2
vdivsd xmm1, [rax]
vdivsd xmm1, qword [rax]
vdivsd xmm1, xmm2, xmm3
vdivsd xmm1, xmm2, [rax]
vdivsd xmm1, xmm2, qword [rax]

divss xmm1, xmm2
divss xmm1, [rax]
divss xmm1, dword [rax]
vdivss xmm1, xmm2
vdivss xmm1, [rax]
vdivss xmm1, dword [rax]
vdivss xmm1, xmm2, xmm3
vdivss xmm1, xmm2, [rax]
vdivss xmm1, xmm2, dword [rax]

dppd xmm1, xmm2, 5
dppd xmm1, [rax], byte 5
dppd xmm1, dqword [rax], 5
vdppd xmm1, xmm2, 5
vdppd xmm1, [rax], byte 5
vdppd xmm1, dqword [rax], 5
vdppd xmm1, xmm2, xmm3, 5
vdppd xmm1, xmm2, [rax], byte 5
vdppd xmm1, xmm2, dqword [rax], 5
; no ymm version

dpps xmm1, xmm2, 5
dpps xmm1, [rax], byte 5
dpps xmm1, dqword [rax], 5
vdpps xmm1, xmm2, 5
vdpps xmm1, [rax], byte 5
vdpps xmm1, dqword [rax], 5
vdpps xmm1, xmm2, xmm3, 5
vdpps xmm1, xmm2, [rax], byte 5
vdpps xmm1, xmm2, dqword [rax], 5
vdpps ymm1, ymm2, ymm3, 5
vdpps ymm1, ymm2, [rax], byte 5
vdpps ymm1, ymm2, yword [rax], 5

vextractf128 xmm1, ymm2, 5
vextractf128 [rax], ymm2, byte 5
vextractf128 dqword [rax], ymm2, 5

extractps eax, xmm1, 5
extractps rax, xmm1, 5
extractps [rax], xmm1, byte 5
extractps dword [rax], xmm1, 5
vextractps eax, xmm1, 5
vextractps rax, xmm1, 5
vextractps [rax], xmm1, byte 5
vextractps dword [rax], xmm1, 5

haddpd xmm1, xmm2
haddpd xmm1, [rax]
haddpd xmm1, dqword [rax]
vhaddpd xmm1, xmm2
vhaddpd xmm1, [rax]
vhaddpd xmm1, dqword [rax]
vhaddpd xmm1, xmm2, xmm3
vhaddpd xmm1, xmm2, [rax]
vhaddpd xmm1, xmm2, dqword [rax]
vhaddpd ymm1, ymm2, ymm3
vhaddpd ymm1, ymm2, [rax]
vhaddpd ymm1, ymm2, yword [rax]

haddps xmm1, xmm2
haddps xmm1, [rax]
haddps xmm1, dqword [rax]
vhaddps xmm1, xmm2
vhaddps xmm1, [rax]
vhaddps xmm1, dqword [rax]
vhaddps xmm1, xmm2, xmm3
vhaddps xmm1, xmm2, [rax]
vhaddps xmm1, xmm2, dqword [rax]
vhaddps ymm1, ymm2, ymm3
vhaddps ymm1, ymm2, [rax]
vhaddps ymm1, ymm2, yword [rax]

hsubpd xmm1, xmm2
hsubpd xmm1, [rax]
hsubpd xmm1, dqword [rax]
vhsubpd xmm1, xmm2
vhsubpd xmm1, [rax]
vhsubpd xmm1, dqword [rax]
vhsubpd xmm1, xmm2, xmm3
vhsubpd xmm1, xmm2, [rax]
vhsubpd xmm1, xmm2, dqword [rax]
vhsubpd ymm1, ymm2, ymm3
vhsubpd ymm1, ymm2, [rax]
vhsubpd ymm1, ymm2, yword [rax]

hsubps xmm1, xmm2
hsubps xmm1, [rax]
hsubps xmm1, dqword [rax]
vhsubps xmm1, xmm2
vhsubps xmm1, [rax]
vhsubps xmm1, dqword [rax]
vhsubps xmm1, xmm2, xmm3
vhsubps xmm1, xmm2, [rax]
vhsubps xmm1, xmm2, dqword [rax]
vhsubps ymm1, ymm2, ymm3
vhsubps ymm1, ymm2, [rax]
vhsubps ymm1, ymm2, yword [rax]

vinsertf128 ymm1, ymm2, xmm3, 5
vinsertf128 ymm1, ymm2, [rax], byte 5
vinsertf128 ymm1, ymm2, dqword [rax], 5

insertps xmm1, xmm2, 5
insertps xmm1, [rax], byte 5
insertps xmm1, dword [rax], 5
vinsertps xmm1, xmm2, 5
vinsertps xmm1, [rax], byte 5
vinsertps xmm1, dword [rax], 5
vinsertps xmm1, xmm2, xmm3, 5
vinsertps xmm1, xmm2, [rax], byte 5
vinsertps xmm1, xmm2, dword [rax], 5

lddqu xmm1, [rax]
lddqu xmm1, dqword [rax]
vlddqu xmm1, [rax]
vlddqu xmm1, dqword [rax]
vlddqu ymm1, [rax]
vlddqu ymm1, yword [rax]

ldmxcsr [rax]
ldmxcsr dword [rax]
vldmxcsr [rax]
vldmxcsr dword [rax]

maskmovdqu xmm1, xmm2
vmaskmovdqu xmm1, xmm2

vmaskmovps xmm1, xmm2, [rax]
vmaskmovps xmm1, xmm2, dqword [rax]
vmaskmovps ymm1, ymm2, [rax]
vmaskmovps ymm1, ymm2, yword [rax]
vmaskmovps [rax], xmm2, xmm3
vmaskmovps dqword [rax], xmm2, xmm3
vmaskmovps [rax], ymm2, ymm3
vmaskmovps yword [rax], ymm2, ymm3

vmaskmovpd xmm1, xmm2, [rax]
vmaskmovpd xmm1, xmm2, dqword [rax]
vmaskmovpd ymm1, ymm2, [rax]
vmaskmovpd ymm1, ymm2, yword [rax]
vmaskmovpd [rax], xmm2, xmm3
vmaskmovpd dqword [rax], xmm2, xmm3
vmaskmovpd [rax], ymm2, ymm3
vmaskmovpd yword [rax], ymm2, ymm3

maxpd xmm1, xmm2
maxpd xmm1, [rax]
maxpd xmm1, dqword [rax]
vmaxpd xmm1, xmm2
vmaxpd xmm1, [rax]
vmaxpd xmm1, dqword [rax]
vmaxpd xmm1, xmm2, xmm3
vmaxpd xmm1, xmm2, [rax]
vmaxpd xmm1, xmm2, dqword [rax]
vmaxpd ymm1, ymm2, ymm3
vmaxpd ymm1, ymm2, [rax]
vmaxpd ymm1, ymm2, yword [rax]

maxps xmm1, xmm2
maxps xmm1, [rax]
maxps xmm1, dqword [rax]
vmaxps xmm1, xmm2
vmaxps xmm1, [rax]
vmaxps xmm1, dqword [rax]
vmaxps xmm1, xmm2, xmm3
vmaxps xmm1, xmm2, [rax]
vmaxps xmm1, xmm2, dqword [rax]
vmaxps ymm1, ymm2, ymm3
vmaxps ymm1, ymm2, [rax]
vmaxps ymm1, ymm2, yword [rax]

maxsd xmm1, xmm2
maxsd xmm1, [rax]
maxsd xmm1, qword [rax]
vmaxsd xmm1, xmm2
vmaxsd xmm1, [rax]
vmaxsd xmm1, qword [rax]
vmaxsd xmm1, xmm2, xmm3
vmaxsd xmm1, xmm2, [rax]
vmaxsd xmm1, xmm2, qword [rax]

maxss xmm1, xmm2
maxss xmm1, [rax]
maxss xmm1, dword [rax]
vmaxss xmm1, xmm2
vmaxss xmm1, [rax]
vmaxss xmm1, dword [rax]
vmaxss xmm1, xmm2, xmm3
vmaxss xmm1, xmm2, [rax]
vmaxss xmm1, xmm2, dword [rax]

minpd xmm1, xmm2
minpd xmm1, [rax]
minpd xmm1, dqword [rax]
vminpd xmm1, xmm2
vminpd xmm1, [rax]
vminpd xmm1, dqword [rax]
vminpd xmm1, xmm2, xmm3
vminpd xmm1, xmm2, [rax]
vminpd xmm1, xmm2, dqword [rax]
vminpd ymm1, ymm2, ymm3
vminpd ymm1, ymm2, [rax]
vminpd ymm1, ymm2, yword [rax]

minps xmm1, xmm2
minps xmm1, [rax]
minps xmm1, dqword [rax]
vminps xmm1, xmm2
vminps xmm1, [rax]
vminps xmm1, dqword [rax]
vminps xmm1, xmm2, xmm3
vminps xmm1, xmm2, [rax]
vminps xmm1, xmm2, dqword [rax]
vminps ymm1, ymm2, ymm3
vminps ymm1, ymm2, [rax]
vminps ymm1, ymm2, yword [rax]

minsd xmm1, xmm2
minsd xmm1, [rax]
minsd xmm1, qword [rax]
vminsd xmm1, xmm2
vminsd xmm1, [rax]
vminsd xmm1, qword [rax]
vminsd xmm1, xmm2, xmm3
vminsd xmm1, xmm2, [rax]
vminsd xmm1, xmm2, qword [rax]

minss xmm1, xmm2
minss xmm1, [rax]
minss xmm1, dword [rax]
vminss xmm1, xmm2
vminss xmm1, [rax]
vminss xmm1, dword [rax]
vminss xmm1, xmm2, xmm3
vminss xmm1, xmm2, [rax]
vminss xmm1, xmm2, dword [rax]

movapd xmm1, xmm2
movapd xmm1, [rax]
movapd xmm1, dqword [rax]
vmovapd xmm1, xmm2
vmovapd xmm1, [rax]
vmovapd xmm1, dqword [rax]
movapd [rax], xmm2
movapd dqword [rax], xmm2
vmovapd [rax], xmm2
vmovapd dqword [rax], xmm2
vmovapd ymm1, ymm2
vmovapd ymm1, [rax]
vmovapd ymm1, yword [rax]
vmovapd [rax], ymm2
vmovapd yword [rax], ymm2

movaps xmm1, xmm2
movaps xmm1, [rax]
movaps xmm1, dqword [rax]
vmovaps xmm1, xmm2
vmovaps xmm1, [rax]
vmovaps xmm1, dqword [rax]
movaps [rax], xmm2
movaps dqword [rax], xmm2
vmovaps [rax], xmm2
vmovaps dqword [rax], xmm2
vmovaps ymm1, ymm2
vmovaps ymm1, [rax]
vmovaps ymm1, yword [rax]
vmovaps [rax], ymm2
vmovaps yword [rax], ymm2

movd xmm1, eax
movd xmm1, [rax]
movd xmm1, dword [rax]
vmovd xmm1, eax
vmovd xmm1, [rax]
vmovd xmm1, dword [rax]
movd eax, xmm2
movd [rax], xmm2
movd dword [rax], xmm2
vmovd eax, xmm2
vmovd [rax], xmm2
vmovd dword [rax], xmm2

movq xmm1, rax
movq xmm1, [rax]
movq xmm1, qword [rax]
vmovq xmm1, rax
vmovq xmm1, [rax]
vmovq xmm1, qword [rax]
movq rax, xmm2
movq [rax], xmm2
movq qword [rax], xmm2
vmovq rax, xmm2
vmovq [rax], xmm2
vmovq qword [rax], xmm2

movq xmm1, xmm2
movq xmm1, [rax]
movq xmm1, qword [rax]
vmovq xmm1, xmm2
vmovq xmm1, [rax]
vmovq xmm1, qword [rax]
movq [rax], xmm1
movq qword [rax], xmm1
vmovq [rax], xmm1
vmovq qword [rax], xmm1

movddup xmm1, xmm2
movddup xmm1, [rax]
movddup xmm1, qword [rax]
vmovddup xmm1, xmm2
vmovddup xmm1, [rax]
vmovddup xmm1, qword [rax]
vmovddup ymm1, ymm2
vmovddup ymm1, [rax]
vmovddup ymm1, yword [rax]

movdqa xmm1, xmm2
movdqa xmm1, [rax]
movdqa xmm1, dqword [rax]
movdqa [rax], xmm2
movdqa dqword [rax], xmm2
vmovdqa xmm1, xmm2
vmovdqa xmm1, [rax]
vmovdqa xmm1, dqword [rax]
vmovdqa [rax], xmm2
vmovdqa dqword [rax], xmm2
vmovdqa ymm1, ymm2
vmovdqa ymm1, [rax]
vmovdqa ymm1, yword [rax]
vmovdqa [rax], ymm2
vmovdqa yword [rax], ymm2

movdqu xmm1, xmm2
movdqu xmm1, [rax]
movdqu xmm1, dqword [rax]
movdqu [rax], xmm2
movdqu dqword [rax], xmm2
vmovdqu xmm1, xmm2
vmovdqu xmm1, [rax]
vmovdqu xmm1, dqword [rax]
vmovdqu [rax], xmm2
vmovdqu dqword [rax], xmm2
vmovdqu ymm1, ymm2
vmovdqu ymm1, [rax]
vmovdqu ymm1, yword [rax]
vmovdqu [rax], ymm2
vmovdqu yword [rax], ymm2

movhlps xmm1, xmm2
vmovhlps xmm1, xmm2
vmovhlps xmm1, xmm2, xmm3

movhpd xmm1, [rax]
movhpd xmm1, qword [rax]
vmovhpd xmm1, [rax]
vmovhpd xmm1, qword [rax]
vmovhpd xmm1, xmm2, [rax]
vmovhpd xmm1, xmm2, qword [rax]
movhpd [rax], xmm2
movhpd qword [rax], xmm2
vmovhpd [rax], xmm2
vmovhpd qword [rax], xmm2

movhps xmm1, [rax]
movhps xmm1, qword [rax]
vmovhps xmm1, [rax]
vmovhps xmm1, qword [rax]
vmovhps xmm1, xmm2, [rax]
vmovhps xmm1, xmm2, qword [rax]
movhps [rax], xmm2
movhps qword [rax], xmm2
vmovhps [rax], xmm2
vmovhps qword [rax], xmm2

movhlps xmm1, xmm2
vmovhlps xmm1, xmm2
vmovhlps xmm1, xmm2, xmm3

movlpd xmm1, [rax]
movlpd xmm1, qword [rax]
vmovlpd xmm1, [rax]
vmovlpd xmm1, qword [rax]
vmovlpd xmm1, xmm2, [rax]
vmovlpd xmm1, xmm2, qword [rax]
movlpd [rax], xmm2
movlpd qword [rax], xmm2
vmovlpd [rax], xmm2
vmovlpd qword [rax], xmm2

movlps xmm1, [rax]
movlps xmm1, qword [rax]
vmovlps xmm1, [rax]
vmovlps xmm1, qword [rax]
vmovlps xmm1, xmm2, [rax]
vmovlps xmm1, xmm2, qword [rax]
movlps [rax], xmm2
movlps qword [rax], xmm2
vmovlps [rax], xmm2
vmovlps qword [rax], xmm2

movmskpd eax, xmm2
movmskpd rax, xmm2
vmovmskpd eax, xmm2
vmovmskpd rax, xmm2
vmovmskpd eax, ymm2
vmovmskpd rax, ymm2

movmskps eax, xmm2
movmskps rax, xmm2
vmovmskps eax, xmm2
vmovmskps rax, xmm2
vmovmskps eax, ymm2
vmovmskps rax, ymm2

movntdq [rax], xmm1
movntdq dqword [rax], xmm1
vmovntdq [rax], xmm1
vmovntdq dqword [rax], xmm1
vmovntdq [rax], ymm1
vmovntdq yword [rax], ymm1

movntdqa xmm1, [rax]
movntdqa xmm1, dqword [rax]
vmovntdqa xmm1, [rax]
vmovntdqa xmm1, dqword [rax]

movntpd [rax], xmm1
movntpd dqword [rax], xmm1
vmovntpd [rax], xmm1
vmovntpd dqword [rax], xmm1
vmovntpd [rax], ymm1
vmovntpd yword [rax], ymm1

movntps [rax], xmm1
movntps dqword [rax], xmm1
vmovntps [rax], xmm1
vmovntps dqword [rax], xmm1
vmovntps [rax], ymm1
vmovntps yword [rax], ymm1

movsd xmm1, xmm2
vmovsd xmm1, xmm2
vmovsd xmm1, xmm2, xmm3
movsd xmm1, [rax]
movsd xmm1, qword [rax]
vmovsd xmm1, [rax]
vmovsd xmm1, qword [rax]
movsd [rax], xmm2
movsd qword [rax], xmm2
vmovsd [rax], xmm2
vmovsd qword [rax], xmm2

movshdup xmm1, xmm2
movshdup xmm1, [rax]
movshdup xmm1, dqword [rax]
vmovshdup xmm1, xmm2
vmovshdup xmm1, [rax]
vmovshdup xmm1, dqword [rax]
vmovshdup ymm1, ymm2
vmovshdup ymm1, [rax]
vmovshdup ymm1, yword [rax]

movsldup xmm1, xmm2
movsldup xmm1, [rax]
movsldup xmm1, dqword [rax]
vmovsldup xmm1, xmm2
vmovsldup xmm1, [rax]
vmovsldup xmm1, dqword [rax]
vmovsldup ymm1, ymm2
vmovsldup ymm1, [rax]
vmovsldup ymm1, yword [rax]

movss xmm1, xmm2
vmovss xmm1, xmm2
vmovss xmm1, xmm2, xmm3
movss xmm1, [rax]
movss xmm1, dword [rax]
vmovss xmm1, [rax]
vmovss xmm1, dword [rax]
movss [rax], xmm2
movss dword [rax], xmm2
vmovss [rax], xmm2
vmovss dword [rax], xmm2

movupd xmm1, xmm2
movupd xmm1, [rax]
movupd xmm1, dqword [rax]
vmovupd xmm1, xmm2
vmovupd xmm1, [rax]
vmovupd xmm1, dqword [rax]
movupd [rax], xmm2
movupd dqword [rax], xmm2
vmovupd [rax], xmm2
vmovupd dqword [rax], xmm2
vmovupd ymm1, ymm2
vmovupd ymm1, [rax]
vmovupd ymm1, yword [rax]
vmovupd [rax], ymm2
vmovupd yword [rax], ymm2

movups xmm1, xmm2
movups xmm1, [rax]
movups xmm1, dqword [rax]
vmovups xmm1, xmm2
vmovups xmm1, [rax]
vmovups xmm1, dqword [rax]
movups [rax], xmm2
movups dqword [rax], xmm2
vmovups [rax], xmm2
vmovups dqword [rax], xmm2
vmovups ymm1, ymm2
vmovups ymm1, [rax]
vmovups ymm1, yword [rax]
vmovups [rax], ymm2
vmovups yword [rax], ymm2

mpsadbw xmm1, xmm2, 5
mpsadbw xmm1, [rax], byte 5
mpsadbw xmm1, dqword [rax], 5
vmpsadbw xmm1, xmm2, 5
vmpsadbw xmm1, [rax], byte 5
vmpsadbw xmm1, dqword [rax], 5
vmpsadbw xmm1, xmm2, xmm3, 5
vmpsadbw xmm1, xmm2, [rax], byte 5
vmpsadbw xmm1, xmm2, dqword [rax], 5

mulpd xmm1, xmm2
mulpd xmm1, [rax]
mulpd xmm1, dqword [rax]
vmulpd xmm1, xmm2
vmulpd xmm1, [rax]
vmulpd xmm1, dqword [rax]
vmulpd xmm1, xmm2, xmm3
vmulpd xmm1, xmm2, [rax]
vmulpd xmm1, xmm2, dqword [rax]
vmulpd ymm1, ymm2, ymm3
vmulpd ymm1, ymm2, [rax]
vmulpd ymm1, ymm2, yword [rax]

mulps xmm1, xmm2
mulps xmm1, [rax]
mulps xmm1, dqword [rax]
vmulps xmm1, xmm2
vmulps xmm1, [rax]
vmulps xmm1, dqword [rax]
vmulps xmm1, xmm2, xmm3
vmulps xmm1, xmm2, [rax]
vmulps xmm1, xmm2, dqword [rax]
vmulps ymm1, ymm2, ymm3
vmulps ymm1, ymm2, [rax]
vmulps ymm1, ymm2, yword [rax]

mulsd xmm1, xmm2
mulsd xmm1, [rax]
mulsd xmm1, qword [rax]
vmulsd xmm1, xmm2
vmulsd xmm1, [rax]
vmulsd xmm1, qword [rax]
vmulsd xmm1, xmm2, xmm3
vmulsd xmm1, xmm2, [rax]
vmulsd xmm1, xmm2, qword [rax]

mulss xmm1, xmm2
mulss xmm1, [rax]
mulss xmm1, dword [rax]
vmulss xmm1, xmm2
vmulss xmm1, [rax]
vmulss xmm1, dword [rax]
vmulss xmm1, xmm2, xmm3
vmulss xmm1, xmm2, [rax]
vmulss xmm1, xmm2, dword [rax]

orpd xmm1, xmm2
orpd xmm1, [rax]
orpd xmm1, dqword [rax]
vorpd xmm1, xmm2
vorpd xmm1, [rax]
vorpd xmm1, dqword [rax]
vorpd xmm1, xmm2, xmm3
vorpd xmm1, xmm2, [rax]
vorpd xmm1, xmm2, dqword [rax]
vorpd ymm1, ymm2, ymm3
vorpd ymm1, ymm2, [rax]
vorpd ymm1, ymm2, yword [rax]

orps xmm1, xmm2
orps xmm1, [rax]
orps xmm1, dqword [rax]
vorps xmm1, xmm2
vorps xmm1, [rax]
vorps xmm1, dqword [rax]
vorps xmm1, xmm2, xmm3
vorps xmm1, xmm2, [rax]
vorps xmm1, xmm2, dqword [rax]
vorps ymm1, ymm2, ymm3
vorps ymm1, ymm2, [rax]
vorps ymm1, ymm2, yword [rax]

pabsb xmm1, xmm2
pabsb xmm1, [rax]
pabsb xmm1, dqword [rax]
vpabsb xmm1, xmm2
vpabsb xmm1, [rax]
vpabsb xmm1, dqword [rax]

pabsw xmm1, xmm2
pabsw xmm1, [rax]
pabsw xmm1, dqword [rax]
vpabsw xmm1, xmm2
vpabsw xmm1, [rax]
vpabsw xmm1, dqword [rax]

pabsd xmm1, xmm2
pabsd xmm1, [rax]
pabsd xmm1, dqword [rax]
vpabsd xmm1, xmm2
vpabsd xmm1, [rax]
vpabsd xmm1, dqword [rax]

packsswb xmm1, xmm2
packsswb xmm1, [rax]
packsswb xmm1, dqword [rax]
vpacksswb xmm1, xmm2
vpacksswb xmm1, [rax]
vpacksswb xmm1, dqword [rax]
vpacksswb xmm1, xmm2, xmm3
vpacksswb xmm1, xmm2, [rax]
vpacksswb xmm1, xmm2, dqword [rax]

packssdw xmm1, xmm2
packssdw xmm1, [rax]
packssdw xmm1, dqword [rax]
vpackssdw xmm1, xmm2
vpackssdw xmm1, [rax]
vpackssdw xmm1, dqword [rax]
vpackssdw xmm1, xmm2, xmm3
vpackssdw xmm1, xmm2, [rax]
vpackssdw xmm1, xmm2, dqword [rax]

packuswb xmm1, xmm2
packuswb xmm1, [rax]
packuswb xmm1, dqword [rax]
vpackuswb xmm1, xmm2
vpackuswb xmm1, [rax]
vpackuswb xmm1, dqword [rax]
vpackuswb xmm1, xmm2, xmm3
vpackuswb xmm1, xmm2, [rax]
vpackuswb xmm1, xmm2, dqword [rax]

packusdw xmm1, xmm2
packusdw xmm1, [rax]
packusdw xmm1, dqword [rax]
vpackusdw xmm1, xmm2
vpackusdw xmm1, [rax]
vpackusdw xmm1, dqword [rax]
vpackusdw xmm1, xmm2, xmm3
vpackusdw xmm1, xmm2, [rax]
vpackusdw xmm1, xmm2, dqword [rax]

paddb xmm1, xmm2
paddb xmm1, [rax]
paddb xmm1, dqword [rax]
vpaddb xmm1, xmm2
vpaddb xmm1, [rax]
vpaddb xmm1, dqword [rax]
vpaddb xmm1, xmm2, xmm3
vpaddb xmm1, xmm2, [rax]
vpaddb xmm1, xmm2, dqword [rax]

paddw xmm1, xmm2
paddw xmm1, [rax]
paddw xmm1, dqword [rax]
vpaddw xmm1, xmm2
vpaddw xmm1, [rax]
vpaddw xmm1, dqword [rax]
vpaddw xmm1, xmm2, xmm3
vpaddw xmm1, xmm2, [rax]
vpaddw xmm1, xmm2, dqword [rax]

paddd xmm1, xmm2
paddd xmm1, [rax]
paddd xmm1, dqword [rax]
vpaddd xmm1, xmm2
vpaddd xmm1, [rax]
vpaddd xmm1, dqword [rax]
vpaddd xmm1, xmm2, xmm3
vpaddd xmm1, xmm2, [rax]
vpaddd xmm1, xmm2, dqword [rax]

paddq xmm1, xmm2
paddq xmm1, [rax]
paddq xmm1, dqword [rax]
vpaddq xmm1, xmm2
vpaddq xmm1, [rax]
vpaddq xmm1, dqword [rax]
vpaddq xmm1, xmm2, xmm3
vpaddq xmm1, xmm2, [rax]
vpaddq xmm1, xmm2, dqword [rax]

paddsb xmm1, xmm2
paddsb xmm1, [rax]
paddsb xmm1, dqword [rax]
vpaddsb xmm1, xmm2
vpaddsb xmm1, [rax]
vpaddsb xmm1, dqword [rax]
vpaddsb xmm1, xmm2, xmm3
vpaddsb xmm1, xmm2, [rax]
vpaddsb xmm1, xmm2, dqword [rax]

paddsw xmm1, xmm2
paddsw xmm1, [rax]
paddsw xmm1, dqword [rax]
vpaddsw xmm1, xmm2
vpaddsw xmm1, [rax]
vpaddsw xmm1, dqword [rax]
vpaddsw xmm1, xmm2, xmm3
vpaddsw xmm1, xmm2, [rax]
vpaddsw xmm1, xmm2, dqword [rax]

paddusb xmm1, xmm2
paddusb xmm1, [rax]
paddusb xmm1, dqword [rax]
vpaddusb xmm1, xmm2
vpaddusb xmm1, [rax]
vpaddusb xmm1, dqword [rax]
vpaddusb xmm1, xmm2, xmm3
vpaddusb xmm1, xmm2, [rax]
vpaddusb xmm1, xmm2, dqword [rax]

paddusw xmm1, xmm2
paddusw xmm1, [rax]
paddusw xmm1, dqword [rax]
vpaddusw xmm1, xmm2
vpaddusw xmm1, [rax]
vpaddusw xmm1, dqword [rax]
vpaddusw xmm1, xmm2, xmm3
vpaddusw xmm1, xmm2, [rax]
vpaddusw xmm1, xmm2, dqword [rax]

palignr xmm1, xmm2, 5
palignr xmm1, [rax], byte 5
palignr xmm1, dqword [rax], 5
vpalignr xmm1, xmm2, 5
vpalignr xmm1, [rax], byte 5
vpalignr xmm1, dqword [rax], 5
vpalignr xmm1, xmm2, xmm3, 5
vpalignr xmm1, xmm2, [rax], byte 5
vpalignr xmm1, xmm2, dqword [rax], 5

pand xmm1, xmm2
pand xmm1, [rax]
pand xmm1, dqword [rax]
vpand xmm1, xmm2
vpand xmm1, [rax]
vpand xmm1, dqword [rax]
vpand xmm1, xmm2, xmm3
vpand xmm1, xmm2, [rax]
vpand xmm1, xmm2, dqword [rax]

pandn xmm1, xmm2
pandn xmm1, [rax]
pandn xmm1, dqword [rax]
vpandn xmm1, xmm2
vpandn xmm1, [rax]
vpandn xmm1, dqword [rax]
vpandn xmm1, xmm2, xmm3
vpandn xmm1, xmm2, [rax]
vpandn xmm1, xmm2, dqword [rax]

pavgb xmm1, xmm2
pavgb xmm1, [rax]
pavgb xmm1, dqword [rax]
vpavgb xmm1, xmm2
vpavgb xmm1, [rax]
vpavgb xmm1, dqword [rax]
vpavgb xmm1, xmm2, xmm3
vpavgb xmm1, xmm2, [rax]
vpavgb xmm1, xmm2, dqword [rax]

pavgw xmm1, xmm2
pavgw xmm1, [rax]
pavgw xmm1, dqword [rax]
vpavgw xmm1, xmm2
vpavgw xmm1, [rax]
vpavgw xmm1, dqword [rax]
vpavgw xmm1, xmm2, xmm3
vpavgw xmm1, xmm2, [rax]
vpavgw xmm1, xmm2, dqword [rax]

; implicit XMM0 cannot be VEX encoded
pblendvb xmm1, xmm2
pblendvb xmm1, [rax]
pblendvb xmm1, dqword [rax]
pblendvb xmm1, xmm2, xmm0
pblendvb xmm1, [rax], xmm0
pblendvb xmm1, dqword [rax], xmm0
vpblendvb xmm1, xmm2, xmm3, xmm4
vpblendvb xmm1, xmm2, [rax], xmm4
vpblendvb xmm1, xmm2, dqword [rax], xmm4

pblendw xmm1, xmm2, 5
pblendw xmm1, [rax], byte 5
pblendw xmm1, dqword [rax], 5
vpblendw xmm1, xmm2, 5
vpblendw xmm1, [rax], byte 5
vpblendw xmm1, dqword [rax], 5
vpblendw xmm1, xmm2, xmm3, 5
vpblendw xmm1, xmm2, [rax], byte 5
vpblendw xmm1, xmm2, dqword [rax], 5

pcmpestri xmm1, xmm2, 5
pcmpestri xmm1, [rax], byte 5
pcmpestri xmm1, dqword [rax], 5
vpcmpestri xmm1, xmm2, 5
vpcmpestri xmm1, [rax], byte 5
vpcmpestri xmm1, dqword [rax], 5

pcmpestrm xmm1, xmm2, 5
pcmpestrm xmm1, [rax], byte 5
pcmpestrm xmm1, dqword [rax], 5
vpcmpestrm xmm1, xmm2, 5
vpcmpestrm xmm1, [rax], byte 5
vpcmpestrm xmm1, dqword [rax], 5

pcmpistri xmm1, xmm2, 5
pcmpistri xmm1, [rax], byte 5
pcmpistri xmm1, dqword [rax], 5
vpcmpistri xmm1, xmm2, 5
vpcmpistri xmm1, [rax], byte 5
vpcmpistri xmm1, dqword [rax], 5

pcmpistrm xmm1, xmm2, 5
pcmpistrm xmm1, [rax], byte 5
pcmpistrm xmm1, dqword [rax], 5
vpcmpistrm xmm1, xmm2, 5
vpcmpistrm xmm1, [rax], byte 5
vpcmpistrm xmm1, dqword [rax], 5

pcmpeqb xmm1, xmm2
pcmpeqb xmm1, [rax]
pcmpeqb xmm1, dqword [rax]
vpcmpeqb xmm1, xmm2
vpcmpeqb xmm1, [rax]
vpcmpeqb xmm1, dqword [rax]
vpcmpeqb xmm1, xmm2, xmm3
vpcmpeqb xmm1, xmm2, [rax]
vpcmpeqb xmm1, xmm2, dqword [rax]

pcmpeqw xmm1, xmm2
pcmpeqw xmm1, [rax]
pcmpeqw xmm1, dqword [rax]
vpcmpeqw xmm1, xmm2
vpcmpeqw xmm1, [rax]
vpcmpeqw xmm1, dqword [rax]
vpcmpeqw xmm1, xmm2, xmm3
vpcmpeqw xmm1, xmm2, [rax]
vpcmpeqw xmm1, xmm2, dqword [rax]

pcmpeqd xmm1, xmm2
pcmpeqd xmm1, [rax]
pcmpeqd xmm1, dqword [rax]
vpcmpeqd xmm1, xmm2
vpcmpeqd xmm1, [rax]
vpcmpeqd xmm1, dqword [rax]
vpcmpeqd xmm1, xmm2, xmm3
vpcmpeqd xmm1, xmm2, [rax]
vpcmpeqd xmm1, xmm2, dqword [rax]

pcmpeqq xmm1, xmm2
pcmpeqq xmm1, [rax]
pcmpeqq xmm1, dqword [rax]
vpcmpeqq xmm1, xmm2
vpcmpeqq xmm1, [rax]
vpcmpeqq xmm1, dqword [rax]
vpcmpeqq xmm1, xmm2, xmm3
vpcmpeqq xmm1, xmm2, [rax]
vpcmpeqq xmm1, xmm2, dqword [rax]

pcmpgtb xmm1, xmm2
pcmpgtb xmm1, [rax]
pcmpgtb xmm1, dqword [rax]
vpcmpgtb xmm1, xmm2
vpcmpgtb xmm1, [rax]
vpcmpgtb xmm1, dqword [rax]
vpcmpgtb xmm1, xmm2, xmm3
vpcmpgtb xmm1, xmm2, [rax]
vpcmpgtb xmm1, xmm2, dqword [rax]

pcmpgtw xmm1, xmm2
pcmpgtw xmm1, [rax]
pcmpgtw xmm1, dqword [rax]
vpcmpgtw xmm1, xmm2
vpcmpgtw xmm1, [rax]
vpcmpgtw xmm1, dqword [rax]
vpcmpgtw xmm1, xmm2, xmm3
vpcmpgtw xmm1, xmm2, [rax]
vpcmpgtw xmm1, xmm2, dqword [rax]

pcmpgtd xmm1, xmm2
pcmpgtd xmm1, [rax]
pcmpgtd xmm1, dqword [rax]
vpcmpgtd xmm1, xmm2
vpcmpgtd xmm1, [rax]
vpcmpgtd xmm1, dqword [rax]
vpcmpgtd xmm1, xmm2, xmm3
vpcmpgtd xmm1, xmm2, [rax]
vpcmpgtd xmm1, xmm2, dqword [rax]

pcmpgtq xmm1, xmm2
pcmpgtq xmm1, [rax]
pcmpgtq xmm1, dqword [rax]
vpcmpgtq xmm1, xmm2
vpcmpgtq xmm1, [rax]
vpcmpgtq xmm1, dqword [rax]
vpcmpgtq xmm1, xmm2, xmm3
vpcmpgtq xmm1, xmm2, [rax]
vpcmpgtq xmm1, xmm2, dqword [rax]

vpermilpd xmm1, xmm2, xmm3
vpermilpd xmm1, xmm2, [rax]
vpermilpd xmm1, xmm2, dqword [rax]
vpermilpd ymm1, ymm2, ymm3
vpermilpd ymm1, ymm2, [rax]
vpermilpd ymm1, ymm2, yword [rax]
vpermilpd xmm1, [rax], byte 5
vpermilpd xmm1, dqword [rax], 5
vpermilpd ymm1, [rax], byte 5
vpermilpd ymm1, yword [rax], 5

vpermilps xmm1, xmm2, xmm3
vpermilps xmm1, xmm2, [rax]
vpermilps xmm1, xmm2, dqword [rax]
vpermilps ymm1, ymm2, ymm3
vpermilps ymm1, ymm2, [rax]
vpermilps ymm1, ymm2, yword [rax]
vpermilps xmm1, [rax], byte 5
vpermilps xmm1, dqword [rax], 5
vpermilps ymm1, [rax], byte 5
vpermilps ymm1, yword [rax], 5

vperm2f128 ymm1, ymm2, ymm3, 5
vperm2f128 ymm1, ymm2, [rax], byte 5
vperm2f128 ymm1, ymm2, yword [rax], 5

pextrb eax, xmm2, 5
pextrb eax, xmm2, byte 5
pextrb rax, xmm2, 5
pextrb rax, xmm2, byte 5
pextrb byte [rax], xmm2, 5
pextrb [rax], xmm2, byte 5
vpextrb eax, xmm2, 5
vpextrb eax, xmm2, byte 5
vpextrb rax, xmm2, 5
vpextrb rax, xmm2, byte 5
vpextrb byte [rax], xmm2, 5
vpextrb [rax], xmm2, byte 5

pextrw eax, xmm2, 5
pextrw eax, xmm2, byte 5
pextrw rax, xmm2, 5
pextrw rax, xmm2, byte 5
pextrw word [rax], xmm2, 5
pextrw [rax], xmm2, byte 5
vpextrw eax, xmm2, 5
vpextrw eax, xmm2, byte 5
vpextrw rax, xmm2, 5
vpextrw rax, xmm2, byte 5
vpextrw word [rax], xmm2, 5
vpextrw [rax], xmm2, byte 5

pextrd eax, xmm2, 5
pextrd eax, xmm2, byte 5
pextrd dword [rax], xmm2, 5
pextrd [rax], xmm2, byte 5
vpextrd eax, xmm2, 5
vpextrd eax, xmm2, byte 5
vpextrd dword [rax], xmm2, 5
vpextrd [rax], xmm2, byte 5

pextrq rax, xmm2, 5
pextrq rax, xmm2, byte 5
pextrq qword [rax], xmm2, 5
pextrq [rax], xmm2, byte 5
vpextrq rax, xmm2, 5
vpextrq rax, xmm2, byte 5
vpextrq qword [rax], xmm2, 5
vpextrq [rax], xmm2, byte 5

phaddw xmm1, xmm2
phaddw xmm1, [rax]
phaddw xmm1, dqword [rax]
vphaddw xmm1, xmm2
vphaddw xmm1, [rax]
vphaddw xmm1, dqword [rax]
vphaddw xmm1, xmm2, xmm3
vphaddw xmm1, xmm2, [rax]
vphaddw xmm1, xmm2, dqword [rax]

phaddd xmm1, xmm2
phaddd xmm1, [rax]
phaddd xmm1, dqword [rax]
vphaddd xmm1, xmm2
vphaddd xmm1, [rax]
vphaddd xmm1, dqword [rax]
vphaddd xmm1, xmm2, xmm3
vphaddd xmm1, xmm2, [rax]
vphaddd xmm1, xmm2, dqword [rax]

phaddsw xmm1, xmm2
phaddsw xmm1, [rax]
phaddsw xmm1, dqword [rax]
vphaddsw xmm1, xmm2
vphaddsw xmm1, [rax]
vphaddsw xmm1, dqword [rax]
vphaddsw xmm1, xmm2, xmm3
vphaddsw xmm1, xmm2, [rax]
vphaddsw xmm1, xmm2, dqword [rax]

phminposuw xmm1, xmm2
phminposuw xmm1, [rax]
phminposuw xmm1, dqword [rax]
vphminposuw xmm1, xmm2
vphminposuw xmm1, [rax]
vphminposuw xmm1, dqword [rax]

phsubw xmm1, xmm2
phsubw xmm1, [rax]
phsubw xmm1, dqword [rax]
vphsubw xmm1, xmm2
vphsubw xmm1, [rax]
vphsubw xmm1, dqword [rax]
vphsubw xmm1, xmm2, xmm3
vphsubw xmm1, xmm2, [rax]
vphsubw xmm1, xmm2, dqword [rax]

phsubd xmm1, xmm2
phsubd xmm1, [rax]
phsubd xmm1, dqword [rax]
vphsubd xmm1, xmm2
vphsubd xmm1, [rax]
vphsubd xmm1, dqword [rax]
vphsubd xmm1, xmm2, xmm3
vphsubd xmm1, xmm2, [rax]
vphsubd xmm1, xmm2, dqword [rax]

phsubsw xmm1, xmm2
phsubsw xmm1, [rax]
phsubsw xmm1, dqword [rax]
vphsubsw xmm1, xmm2
vphsubsw xmm1, [rax]
vphsubsw xmm1, dqword [rax]
vphsubsw xmm1, xmm2, xmm3
vphsubsw xmm1, xmm2, [rax]
vphsubsw xmm1, xmm2, dqword [rax]

pinsrb xmm1, eax, 5
pinsrb xmm1, byte [rax], 5
pinsrb xmm1, [rax], byte 5
vpinsrb xmm1, eax, 5
vpinsrb xmm1, byte [rax], 5
vpinsrb xmm1, [rax], byte 5
vpinsrb xmm1, xmm2, eax, 5
vpinsrb xmm1, xmm2, byte [rax], 5
vpinsrb xmm1, xmm2, [rax], byte 5

pinsrw xmm1, eax, 5
pinsrw xmm1, word [rax], 5
pinsrw xmm1, [rax], byte 5
vpinsrw xmm1, eax, 5
vpinsrw xmm1, word [rax], 5
vpinsrw xmm1, [rax], byte 5
vpinsrw xmm1, xmm2, eax, 5
vpinsrw xmm1, xmm2, word [rax], 5
vpinsrw xmm1, xmm2, [rax], byte 5

pinsrd xmm1, eax, 5
pinsrd xmm1, dword [rax], 5
pinsrd xmm1, [rax], byte 5
vpinsrd xmm1, eax, 5
vpinsrd xmm1, dword [rax], 5
vpinsrd xmm1, [rax], byte 5
vpinsrd xmm1, xmm2, eax, 5
vpinsrd xmm1, xmm2, dword [rax], 5
vpinsrd xmm1, xmm2, [rax], byte 5

pinsrq xmm1, rax, 5
pinsrq xmm1, qword [rax], 5
pinsrq xmm1, [rax], byte 5
vpinsrq xmm1, rax, 5
vpinsrq xmm1, qword [rax], 5
vpinsrq xmm1, [rax], byte 5
vpinsrq xmm1, xmm2, rax, 5
vpinsrq xmm1, xmm2, qword [rax], 5
vpinsrq xmm1, xmm2, [rax], byte 5

pmaddwd xmm1, xmm2
pmaddwd xmm1, [rax]
pmaddwd xmm1, dqword [rax]
vpmaddwd xmm1, xmm2
vpmaddwd xmm1, [rax]
vpmaddwd xmm1, dqword [rax]
vpmaddwd xmm1, xmm2, xmm3
vpmaddwd xmm1, xmm2, [rax]
vpmaddwd xmm1, xmm2, dqword [rax]

pmaddubsw xmm1, xmm2
pmaddubsw xmm1, [rax]
pmaddubsw xmm1, dqword [rax]
vpmaddubsw xmm1, xmm2
vpmaddubsw xmm1, [rax]
vpmaddubsw xmm1, dqword [rax]
vpmaddubsw xmm1, xmm2, xmm3
vpmaddubsw xmm1, xmm2, [rax]
vpmaddubsw xmm1, xmm2, dqword [rax]

pmaxsb xmm1, xmm2
pmaxsb xmm1, [rax]
pmaxsb xmm1, dqword [rax]
vpmaxsb xmm1, xmm2
vpmaxsb xmm1, [rax]
vpmaxsb xmm1, dqword [rax]
vpmaxsb xmm1, xmm2, xmm3
vpmaxsb xmm1, xmm2, [rax]
vpmaxsb xmm1, xmm2, dqword [rax]

pmaxsw xmm1, xmm2
pmaxsw xmm1, [rax]
pmaxsw xmm1, dqword [rax]
vpmaxsw xmm1, xmm2
vpmaxsw xmm1, [rax]
vpmaxsw xmm1, dqword [rax]
vpmaxsw xmm1, xmm2, xmm3
vpmaxsw xmm1, xmm2, [rax]
vpmaxsw xmm1, xmm2, dqword [rax]

pmaxsd xmm1, xmm2
pmaxsd xmm1, [rax]
pmaxsd xmm1, dqword [rax]
vpmaxsd xmm1, xmm2
vpmaxsd xmm1, [rax]
vpmaxsd xmm1, dqword [rax]
vpmaxsd xmm1, xmm2, xmm3
vpmaxsd xmm1, xmm2, [rax]
vpmaxsd xmm1, xmm2, dqword [rax]

pmaxub xmm1, xmm2
pmaxub xmm1, [rax]
pmaxub xmm1, dqword [rax]
vpmaxub xmm1, xmm2
vpmaxub xmm1, [rax]
vpmaxub xmm1, dqword [rax]
vpmaxub xmm1, xmm2, xmm3
vpmaxub xmm1, xmm2, [rax]
vpmaxub xmm1, xmm2, dqword [rax]

pmaxuw xmm1, xmm2
pmaxuw xmm1, [rax]
pmaxuw xmm1, dqword [rax]
vpmaxuw xmm1, xmm2
vpmaxuw xmm1, [rax]
vpmaxuw xmm1, dqword [rax]
vpmaxuw xmm1, xmm2, xmm3
vpmaxuw xmm1, xmm2, [rax]
vpmaxuw xmm1, xmm2, dqword [rax]

pmaxud xmm1, xmm2
pmaxud xmm1, [rax]
pmaxud xmm1, dqword [rax]
vpmaxud xmm1, xmm2
vpmaxud xmm1, [rax]
vpmaxud xmm1, dqword [rax]
vpmaxud xmm1, xmm2, xmm3
vpmaxud xmm1, xmm2, [rax]
vpmaxud xmm1, xmm2, dqword [rax]

pminsb xmm1, xmm2
pminsb xmm1, [rax]
pminsb xmm1, dqword [rax]
vpminsb xmm1, xmm2
vpminsb xmm1, [rax]
vpminsb xmm1, dqword [rax]
vpminsb xmm1, xmm2, xmm3
vpminsb xmm1, xmm2, [rax]
vpminsb xmm1, xmm2, dqword [rax]

pminsw xmm1, xmm2
pminsw xmm1, [rax]
pminsw xmm1, dqword [rax]
vpminsw xmm1, xmm2
vpminsw xmm1, [rax]
vpminsw xmm1, dqword [rax]
vpminsw xmm1, xmm2, xmm3
vpminsw xmm1, xmm2, [rax]
vpminsw xmm1, xmm2, dqword [rax]

pminsd xmm1, xmm2
pminsd xmm1, [rax]
pminsd xmm1, dqword [rax]
vpminsd xmm1, xmm2
vpminsd xmm1, [rax]
vpminsd xmm1, dqword [rax]
vpminsd xmm1, xmm2, xmm3
vpminsd xmm1, xmm2, [rax]
vpminsd xmm1, xmm2, dqword [rax]

pminub xmm1, xmm2
pminub xmm1, [rax]
pminub xmm1, dqword [rax]
vpminub xmm1, xmm2
vpminub xmm1, [rax]
vpminub xmm1, dqword [rax]
vpminub xmm1, xmm2, xmm3
vpminub xmm1, xmm2, [rax]
vpminub xmm1, xmm2, dqword [rax]

pminuw xmm1, xmm2
pminuw xmm1, [rax]
pminuw xmm1, dqword [rax]
vpminuw xmm1, xmm2
vpminuw xmm1, [rax]
vpminuw xmm1, dqword [rax]
vpminuw xmm1, xmm2, xmm3
vpminuw xmm1, xmm2, [rax]
vpminuw xmm1, xmm2, dqword [rax]

pminud xmm1, xmm2
pminud xmm1, [rax]
pminud xmm1, dqword [rax]
vpminud xmm1, xmm2
vpminud xmm1, [rax]
vpminud xmm1, dqword [rax]
vpminud xmm1, xmm2, xmm3
vpminud xmm1, xmm2, [rax]
vpminud xmm1, xmm2, dqword [rax]

pmovmskb eax, xmm1
pmovmskb rax, xmm1
vpmovmskb eax, xmm1
vpmovmskb rax, xmm1

pmovsxbw xmm1, xmm2
pmovsxbw xmm1, [rax]
pmovsxbw xmm1, qword [rax]
vpmovsxbw xmm1, xmm2
vpmovsxbw xmm1, [rax]
vpmovsxbw xmm1, qword [rax]

pmovsxbd xmm1, xmm2
pmovsxbd xmm1, [rax]
pmovsxbd xmm1, dword [rax]
vpmovsxbd xmm1, xmm2
vpmovsxbd xmm1, [rax]
vpmovsxbd xmm1, dword [rax]

pmovsxbq xmm1, xmm2
pmovsxbq xmm1, [rax]
pmovsxbq xmm1, word [rax]
vpmovsxbq xmm1, xmm2
vpmovsxbq xmm1, [rax]
vpmovsxbq xmm1, word [rax]

pmovsxwd xmm1, xmm2
pmovsxwd xmm1, [rax]
pmovsxwd xmm1, qword [rax]
vpmovsxwd xmm1, xmm2
vpmovsxwd xmm1, [rax]
vpmovsxwd xmm1, qword [rax]

pmovsxwq xmm1, xmm2
pmovsxwq xmm1, [rax]
pmovsxwq xmm1, dword [rax]
vpmovsxwq xmm1, xmm2
vpmovsxwq xmm1, [rax]
vpmovsxwq xmm1, dword [rax]

pmovsxdq xmm1, xmm2
pmovsxdq xmm1, [rax]
pmovsxdq xmm1, qword [rax]
vpmovsxdq xmm1, xmm2
vpmovsxdq xmm1, [rax]
vpmovsxdq xmm1, qword [rax]

pmovzxbw xmm1, xmm2
pmovzxbw xmm1, [rax]
pmovzxbw xmm1, qword [rax]
vpmovzxbw xmm1, xmm2
vpmovzxbw xmm1, [rax]
vpmovzxbw xmm1, qword [rax]

pmovzxbd xmm1, xmm2
pmovzxbd xmm1, [rax]
pmovzxbd xmm1, dword [rax]
vpmovzxbd xmm1, xmm2
vpmovzxbd xmm1, [rax]
vpmovzxbd xmm1, dword [rax]

pmovzxbq xmm1, xmm2
pmovzxbq xmm1, [rax]
pmovzxbq xmm1, word [rax]
vpmovzxbq xmm1, xmm2
vpmovzxbq xmm1, [rax]
vpmovzxbq xmm1, word [rax]

pmovzxwd xmm1, xmm2
pmovzxwd xmm1, [rax]
pmovzxwd xmm1, qword [rax]
vpmovzxwd xmm1, xmm2
vpmovzxwd xmm1, [rax]
vpmovzxwd xmm1, qword [rax]

pmovzxwq xmm1, xmm2
pmovzxwq xmm1, [rax]
pmovzxwq xmm1, dword [rax]
vpmovzxwq xmm1, xmm2
vpmovzxwq xmm1, [rax]
vpmovzxwq xmm1, dword [rax]

pmovzxdq xmm1, xmm2
pmovzxdq xmm1, [rax]
pmovzxdq xmm1, qword [rax]
vpmovzxdq xmm1, xmm2
vpmovzxdq xmm1, [rax]
vpmovzxdq xmm1, qword [rax]

pmulhuw xmm1, xmm2
pmulhuw xmm1, [rax]
pmulhuw xmm1, dqword [rax]
vpmulhuw xmm1, xmm2
vpmulhuw xmm1, [rax]
vpmulhuw xmm1, dqword [rax]
vpmulhuw xmm1, xmm2, xmm3
vpmulhuw xmm1, xmm2, [rax]
vpmulhuw xmm1, xmm2, dqword [rax]

pmulhrsw xmm1, xmm2
pmulhrsw xmm1, [rax]
pmulhrsw xmm1, dqword [rax]
vpmulhrsw xmm1, xmm2
vpmulhrsw xmm1, [rax]
vpmulhrsw xmm1, dqword [rax]
vpmulhrsw xmm1, xmm2, xmm3
vpmulhrsw xmm1, xmm2, [rax]
vpmulhrsw xmm1, xmm2, dqword [rax]

pmulhw xmm1, xmm2
pmulhw xmm1, [rax]
pmulhw xmm1, dqword [rax]
vpmulhw xmm1, xmm2
vpmulhw xmm1, [rax]
vpmulhw xmm1, dqword [rax]
vpmulhw xmm1, xmm2, xmm3
vpmulhw xmm1, xmm2, [rax]
vpmulhw xmm1, xmm2, dqword [rax]

pmullw xmm1, xmm2
pmullw xmm1, [rax]
pmullw xmm1, dqword [rax]
vpmullw xmm1, xmm2
vpmullw xmm1, [rax]
vpmullw xmm1, dqword [rax]
vpmullw xmm1, xmm2, xmm3
vpmullw xmm1, xmm2, [rax]
vpmullw xmm1, xmm2, dqword [rax]

pmulld xmm1, xmm2
pmulld xmm1, [rax]
pmulld xmm1, dqword [rax]
vpmulld xmm1, xmm2
vpmulld xmm1, [rax]
vpmulld xmm1, dqword [rax]
vpmulld xmm1, xmm2, xmm3
vpmulld xmm1, xmm2, [rax]
vpmulld xmm1, xmm2, dqword [rax]

pmuludq xmm1, xmm2
pmuludq xmm1, [rax]
pmuludq xmm1, dqword [rax]
vpmuludq xmm1, xmm2
vpmuludq xmm1, [rax]
vpmuludq xmm1, dqword [rax]
vpmuludq xmm1, xmm2, xmm3
vpmuludq xmm1, xmm2, [rax]
vpmuludq xmm1, xmm2, dqword [rax]

pmuldq xmm1, xmm2
pmuldq xmm1, [rax]
pmuldq xmm1, dqword [rax]
vpmuldq xmm1, xmm2
vpmuldq xmm1, [rax]
vpmuldq xmm1, dqword [rax]
vpmuldq xmm1, xmm2, xmm3
vpmuldq xmm1, xmm2, [rax]
vpmuldq xmm1, xmm2, dqword [rax]

por xmm1, xmm2
por xmm1, [rax]
por xmm1, dqword [rax]
vpor xmm1, xmm2
vpor xmm1, [rax]
vpor xmm1, dqword [rax]
vpor xmm1, xmm2, xmm3
vpor xmm1, xmm2, [rax]
vpor xmm1, xmm2, dqword [rax]

psadbw xmm1, xmm2
psadbw xmm1, [rax]
psadbw xmm1, dqword [rax]
vpsadbw xmm1, xmm2
vpsadbw xmm1, [rax]
vpsadbw xmm1, dqword [rax]
vpsadbw xmm1, xmm2, xmm3
vpsadbw xmm1, xmm2, [rax]
vpsadbw xmm1, xmm2, dqword [rax]

pshufb xmm1, xmm2
pshufb xmm1, [rax]
pshufb xmm1, dqword [rax]
vpshufb xmm1, xmm2
vpshufb xmm1, [rax]
vpshufb xmm1, dqword [rax]
vpshufb xmm1, xmm2, xmm3
vpshufb xmm1, xmm2, [rax]
vpshufb xmm1, xmm2, dqword [rax]

pshufd xmm1, xmm2, 5
pshufd xmm1, [rax], byte 5
pshufd xmm1, dqword [rax], 5
vpshufd xmm1, xmm2, 5
vpshufd xmm1, [rax], byte 5
vpshufd xmm1, dqword [rax], 5

pshufhw xmm1, xmm2, 5
pshufhw xmm1, [rax], byte 5
pshufhw xmm1, dqword [rax], 5
vpshufhw xmm1, xmm2, 5
vpshufhw xmm1, [rax], byte 5
vpshufhw xmm1, dqword [rax], 5

pshuflw xmm1, xmm2, 5
pshuflw xmm1, [rax], byte 5
pshuflw xmm1, dqword [rax], 5
vpshuflw xmm1, xmm2, 5
vpshuflw xmm1, [rax], byte 5
vpshuflw xmm1, dqword [rax], 5

psignb xmm1, xmm2
psignb xmm1, [rax]
psignb xmm1, dqword [rax]
vpsignb xmm1, xmm2
vpsignb xmm1, [rax]
vpsignb xmm1, dqword [rax]
vpsignb xmm1, xmm2, xmm3
vpsignb xmm1, xmm2, [rax]
vpsignb xmm1, xmm2, dqword [rax]

psignw xmm1, xmm2
psignw xmm1, [rax]
psignw xmm1, dqword [rax]
vpsignw xmm1, xmm2
vpsignw xmm1, [rax]
vpsignw xmm1, dqword [rax]
vpsignw xmm1, xmm2, xmm3
vpsignw xmm1, xmm2, [rax]
vpsignw xmm1, xmm2, dqword [rax]

psignd xmm1, xmm2
psignd xmm1, [rax]
psignd xmm1, dqword [rax]
vpsignd xmm1, xmm2
vpsignd xmm1, [rax]
vpsignd xmm1, dqword [rax]
vpsignd xmm1, xmm2, xmm3
vpsignd xmm1, xmm2, [rax]
vpsignd xmm1, xmm2, dqword [rax]

; Test these with high regs as it goes into VEX.B (REX.B)
pslldq xmm11, 5
pslldq xmm11, byte 5
vpslldq xmm11, 5
vpslldq xmm11, byte 5
vpslldq xmm11, xmm12, 5
vpslldq xmm11, xmm12, byte 5

pslldq xmm1, 5
pslldq xmm1, byte 5
vpslldq xmm1, 5
vpslldq xmm1, byte 5
vpslldq xmm1, xmm2, 5
vpslldq xmm1, xmm2, byte 5

psrldq xmm1, 5
psrldq xmm1, byte 5
vpsrldq xmm1, 5
vpsrldq xmm1, byte 5
vpsrldq xmm1, xmm2, 5
vpsrldq xmm1, xmm2, byte 5

psllw xmm1, xmm2
psllw xmm1, [rax]
psllw xmm1, dqword [rax]
vpsllw xmm1, xmm2
vpsllw xmm1, [rax]
vpsllw xmm1, dqword [rax]
vpsllw xmm1, xmm2, xmm3
vpsllw xmm1, xmm2, [rax]
vpsllw xmm1, xmm2, dqword [rax]
psllw xmm1, 5
psllw xmm1, byte 5
vpsllw xmm1, 5
vpsllw xmm1, byte 5
vpsllw xmm1, xmm2, 5
vpsllw xmm1, xmm2, byte 5

pslld xmm1, xmm2
pslld xmm1, [rax]
pslld xmm1, dqword [rax]
vpslld xmm1, xmm2
vpslld xmm1, [rax]
vpslld xmm1, dqword [rax]
vpslld xmm1, xmm2, xmm3
vpslld xmm1, xmm2, [rax]
vpslld xmm1, xmm2, dqword [rax]
pslld xmm1, 5
pslld xmm1, byte 5
vpslld xmm1, 5
vpslld xmm1, byte 5
vpslld xmm1, xmm2, 5
vpslld xmm1, xmm2, byte 5

psllq xmm1, xmm2
psllq xmm1, [rax]
psllq xmm1, dqword [rax]
vpsllq xmm1, xmm2
vpsllq xmm1, [rax]
vpsllq xmm1, dqword [rax]
vpsllq xmm1, xmm2, xmm3
vpsllq xmm1, xmm2, [rax]
vpsllq xmm1, xmm2, dqword [rax]
psllq xmm1, 5
psllq xmm1, byte 5
vpsllq xmm1, 5
vpsllq xmm1, byte 5
vpsllq xmm1, xmm2, 5
vpsllq xmm1, xmm2, byte 5

psraw xmm1, xmm2
psraw xmm1, [rax]
psraw xmm1, dqword [rax]
vpsraw xmm1, xmm2
vpsraw xmm1, [rax]
vpsraw xmm1, dqword [rax]
vpsraw xmm1, xmm2, xmm3
vpsraw xmm1, xmm2, [rax]
vpsraw xmm1, xmm2, dqword [rax]
psraw xmm1, 5
psraw xmm1, byte 5
vpsraw xmm1, 5
vpsraw xmm1, byte 5
vpsraw xmm1, xmm2, 5
vpsraw xmm1, xmm2, byte 5

psrad xmm1, xmm2
psrad xmm1, [rax]
psrad xmm1, dqword [rax]
vpsrad xmm1, xmm2
vpsrad xmm1, [rax]
vpsrad xmm1, dqword [rax]
vpsrad xmm1, xmm2, xmm3
vpsrad xmm1, xmm2, [rax]
vpsrad xmm1, xmm2, dqword [rax]
psrad xmm1, 5
psrad xmm1, byte 5
vpsrad xmm1, 5
vpsrad xmm1, byte 5
vpsrad xmm1, xmm2, 5
vpsrad xmm1, xmm2, byte 5

psrlw xmm1, xmm2
psrlw xmm1, [rax]
psrlw xmm1, dqword [rax]
vpsrlw xmm1, xmm2
vpsrlw xmm1, [rax]
vpsrlw xmm1, dqword [rax]
vpsrlw xmm1, xmm2, xmm3
vpsrlw xmm1, xmm2, [rax]
vpsrlw xmm1, xmm2, dqword [rax]
psrlw xmm1, 5
psrlw xmm1, byte 5
vpsrlw xmm1, 5
vpsrlw xmm1, byte 5
vpsrlw xmm1, xmm2, 5
vpsrlw xmm1, xmm2, byte 5

psrld xmm1, xmm2
psrld xmm1, [rax]
psrld xmm1, dqword [rax]
vpsrld xmm1, xmm2
vpsrld xmm1, [rax]
vpsrld xmm1, dqword [rax]
vpsrld xmm1, xmm2, xmm3
vpsrld xmm1, xmm2, [rax]
vpsrld xmm1, xmm2, dqword [rax]
psrld xmm1, 5
psrld xmm1, byte 5
vpsrld xmm1, 5
vpsrld xmm1, byte 5
vpsrld xmm1, xmm2, 5
vpsrld xmm1, xmm2, byte 5

psrlq xmm1, xmm2
psrlq xmm1, [rax]
psrlq xmm1, dqword [rax]
vpsrlq xmm1, xmm2
vpsrlq xmm1, [rax]
vpsrlq xmm1, dqword [rax]
vpsrlq xmm1, xmm2, xmm3
vpsrlq xmm1, xmm2, [rax]
vpsrlq xmm1, xmm2, dqword [rax]
psrlq xmm1, 5
psrlq xmm1, byte 5
vpsrlq xmm1, 5
vpsrlq xmm1, byte 5
vpsrlq xmm1, xmm2, 5
vpsrlq xmm1, xmm2, byte 5

ptest xmm1, xmm2
ptest xmm1, [rax]
ptest xmm1, dqword [rax]
vptest xmm1, xmm2
vptest xmm1, [rax]
vptest xmm1, dqword [rax]
vptest ymm1, ymm2
vptest ymm1, [rax]
vptest ymm1, yword [rax]

vtestps xmm1, xmm2
vtestps xmm1, [rax]
vtestps xmm1, dqword [rax]
vtestps ymm1, ymm2
vtestps ymm1, [rax]
vtestps ymm1, yword [rax]

vtestpd xmm1, xmm2
vtestpd xmm1, [rax]
vtestpd xmm1, dqword [rax]
vtestpd ymm1, ymm2
vtestpd ymm1, [rax]
vtestpd ymm1, yword [rax]

psubb xmm1, xmm2
psubb xmm1, [rax]
psubb xmm1, dqword [rax]
vpsubb xmm1, xmm2
vpsubb xmm1, [rax]
vpsubb xmm1, dqword [rax]
vpsubb xmm1, xmm2, xmm3
vpsubb xmm1, xmm2, [rax]
vpsubb xmm1, xmm2, dqword [rax]

psubw xmm1, xmm2
psubw xmm1, [rax]
psubw xmm1, dqword [rax]
vpsubw xmm1, xmm2
vpsubw xmm1, [rax]
vpsubw xmm1, dqword [rax]
vpsubw xmm1, xmm2, xmm3
vpsubw xmm1, xmm2, [rax]
vpsubw xmm1, xmm2, dqword [rax]

psubd xmm1, xmm2
psubd xmm1, [rax]
psubd xmm1, dqword [rax]
vpsubd xmm1, xmm2
vpsubd xmm1, [rax]
vpsubd xmm1, dqword [rax]
vpsubd xmm1, xmm2, xmm3
vpsubd xmm1, xmm2, [rax]
vpsubd xmm1, xmm2, dqword [rax]

psubq xmm1, xmm2
psubq xmm1, [rax]
psubq xmm1, dqword [rax]
vpsubq xmm1, xmm2
vpsubq xmm1, [rax]
vpsubq xmm1, dqword [rax]
vpsubq xmm1, xmm2, xmm3
vpsubq xmm1, xmm2, [rax]
vpsubq xmm1, xmm2, dqword [rax]

psubsb xmm1, xmm2
psubsb xmm1, [rax]
psubsb xmm1, dqword [rax]
vpsubsb xmm1, xmm2
vpsubsb xmm1, [rax]
vpsubsb xmm1, dqword [rax]
vpsubsb xmm1, xmm2, xmm3
vpsubsb xmm1, xmm2, [rax]
vpsubsb xmm1, xmm2, dqword [rax]

psubsw xmm1, xmm2
psubsw xmm1, [rax]
psubsw xmm1, dqword [rax]
vpsubsw xmm1, xmm2
vpsubsw xmm1, [rax]
vpsubsw xmm1, dqword [rax]
vpsubsw xmm1, xmm2, xmm3
vpsubsw xmm1, xmm2, [rax]
vpsubsw xmm1, xmm2, dqword [rax]

psubusb xmm1, xmm2
psubusb xmm1, [rax]
psubusb xmm1, dqword [rax]
vpsubusb xmm1, xmm2
vpsubusb xmm1, [rax]
vpsubusb xmm1, dqword [rax]
vpsubusb xmm1, xmm2, xmm3
vpsubusb xmm1, xmm2, [rax]
vpsubusb xmm1, xmm2, dqword [rax]

psubusw xmm1, xmm2
psubusw xmm1, [rax]
psubusw xmm1, dqword [rax]
vpsubusw xmm1, xmm2
vpsubusw xmm1, [rax]
vpsubusw xmm1, dqword [rax]
vpsubusw xmm1, xmm2, xmm3
vpsubusw xmm1, xmm2, [rax]
vpsubusw xmm1, xmm2, dqword [rax]

punpckhbw xmm1, xmm2
punpckhbw xmm1, [rax]
punpckhbw xmm1, dqword [rax]
vpunpckhbw xmm1, xmm2
vpunpckhbw xmm1, [rax]
vpunpckhbw xmm1, dqword [rax]
vpunpckhbw xmm1, xmm2, xmm3
vpunpckhbw xmm1, xmm2, [rax]
vpunpckhbw xmm1, xmm2, dqword [rax]

punpckhwd xmm1, xmm2
punpckhwd xmm1, [rax]
punpckhwd xmm1, dqword [rax]
vpunpckhwd xmm1, xmm2
vpunpckhwd xmm1, [rax]
vpunpckhwd xmm1, dqword [rax]
vpunpckhwd xmm1, xmm2, xmm3
vpunpckhwd xmm1, xmm2, [rax]
vpunpckhwd xmm1, xmm2, dqword [rax]

punpckhdq xmm1, xmm2
punpckhdq xmm1, [rax]
punpckhdq xmm1, dqword [rax]
vpunpckhdq xmm1, xmm2
vpunpckhdq xmm1, [rax]
vpunpckhdq xmm1, dqword [rax]
vpunpckhdq xmm1, xmm2, xmm3
vpunpckhdq xmm1, xmm2, [rax]
vpunpckhdq xmm1, xmm2, dqword [rax]

punpckhqdq xmm1, xmm2
punpckhqdq xmm1, [rax]
punpckhqdq xmm1, dqword [rax]
vpunpckhqdq xmm1, xmm2
vpunpckhqdq xmm1, [rax]
vpunpckhqdq xmm1, dqword [rax]
vpunpckhqdq xmm1, xmm2, xmm3
vpunpckhqdq xmm1, xmm2, [rax]
vpunpckhqdq xmm1, xmm2, dqword [rax]

punpcklbw xmm1, xmm2
punpcklbw xmm1, [rax]
punpcklbw xmm1, dqword [rax]
vpunpcklbw xmm1, xmm2
vpunpcklbw xmm1, [rax]
vpunpcklbw xmm1, dqword [rax]
vpunpcklbw xmm1, xmm2, xmm3
vpunpcklbw xmm1, xmm2, [rax]
vpunpcklbw xmm1, xmm2, dqword [rax]

punpcklwd xmm1, xmm2
punpcklwd xmm1, [rax]
punpcklwd xmm1, dqword [rax]
vpunpcklwd xmm1, xmm2
vpunpcklwd xmm1, [rax]
vpunpcklwd xmm1, dqword [rax]
vpunpcklwd xmm1, xmm2, xmm3
vpunpcklwd xmm1, xmm2, [rax]
vpunpcklwd xmm1, xmm2, dqword [rax]

punpckldq xmm1, xmm2
punpckldq xmm1, [rax]
punpckldq xmm1, dqword [rax]
vpunpckldq xmm1, xmm2
vpunpckldq xmm1, [rax]
vpunpckldq xmm1, dqword [rax]
vpunpckldq xmm1, xmm2, xmm3
vpunpckldq xmm1, xmm2, [rax]
vpunpckldq xmm1, xmm2, dqword [rax]

punpcklqdq xmm1, xmm2
punpcklqdq xmm1, [rax]
punpcklqdq xmm1, dqword [rax]
vpunpcklqdq xmm1, xmm2
vpunpcklqdq xmm1, [rax]
vpunpcklqdq xmm1, dqword [rax]
vpunpcklqdq xmm1, xmm2, xmm3
vpunpcklqdq xmm1, xmm2, [rax]
vpunpcklqdq xmm1, xmm2, dqword [rax]

pxor xmm1, xmm2
pxor xmm1, [rax]
pxor xmm1, dqword [rax]
vpxor xmm1, xmm2
vpxor xmm1, [rax]
vpxor xmm1, dqword [rax]
vpxor xmm1, xmm2, xmm3
vpxor xmm1, xmm2, [rax]
vpxor xmm1, xmm2, dqword [rax]

rcpps xmm1, xmm2
rcpps xmm1, [rax]
rcpps xmm1, dqword [rax]
vrcpps xmm1, xmm2
vrcpps xmm1, [rax]
vrcpps xmm1, dqword [rax]
vrcpps ymm1, ymm2
vrcpps ymm1, [rax]
vrcpps ymm1, yword [rax]

rcpss xmm1, xmm2
rcpss xmm1, [rax]
rcpss xmm1, dword [rax]
vrcpss xmm1, xmm2
vrcpss xmm1, [rax]
vrcpss xmm1, dword [rax]
vrcpss xmm1, xmm2, xmm3
vrcpss xmm1, xmm2, [rax]
vrcpss xmm1, xmm2, dword [rax]

rsqrtps xmm1, xmm2
rsqrtps xmm1, [rax]
rsqrtps xmm1, dqword [rax]
vrsqrtps xmm1, xmm2
vrsqrtps xmm1, [rax]
vrsqrtps xmm1, dqword [rax]
vrsqrtps ymm1, ymm2
vrsqrtps ymm1, [rax]
vrsqrtps ymm1, yword [rax]

rsqrtss xmm1, xmm2
rsqrtss xmm1, [rax]
rsqrtss xmm1, dword [rax]
vrsqrtss xmm1, xmm2
vrsqrtss xmm1, [rax]
vrsqrtss xmm1, dword [rax]
vrsqrtss xmm1, xmm2, xmm3
vrsqrtss xmm1, xmm2, [rax]
vrsqrtss xmm1, xmm2, dword [rax]

roundpd xmm1, xmm2, 5
roundpd xmm1, [rax], byte 5
roundpd xmm1, dqword [rax], 5
vroundpd xmm1, xmm2, 5
vroundpd xmm1, [rax], byte 5
vroundpd xmm1, dqword [rax], 5
vroundpd ymm1, ymm2, 5
vroundpd ymm1, [rax], byte 5
vroundpd ymm1, yword [rax], 5

roundps xmm1, xmm2, 5
roundps xmm1, [rax], byte 5
roundps xmm1, dqword [rax], 5
vroundps xmm1, xmm2, 5
vroundps xmm1, [rax], byte 5
vroundps xmm1, dqword [rax], 5
vroundps ymm1, ymm2, 5
vroundps ymm1, [rax], byte 5
vroundps ymm1, yword [rax], 5

roundsd xmm1, xmm2, 5
roundsd xmm1, [rax], byte 5
roundsd xmm1, qword [rax], 5
vroundsd xmm1, xmm2, 5
vroundsd xmm1, [rax], byte 5
vroundsd xmm1, qword [rax], 5
vroundsd xmm1, xmm2, xmm3, 5
vroundsd xmm1, xmm2, [rax], byte 5
vroundsd xmm1, xmm2, qword [rax], 5

roundss xmm1, xmm2, 5
roundss xmm1, [rax], byte 5
roundss xmm1, dword [rax], 5
vroundss xmm1, xmm2, 5
vroundss xmm1, [rax], byte 5
vroundss xmm1, dword [rax], 5
vroundss xmm1, xmm2, xmm3, 5
vroundss xmm1, xmm2, [rax], byte 5
vroundss xmm1, xmm2, dword [rax], 5

shufpd xmm1, xmm2, 5
shufpd xmm1, [rax], byte 5
shufpd xmm1, dqword [rax], 5
vshufpd xmm1, xmm2, 5
vshufpd xmm1, [rax], byte 5
vshufpd xmm1, dqword [rax], 5
vshufpd xmm1, xmm2, xmm3, 5
vshufpd xmm1, xmm2, [rax], byte 5
vshufpd xmm1, xmm2, dqword [rax], 5
vshufpd ymm1, ymm2, ymm3, 5
vshufpd ymm1, ymm2, [rax], byte 5
vshufpd ymm1, ymm2, yword [rax], 5

shufps xmm1, xmm2, 5
shufps xmm1, [rax], byte 5
shufps xmm1, dqword [rax], 5
vshufps xmm1, xmm2, 5
vshufps xmm1, [rax], byte 5
vshufps xmm1, dqword [rax], 5
vshufps xmm1, xmm2, xmm3, 5
vshufps xmm1, xmm2, [rax], byte 5
vshufps xmm1, xmm2, dqword [rax], 5
vshufps ymm1, ymm2, ymm3, 5
vshufps ymm1, ymm2, [rax], byte 5
vshufps ymm1, ymm2, yword [rax], 5

sqrtpd xmm1, xmm2
sqrtpd xmm1, [rax]
sqrtpd xmm1, dqword [rax]
vsqrtpd xmm1, xmm2
vsqrtpd xmm1, [rax]
vsqrtpd xmm1, dqword [rax]
vsqrtpd ymm1, ymm2
vsqrtpd ymm1, [rax]
vsqrtpd ymm1, yword [rax]

sqrtps xmm1, xmm2
sqrtps xmm1, [rax]
sqrtps xmm1, dqword [rax]
vsqrtps xmm1, xmm2
vsqrtps xmm1, [rax]
vsqrtps xmm1, dqword [rax]
vsqrtps ymm1, ymm2
vsqrtps ymm1, [rax]
vsqrtps ymm1, yword [rax]

sqrtsd xmm1, xmm2
sqrtsd xmm1, [rax]
sqrtsd xmm1, qword [rax]
vsqrtsd xmm1, xmm2
vsqrtsd xmm1, [rax]
vsqrtsd xmm1, qword [rax]
vsqrtsd xmm1, xmm2, xmm3
vsqrtsd xmm1, xmm2, [rax]
vsqrtsd xmm1, xmm2, qword [rax]

sqrtss xmm1, xmm2
sqrtss xmm1, [rax]
sqrtss xmm1, dword [rax]
vsqrtss xmm1, xmm2
vsqrtss xmm1, [rax]
vsqrtss xmm1, dword [rax]
vsqrtss xmm1, xmm2, xmm3
vsqrtss xmm1, xmm2, [rax]
vsqrtss xmm1, xmm2, dword [rax]

stmxcsr [rax]
stmxcsr dword [rax]
vstmxcsr [rax]
vstmxcsr dword [rax]

subpd xmm1, xmm2
subpd xmm1, [rax]
subpd xmm1, dqword [rax]
vsubpd xmm1, xmm2
vsubpd xmm1, [rax]
vsubpd xmm1, dqword [rax]
vsubpd xmm1, xmm2, xmm3
vsubpd xmm1, xmm2, [rax]
vsubpd xmm1, xmm2, dqword [rax]
vsubpd ymm1, ymm2, ymm3
vsubpd ymm1, ymm2, [rax]
vsubpd ymm1, ymm2, yword [rax]

subps xmm1, xmm2
subps xmm1, [rax]
subps xmm1, dqword [rax]
vsubps xmm1, xmm2
vsubps xmm1, [rax]
vsubps xmm1, dqword [rax]
vsubps xmm1, xmm2, xmm3
vsubps xmm1, xmm2, [rax]
vsubps xmm1, xmm2, dqword [rax]
vsubps ymm1, ymm2, ymm3
vsubps ymm1, ymm2, [rax]
vsubps ymm1, ymm2, yword [rax]

subsd xmm1, xmm2
subsd xmm1, [rax]
subsd xmm1, qword [rax]
vsubsd xmm1, xmm2
vsubsd xmm1, [rax]
vsubsd xmm1, qword [rax]
vsubsd xmm1, xmm2, xmm3
vsubsd xmm1, xmm2, [rax]
vsubsd xmm1, xmm2, qword [rax]

subss xmm1, xmm2
subss xmm1, [rax]
subss xmm1, dword [rax]
vsubss xmm1, xmm2
vsubss xmm1, [rax]
vsubss xmm1, dword [rax]
vsubss xmm1, xmm2, xmm3
vsubss xmm1, xmm2, [rax]
vsubss xmm1, xmm2, dword [rax]

ucomisd xmm1, xmm2
ucomisd xmm1, [rax]
ucomisd xmm1, qword [rax]
vucomisd xmm1, xmm2
vucomisd xmm1, [rax]
vucomisd xmm1, qword [rax]

ucomiss xmm1, xmm2
ucomiss xmm1, [rax]
ucomiss xmm1, dword [rax]
vucomiss xmm1, xmm2
vucomiss xmm1, [rax]
vucomiss xmm1, dword [rax]

unpckhpd xmm1, xmm2
unpckhpd xmm1, [rax]
unpckhpd xmm1, dqword [rax]
vunpckhpd xmm1, xmm2
vunpckhpd xmm1, [rax]
vunpckhpd xmm1, dqword [rax]
vunpckhpd xmm1, xmm2, xmm3
vunpckhpd xmm1, xmm2, [rax]
vunpckhpd xmm1, xmm2, dqword [rax]
vunpckhpd ymm1, ymm2, ymm3
vunpckhpd ymm1, ymm2, [rax]
vunpckhpd ymm1, ymm2, yword [rax]

unpckhps xmm1, xmm2
unpckhps xmm1, [rax]
unpckhps xmm1, dqword [rax]
vunpckhps xmm1, xmm2
vunpckhps xmm1, [rax]
vunpckhps xmm1, dqword [rax]
vunpckhps xmm1, xmm2, xmm3
vunpckhps xmm1, xmm2, [rax]
vunpckhps xmm1, xmm2, dqword [rax]
vunpckhps ymm1, ymm2, ymm3
vunpckhps ymm1, ymm2, [rax]
vunpckhps ymm1, ymm2, yword [rax]

unpcklpd xmm1, xmm2
unpcklpd xmm1, [rax]
unpcklpd xmm1, dqword [rax]
vunpcklpd xmm1, xmm2
vunpcklpd xmm1, [rax]
vunpcklpd xmm1, dqword [rax]
vunpcklpd xmm1, xmm2, xmm3
vunpcklpd xmm1, xmm2, [rax]
vunpcklpd xmm1, xmm2, dqword [rax]
vunpcklpd ymm1, ymm2, ymm3
vunpcklpd ymm1, ymm2, [rax]
vunpcklpd ymm1, ymm2, yword [rax]

unpcklps xmm1, xmm2
unpcklps xmm1, [rax]
unpcklps xmm1, dqword [rax]
vunpcklps xmm1, xmm2
vunpcklps xmm1, [rax]
vunpcklps xmm1, dqword [rax]
vunpcklps xmm1, xmm2, xmm3
vunpcklps xmm1, xmm2, [rax]
vunpcklps xmm1, xmm2, dqword [rax]
vunpcklps ymm1, ymm2, ymm3
vunpcklps ymm1, ymm2, [rax]
vunpcklps ymm1, ymm2, yword [rax]

xorpd xmm1, xmm2
xorpd xmm1, [rax]
xorpd xmm1, dqword [rax]
vxorpd xmm1, xmm2
vxorpd xmm1, [rax]
vxorpd xmm1, dqword [rax]
vxorpd xmm1, xmm2, xmm3
vxorpd xmm1, xmm2, [rax]
vxorpd xmm1, xmm2, dqword [rax]
vxorpd ymm1, ymm2, ymm3
vxorpd ymm1, ymm2, [rax]
vxorpd ymm1, ymm2, yword [rax]

xorps xmm1, xmm2
xorps xmm1, [rax]
xorps xmm1, dqword [rax]
vxorps xmm1, xmm2
vxorps xmm1, [rax]
vxorps xmm1, dqword [rax]
vxorps xmm1, xmm2, xmm3
vxorps xmm1, xmm2, [rax]
vxorps xmm1, xmm2, dqword [rax]
vxorps ymm1, ymm2, ymm3
vxorps ymm1, ymm2, [rax]
vxorps ymm1, ymm2, yword [rax]

vzeroall

vzeroupper

