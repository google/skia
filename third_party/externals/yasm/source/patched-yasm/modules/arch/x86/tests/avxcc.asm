; Exhaustive test of AVX condition code aliases
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

cmpeqpd xmm1, xmm2			; 00h
cmpltpd xmm1, xmm2			; 01h
cmplepd xmm1, xmm2			; 02h
cmpunordpd xmm1, xmm2			; 03h
cmpneqpd xmm1, xmm2			; 04h
cmpnltpd xmm1, xmm2			; 05h
cmpnlepd xmm1, xmm2			; 06h
cmpordpd xmm1, xmm2			; 07h

vcmpeqpd xmm1, xmm2			; 00h
vcmpltpd xmm1, xmm2			; 01h
vcmplepd xmm1, xmm2			; 02h
vcmpunordpd xmm1, xmm2			; 03h
vcmpneqpd xmm1, xmm2			; 04h
vcmpnltpd xmm1, xmm2			; 05h
vcmpnlepd xmm1, xmm2			; 06h
vcmpordpd xmm1, xmm2			; 07h

vcmpeqpd xmm1, xmm2, xmm3		; 00h
vcmpltpd xmm1, xmm2, xmm3		; 01h
vcmplepd xmm1, xmm2, xmm3		; 02h
vcmpunordpd xmm1, xmm2, xmm3		; 03h
vcmpneqpd xmm1, xmm2, xmm3		; 04h
vcmpnltpd xmm1, xmm2, xmm3		; 05h
vcmpnlepd xmm1, xmm2, xmm3		; 06h
vcmpordpd xmm1, xmm2, xmm3		; 07h

vcmpeq_uqpd xmm1, xmm2, xmm3		; 08h
vcmpngepd xmm1, xmm2, xmm3		; 09h
vcmpngtpd xmm1, xmm2, xmm3		; 0Ah
vcmpfalsepd xmm1, xmm2, xmm3		; 0Bh
vcmpneq_oqpd xmm1, xmm2, xmm3		; 0Ch
vcmpgepd xmm1, xmm2, xmm3		; 0Dh
vcmpgtpd xmm1, xmm2, xmm3		; 0Eh
vcmptruepd xmm1, xmm2, xmm3		; 0Fh

vcmpeq_ospd xmm1, xmm2, xmm3		; 10h
vcmplt_oqpd xmm1, xmm2, xmm3		; 11h
vcmple_oqpd xmm1, xmm2, xmm3		; 12h
vcmpunord_spd xmm1, xmm2, xmm3		; 13h
vcmpneq_uspd xmm1, xmm2, xmm3		; 14h
vcmpnlt_uqpd xmm1, xmm2, xmm3		; 15h
vcmpnle_uqpd xmm1, xmm2, xmm3		; 16h
vcmpord_spd xmm1, xmm2, xmm3		; 17h

vcmpeq_uspd xmm1, xmm2, xmm3		; 18h
vcmpnge_uqpd xmm1, xmm2, xmm3		; 19h
vcmpngt_uqpd xmm1, xmm2, xmm3		; 1Ah
vcmpfalse_ospd xmm1, xmm2, xmm3		; 1Bh
vcmpneq_ospd xmm1, xmm2, xmm3		; 1Ch
vcmpge_oqpd xmm1, xmm2, xmm3		; 1Dh
vcmpgt_oqpd xmm1, xmm2, xmm3		; 1Eh
vcmptrue_uspd xmm1, xmm2, xmm3		; 1Fh

cmpeqpd xmm1, [rax]			; 00h
cmpltpd xmm1, [rax]			; 01h
cmplepd xmm1, [rax]			; 02h
cmpunordpd xmm1, [rax]			; 03h
cmpneqpd xmm1, [rax]			; 04h
cmpnltpd xmm1, [rax]			; 05h
cmpnlepd xmm1, [rax]			; 06h
cmpordpd xmm1, [rax]			; 07h

vcmpeqpd xmm1, [rax]			; 00h
vcmpltpd xmm1, [rax]			; 01h
vcmplepd xmm1, [rax]			; 02h
vcmpunordpd xmm1, [rax]			; 03h
vcmpneqpd xmm1, [rax]			; 04h
vcmpnltpd xmm1, [rax]			; 05h
vcmpnlepd xmm1, [rax]			; 06h
vcmpordpd xmm1, [rax]			; 07h

vcmpeqpd xmm1, xmm2, [rax]		; 00h
vcmpltpd xmm1, xmm2, [rax]		; 01h
vcmplepd xmm1, xmm2, [rax]		; 02h
vcmpunordpd xmm1, xmm2, [rax]		; 03h
vcmpneqpd xmm1, xmm2, [rax]		; 04h
vcmpnltpd xmm1, xmm2, [rax]		; 05h
vcmpnlepd xmm1, xmm2, [rax]		; 06h
vcmpordpd xmm1, xmm2, [rax]		; 07h

vcmpeq_uqpd xmm1, xmm2, [rax]		; 08h
vcmpngepd xmm1, xmm2, [rax]		; 09h
vcmpngtpd xmm1, xmm2, [rax]		; 0Ah
vcmpfalsepd xmm1, xmm2, [rax]		; 0Bh
vcmpneq_oqpd xmm1, xmm2, [rax]		; 0Ch
vcmpgepd xmm1, xmm2, [rax]		; 0Dh
vcmpgtpd xmm1, xmm2, [rax]		; 0Eh
vcmptruepd xmm1, xmm2, [rax]		; 0Fh

vcmpeq_ospd xmm1, xmm2, [rax]		; 10h
vcmplt_oqpd xmm1, xmm2, [rax]		; 11h
vcmple_oqpd xmm1, xmm2, [rax]		; 12h
vcmpunord_spd xmm1, xmm2, [rax]		; 13h
vcmpneq_uspd xmm1, xmm2, [rax]		; 14h
vcmpnlt_uqpd xmm1, xmm2, [rax]		; 15h
vcmpnle_uqpd xmm1, xmm2, [rax]		; 16h
vcmpord_spd xmm1, xmm2, [rax]		; 17h

vcmpeq_uspd xmm1, xmm2, [rax]		; 18h
vcmpnge_uqpd xmm1, xmm2, [rax]		; 19h
vcmpngt_uqpd xmm1, xmm2, [rax]		; 1Ah
vcmpfalse_ospd xmm1, xmm2, [rax]	; 1Bh
vcmpneq_ospd xmm1, xmm2, [rax]		; 1Ch
vcmpge_oqpd xmm1, xmm2, [rax]		; 1Dh
vcmpgt_oqpd xmm1, xmm2, [rax]		; 1Eh
vcmptrue_uspd xmm1, xmm2, [rax]		; 1Fh

cmpeqpd xmm1, dqword [rax]			; 00h
cmpltpd xmm1, dqword [rax]			; 01h
cmplepd xmm1, dqword [rax]			; 02h
cmpunordpd xmm1, dqword [rax]			; 03h
cmpneqpd xmm1, dqword [rax]			; 04h
cmpnltpd xmm1, dqword [rax]			; 05h
cmpnlepd xmm1, dqword [rax]			; 06h
cmpordpd xmm1, dqword [rax]			; 07h

vcmpeqpd xmm1, dqword [rax]			; 00h
vcmpltpd xmm1, dqword [rax]			; 01h
vcmplepd xmm1, dqword [rax]			; 02h
vcmpunordpd xmm1, dqword [rax]			; 03h
vcmpneqpd xmm1, dqword [rax]			; 04h
vcmpnltpd xmm1, dqword [rax]			; 05h
vcmpnlepd xmm1, dqword [rax]			; 06h
vcmpordpd xmm1, dqword [rax]			; 07h

vcmpeqpd xmm1, xmm2, dqword [rax]		; 00h
vcmpltpd xmm1, xmm2, dqword [rax]		; 01h
vcmplepd xmm1, xmm2, dqword [rax]		; 02h
vcmpunordpd xmm1, xmm2, dqword [rax]		; 03h
vcmpneqpd xmm1, xmm2, dqword [rax]		; 04h
vcmpnltpd xmm1, xmm2, dqword [rax]		; 05h
vcmpnlepd xmm1, xmm2, dqword [rax]		; 06h
vcmpordpd xmm1, xmm2, dqword [rax]		; 07h

vcmpeq_uqpd xmm1, xmm2, dqword [rax]		; 08h
vcmpngepd xmm1, xmm2, dqword [rax]		; 09h
vcmpngtpd xmm1, xmm2, dqword [rax]		; 0Ah
vcmpfalsepd xmm1, xmm2, dqword [rax]		; 0Bh
vcmpneq_oqpd xmm1, xmm2, dqword [rax]		; 0Ch
vcmpgepd xmm1, xmm2, dqword [rax]		; 0Dh
vcmpgtpd xmm1, xmm2, dqword [rax]		; 0Eh
vcmptruepd xmm1, xmm2, dqword [rax]		; 0Fh

vcmpeq_ospd xmm1, xmm2, dqword [rax]		; 10h
vcmplt_oqpd xmm1, xmm2, dqword [rax]		; 11h
vcmple_oqpd xmm1, xmm2, dqword [rax]		; 12h
vcmpunord_spd xmm1, xmm2, dqword [rax]		; 13h
vcmpneq_uspd xmm1, xmm2, dqword [rax]		; 14h
vcmpnlt_uqpd xmm1, xmm2, dqword [rax]		; 15h
vcmpnle_uqpd xmm1, xmm2, dqword [rax]		; 16h
vcmpord_spd xmm1, xmm2, dqword [rax]		; 17h

vcmpeq_uspd xmm1, xmm2, dqword [rax]		; 18h
vcmpnge_uqpd xmm1, xmm2, dqword [rax]		; 19h
vcmpngt_uqpd xmm1, xmm2, dqword [rax]		; 1Ah
vcmpfalse_ospd xmm1, xmm2, dqword [rax]		; 1Bh
vcmpneq_ospd xmm1, xmm2, dqword [rax]		; 1Ch
vcmpge_oqpd xmm1, xmm2, dqword [rax]		; 1Dh
vcmpgt_oqpd xmm1, xmm2, dqword [rax]		; 1Eh
vcmptrue_uspd xmm1, xmm2, dqword [rax]		; 1Fh

vcmpeqpd ymm1, ymm2, ymm3		; 00h
vcmpltpd ymm1, ymm2, ymm3		; 01h
vcmplepd ymm1, ymm2, ymm3		; 02h
vcmpunordpd ymm1, ymm2, ymm3		; 03h
vcmpneqpd ymm1, ymm2, ymm3		; 04h
vcmpnltpd ymm1, ymm2, ymm3		; 05h
vcmpnlepd ymm1, ymm2, ymm3		; 06h
vcmpordpd ymm1, ymm2, ymm3		; 07h

vcmpeq_uqpd ymm1, ymm2, ymm3		; 08h
vcmpngepd ymm1, ymm2, ymm3		; 09h
vcmpngtpd ymm1, ymm2, ymm3		; 0Ah
vcmpfalsepd ymm1, ymm2, ymm3		; 0Bh
vcmpneq_oqpd ymm1, ymm2, ymm3		; 0Ch
vcmpgepd ymm1, ymm2, ymm3		; 0Dh
vcmpgtpd ymm1, ymm2, ymm3		; 0Eh
vcmptruepd ymm1, ymm2, ymm3		; 0Fh

vcmpeq_ospd ymm1, ymm2, ymm3		; 10h
vcmplt_oqpd ymm1, ymm2, ymm3		; 11h
vcmple_oqpd ymm1, ymm2, ymm3		; 12h
vcmpunord_spd ymm1, ymm2, ymm3		; 13h
vcmpneq_uspd ymm1, ymm2, ymm3		; 14h
vcmpnlt_uqpd ymm1, ymm2, ymm3		; 15h
vcmpnle_uqpd ymm1, ymm2, ymm3		; 16h
vcmpord_spd ymm1, ymm2, ymm3		; 17h

vcmpeq_uspd ymm1, ymm2, ymm3		; 18h
vcmpnge_uqpd ymm1, ymm2, ymm3		; 19h
vcmpngt_uqpd ymm1, ymm2, ymm3		; 1Ah
vcmpfalse_ospd ymm1, ymm2, ymm3		; 1Bh
vcmpneq_ospd ymm1, ymm2, ymm3		; 1Ch
vcmpge_oqpd ymm1, ymm2, ymm3		; 1Dh
vcmpgt_oqpd ymm1, ymm2, ymm3		; 1Eh
vcmptrue_uspd ymm1, ymm2, ymm3		; 1Fh

vcmpeqpd ymm1, ymm2, [rax]		; 00h
vcmpltpd ymm1, ymm2, [rax]		; 01h
vcmplepd ymm1, ymm2, [rax]		; 02h
vcmpunordpd ymm1, ymm2, [rax]		; 03h
vcmpneqpd ymm1, ymm2, [rax]		; 04h
vcmpnltpd ymm1, ymm2, [rax]		; 05h
vcmpnlepd ymm1, ymm2, [rax]		; 06h
vcmpordpd ymm1, ymm2, [rax]		; 07h

vcmpeq_uqpd ymm1, ymm2, [rax]		; 08h
vcmpngepd ymm1, ymm2, [rax]		; 09h
vcmpngtpd ymm1, ymm2, [rax]		; 0Ah
vcmpfalsepd ymm1, ymm2, [rax]		; 0Bh
vcmpneq_oqpd ymm1, ymm2, [rax]		; 0Ch
vcmpgepd ymm1, ymm2, [rax]		; 0Dh
vcmpgtpd ymm1, ymm2, [rax]		; 0Eh
vcmptruepd ymm1, ymm2, [rax]		; 0Fh

vcmpeq_ospd ymm1, ymm2, [rax]		; 10h
vcmplt_oqpd ymm1, ymm2, [rax]		; 11h
vcmple_oqpd ymm1, ymm2, [rax]		; 12h
vcmpunord_spd ymm1, ymm2, [rax]		; 13h
vcmpneq_uspd ymm1, ymm2, [rax]		; 14h
vcmpnlt_uqpd ymm1, ymm2, [rax]		; 15h
vcmpnle_uqpd ymm1, ymm2, [rax]		; 16h
vcmpord_spd ymm1, ymm2, [rax]		; 17h

vcmpeq_uspd ymm1, ymm2, [rax]		; 18h
vcmpnge_uqpd ymm1, ymm2, [rax]		; 19h
vcmpngt_uqpd ymm1, ymm2, [rax]		; 1Ah
vcmpfalse_ospd ymm1, ymm2, [rax]	; 1Bh
vcmpneq_ospd ymm1, ymm2, [rax]		; 1Ch
vcmpge_oqpd ymm1, ymm2, [rax]		; 1Dh
vcmpgt_oqpd ymm1, ymm2, [rax]		; 1Eh
vcmptrue_uspd ymm1, ymm2, [rax]		; 1Fh

vcmpeqpd ymm1, ymm2, yword [rax]		; 00h
vcmpltpd ymm1, ymm2, yword [rax]		; 01h
vcmplepd ymm1, ymm2, yword [rax]		; 02h
vcmpunordpd ymm1, ymm2, yword [rax]		; 03h
vcmpneqpd ymm1, ymm2, yword [rax]		; 04h
vcmpnltpd ymm1, ymm2, yword [rax]		; 05h
vcmpnlepd ymm1, ymm2, yword [rax]		; 06h
vcmpordpd ymm1, ymm2, yword [rax]		; 07h

vcmpeq_uqpd ymm1, ymm2, yword [rax]		; 08h
vcmpngepd ymm1, ymm2, yword [rax]		; 09h
vcmpngtpd ymm1, ymm2, yword [rax]		; 0Ah
vcmpfalsepd ymm1, ymm2, yword [rax]		; 0Bh
vcmpneq_oqpd ymm1, ymm2, yword [rax]		; 0Ch
vcmpgepd ymm1, ymm2, yword [rax]		; 0Dh
vcmpgtpd ymm1, ymm2, yword [rax]		; 0Eh
vcmptruepd ymm1, ymm2, yword [rax]		; 0Fh

vcmpeq_ospd ymm1, ymm2, yword [rax]		; 10h
vcmplt_oqpd ymm1, ymm2, yword [rax]		; 11h
vcmple_oqpd ymm1, ymm2, yword [rax]		; 12h
vcmpunord_spd ymm1, ymm2, yword [rax]		; 13h
vcmpneq_uspd ymm1, ymm2, yword [rax]		; 14h
vcmpnlt_uqpd ymm1, ymm2, yword [rax]		; 15h
vcmpnle_uqpd ymm1, ymm2, yword [rax]		; 16h
vcmpord_spd ymm1, ymm2, yword [rax]		; 17h

vcmpeq_uspd ymm1, ymm2, yword [rax]		; 18h
vcmpnge_uqpd ymm1, ymm2, yword [rax]		; 19h
vcmpngt_uqpd ymm1, ymm2, yword [rax]		; 1Ah
vcmpfalse_ospd ymm1, ymm2, yword [rax]		; 1Bh
vcmpneq_ospd ymm1, ymm2, yword [rax]		; 1Ch
vcmpge_oqpd ymm1, ymm2, yword [rax]		; 1Dh
vcmpgt_oqpd ymm1, ymm2, yword [rax]		; 1Eh
vcmptrue_uspd ymm1, ymm2, yword [rax]		; 1Fh

;-----------------------------------------------------------------------------

cmpeqps xmm1, xmm2			; 00h
cmpltps xmm1, xmm2			; 01h
cmpleps xmm1, xmm2			; 02h
cmpunordps xmm1, xmm2			; 03h
cmpneqps xmm1, xmm2			; 04h
cmpnltps xmm1, xmm2			; 05h
cmpnleps xmm1, xmm2			; 06h
cmpordps xmm1, xmm2			; 07h

vcmpeqps xmm1, xmm2			; 00h
vcmpltps xmm1, xmm2			; 01h
vcmpleps xmm1, xmm2			; 02h
vcmpunordps xmm1, xmm2			; 03h
vcmpneqps xmm1, xmm2			; 04h
vcmpnltps xmm1, xmm2			; 05h
vcmpnleps xmm1, xmm2			; 06h
vcmpordps xmm1, xmm2			; 07h

vcmpeqps xmm1, xmm2, xmm3		; 00h
vcmpltps xmm1, xmm2, xmm3		; 01h
vcmpleps xmm1, xmm2, xmm3		; 02h
vcmpunordps xmm1, xmm2, xmm3		; 03h
vcmpneqps xmm1, xmm2, xmm3		; 04h
vcmpnltps xmm1, xmm2, xmm3		; 05h
vcmpnleps xmm1, xmm2, xmm3		; 06h
vcmpordps xmm1, xmm2, xmm3		; 07h

vcmpeq_uqps xmm1, xmm2, xmm3		; 08h
vcmpngeps xmm1, xmm2, xmm3		; 09h
vcmpngtps xmm1, xmm2, xmm3		; 0Ah
vcmpfalseps xmm1, xmm2, xmm3		; 0Bh
vcmpneq_oqps xmm1, xmm2, xmm3		; 0Ch
vcmpgeps xmm1, xmm2, xmm3		; 0Dh
vcmpgtps xmm1, xmm2, xmm3		; 0Eh
vcmptrueps xmm1, xmm2, xmm3		; 0Fh

vcmpeq_osps xmm1, xmm2, xmm3		; 10h
vcmplt_oqps xmm1, xmm2, xmm3		; 11h
vcmple_oqps xmm1, xmm2, xmm3		; 12h
vcmpunord_sps xmm1, xmm2, xmm3		; 13h
vcmpneq_usps xmm1, xmm2, xmm3		; 14h
vcmpnlt_uqps xmm1, xmm2, xmm3		; 15h
vcmpnle_uqps xmm1, xmm2, xmm3		; 16h
vcmpord_sps xmm1, xmm2, xmm3		; 17h

vcmpeq_usps xmm1, xmm2, xmm3		; 18h
vcmpnge_uqps xmm1, xmm2, xmm3		; 19h
vcmpngt_uqps xmm1, xmm2, xmm3		; 1Ah
vcmpfalse_osps xmm1, xmm2, xmm3		; 1Bh
vcmpneq_osps xmm1, xmm2, xmm3		; 1Ch
vcmpge_oqps xmm1, xmm2, xmm3		; 1Dh
vcmpgt_oqps xmm1, xmm2, xmm3		; 1Eh
vcmptrue_usps xmm1, xmm2, xmm3		; 1Fh

cmpeqps xmm1, [rax]			; 00h
cmpltps xmm1, [rax]			; 01h
cmpleps xmm1, [rax]			; 02h
cmpunordps xmm1, [rax]			; 03h
cmpneqps xmm1, [rax]			; 04h
cmpnltps xmm1, [rax]			; 05h
cmpnleps xmm1, [rax]			; 06h
cmpordps xmm1, [rax]			; 07h

vcmpeqps xmm1, [rax]			; 00h
vcmpltps xmm1, [rax]			; 01h
vcmpleps xmm1, [rax]			; 02h
vcmpunordps xmm1, [rax]			; 03h
vcmpneqps xmm1, [rax]			; 04h
vcmpnltps xmm1, [rax]			; 05h
vcmpnleps xmm1, [rax]			; 06h
vcmpordps xmm1, [rax]			; 07h

vcmpeqps xmm1, xmm2, [rax]		; 00h
vcmpltps xmm1, xmm2, [rax]		; 01h
vcmpleps xmm1, xmm2, [rax]		; 02h
vcmpunordps xmm1, xmm2, [rax]		; 03h
vcmpneqps xmm1, xmm2, [rax]		; 04h
vcmpnltps xmm1, xmm2, [rax]		; 05h
vcmpnleps xmm1, xmm2, [rax]		; 06h
vcmpordps xmm1, xmm2, [rax]		; 07h

vcmpeq_uqps xmm1, xmm2, [rax]		; 08h
vcmpngeps xmm1, xmm2, [rax]		; 09h
vcmpngtps xmm1, xmm2, [rax]		; 0Ah
vcmpfalseps xmm1, xmm2, [rax]		; 0Bh
vcmpneq_oqps xmm1, xmm2, [rax]		; 0Ch
vcmpgeps xmm1, xmm2, [rax]		; 0Dh
vcmpgtps xmm1, xmm2, [rax]		; 0Eh
vcmptrueps xmm1, xmm2, [rax]		; 0Fh

vcmpeq_osps xmm1, xmm2, [rax]		; 10h
vcmplt_oqps xmm1, xmm2, [rax]		; 11h
vcmple_oqps xmm1, xmm2, [rax]		; 12h
vcmpunord_sps xmm1, xmm2, [rax]		; 13h
vcmpneq_usps xmm1, xmm2, [rax]		; 14h
vcmpnlt_uqps xmm1, xmm2, [rax]		; 15h
vcmpnle_uqps xmm1, xmm2, [rax]		; 16h
vcmpord_sps xmm1, xmm2, [rax]		; 17h

vcmpeq_usps xmm1, xmm2, [rax]		; 18h
vcmpnge_uqps xmm1, xmm2, [rax]		; 19h
vcmpngt_uqps xmm1, xmm2, [rax]		; 1Ah
vcmpfalse_osps xmm1, xmm2, [rax]	; 1Bh
vcmpneq_osps xmm1, xmm2, [rax]		; 1Ch
vcmpge_oqps xmm1, xmm2, [rax]		; 1Dh
vcmpgt_oqps xmm1, xmm2, [rax]		; 1Eh
vcmptrue_usps xmm1, xmm2, [rax]		; 1Fh

cmpeqps xmm1, dqword [rax]			; 00h
cmpltps xmm1, dqword [rax]			; 01h
cmpleps xmm1, dqword [rax]			; 02h
cmpunordps xmm1, dqword [rax]			; 03h
cmpneqps xmm1, dqword [rax]			; 04h
cmpnltps xmm1, dqword [rax]			; 05h
cmpnleps xmm1, dqword [rax]			; 06h
cmpordps xmm1, dqword [rax]			; 07h

vcmpeqps xmm1, dqword [rax]			; 00h
vcmpltps xmm1, dqword [rax]			; 01h
vcmpleps xmm1, dqword [rax]			; 02h
vcmpunordps xmm1, dqword [rax]			; 03h
vcmpneqps xmm1, dqword [rax]			; 04h
vcmpnltps xmm1, dqword [rax]			; 05h
vcmpnleps xmm1, dqword [rax]			; 06h
vcmpordps xmm1, dqword [rax]			; 07h

vcmpeqps xmm1, xmm2, dqword [rax]		; 00h
vcmpltps xmm1, xmm2, dqword [rax]		; 01h
vcmpleps xmm1, xmm2, dqword [rax]		; 02h
vcmpunordps xmm1, xmm2, dqword [rax]		; 03h
vcmpneqps xmm1, xmm2, dqword [rax]		; 04h
vcmpnltps xmm1, xmm2, dqword [rax]		; 05h
vcmpnleps xmm1, xmm2, dqword [rax]		; 06h
vcmpordps xmm1, xmm2, dqword [rax]		; 07h

vcmpeq_uqps xmm1, xmm2, dqword [rax]		; 08h
vcmpngeps xmm1, xmm2, dqword [rax]		; 09h
vcmpngtps xmm1, xmm2, dqword [rax]		; 0Ah
vcmpfalseps xmm1, xmm2, dqword [rax]		; 0Bh
vcmpneq_oqps xmm1, xmm2, dqword [rax]		; 0Ch
vcmpgeps xmm1, xmm2, dqword [rax]		; 0Dh
vcmpgtps xmm1, xmm2, dqword [rax]		; 0Eh
vcmptrueps xmm1, xmm2, dqword [rax]		; 0Fh

vcmpeq_osps xmm1, xmm2, dqword [rax]		; 10h
vcmplt_oqps xmm1, xmm2, dqword [rax]		; 11h
vcmple_oqps xmm1, xmm2, dqword [rax]		; 12h
vcmpunord_sps xmm1, xmm2, dqword [rax]		; 13h
vcmpneq_usps xmm1, xmm2, dqword [rax]		; 14h
vcmpnlt_uqps xmm1, xmm2, dqword [rax]		; 15h
vcmpnle_uqps xmm1, xmm2, dqword [rax]		; 16h
vcmpord_sps xmm1, xmm2, dqword [rax]		; 17h

vcmpeq_usps xmm1, xmm2, dqword [rax]		; 18h
vcmpnge_uqps xmm1, xmm2, dqword [rax]		; 19h
vcmpngt_uqps xmm1, xmm2, dqword [rax]		; 1Ah
vcmpfalse_osps xmm1, xmm2, dqword [rax]		; 1Bh
vcmpneq_osps xmm1, xmm2, dqword [rax]		; 1Ch
vcmpge_oqps xmm1, xmm2, dqword [rax]		; 1Dh
vcmpgt_oqps xmm1, xmm2, dqword [rax]		; 1Eh
vcmptrue_usps xmm1, xmm2, dqword [rax]		; 1Fh

vcmpeqps ymm1, ymm2, ymm3		; 00h
vcmpltps ymm1, ymm2, ymm3		; 01h
vcmpleps ymm1, ymm2, ymm3		; 02h
vcmpunordps ymm1, ymm2, ymm3		; 03h
vcmpneqps ymm1, ymm2, ymm3		; 04h
vcmpnltps ymm1, ymm2, ymm3		; 05h
vcmpnleps ymm1, ymm2, ymm3		; 06h
vcmpordps ymm1, ymm2, ymm3		; 07h

vcmpeq_uqps ymm1, ymm2, ymm3		; 08h
vcmpngeps ymm1, ymm2, ymm3		; 09h
vcmpngtps ymm1, ymm2, ymm3		; 0Ah
vcmpfalseps ymm1, ymm2, ymm3		; 0Bh
vcmpneq_oqps ymm1, ymm2, ymm3		; 0Ch
vcmpgeps ymm1, ymm2, ymm3		; 0Dh
vcmpgtps ymm1, ymm2, ymm3		; 0Eh
vcmptrueps ymm1, ymm2, ymm3		; 0Fh

vcmpeq_osps ymm1, ymm2, ymm3		; 10h
vcmplt_oqps ymm1, ymm2, ymm3		; 11h
vcmple_oqps ymm1, ymm2, ymm3		; 12h
vcmpunord_sps ymm1, ymm2, ymm3		; 13h
vcmpneq_usps ymm1, ymm2, ymm3		; 14h
vcmpnlt_uqps ymm1, ymm2, ymm3		; 15h
vcmpnle_uqps ymm1, ymm2, ymm3		; 16h
vcmpord_sps ymm1, ymm2, ymm3		; 17h

vcmpeq_usps ymm1, ymm2, ymm3		; 18h
vcmpnge_uqps ymm1, ymm2, ymm3		; 19h
vcmpngt_uqps ymm1, ymm2, ymm3		; 1Ah
vcmpfalse_osps ymm1, ymm2, ymm3		; 1Bh
vcmpneq_osps ymm1, ymm2, ymm3		; 1Ch
vcmpge_oqps ymm1, ymm2, ymm3		; 1Dh
vcmpgt_oqps ymm1, ymm2, ymm3		; 1Eh
vcmptrue_usps ymm1, ymm2, ymm3		; 1Fh

vcmpeqps ymm1, ymm2, [rax]		; 00h
vcmpltps ymm1, ymm2, [rax]		; 01h
vcmpleps ymm1, ymm2, [rax]		; 02h
vcmpunordps ymm1, ymm2, [rax]		; 03h
vcmpneqps ymm1, ymm2, [rax]		; 04h
vcmpnltps ymm1, ymm2, [rax]		; 05h
vcmpnleps ymm1, ymm2, [rax]		; 06h
vcmpordps ymm1, ymm2, [rax]		; 07h

vcmpeq_uqps ymm1, ymm2, [rax]		; 08h
vcmpngeps ymm1, ymm2, [rax]		; 09h
vcmpngtps ymm1, ymm2, [rax]		; 0Ah
vcmpfalseps ymm1, ymm2, [rax]		; 0Bh
vcmpneq_oqps ymm1, ymm2, [rax]		; 0Ch
vcmpgeps ymm1, ymm2, [rax]		; 0Dh
vcmpgtps ymm1, ymm2, [rax]		; 0Eh
vcmptrueps ymm1, ymm2, [rax]		; 0Fh

vcmpeq_osps ymm1, ymm2, [rax]		; 10h
vcmplt_oqps ymm1, ymm2, [rax]		; 11h
vcmple_oqps ymm1, ymm2, [rax]		; 12h
vcmpunord_sps ymm1, ymm2, [rax]		; 13h
vcmpneq_usps ymm1, ymm2, [rax]		; 14h
vcmpnlt_uqps ymm1, ymm2, [rax]		; 15h
vcmpnle_uqps ymm1, ymm2, [rax]		; 16h
vcmpord_sps ymm1, ymm2, [rax]		; 17h

vcmpeq_usps ymm1, ymm2, [rax]		; 18h
vcmpnge_uqps ymm1, ymm2, [rax]		; 19h
vcmpngt_uqps ymm1, ymm2, [rax]		; 1Ah
vcmpfalse_osps ymm1, ymm2, [rax]	; 1Bh
vcmpneq_osps ymm1, ymm2, [rax]		; 1Ch
vcmpge_oqps ymm1, ymm2, [rax]		; 1Dh
vcmpgt_oqps ymm1, ymm2, [rax]		; 1Eh
vcmptrue_usps ymm1, ymm2, [rax]		; 1Fh

vcmpeqps ymm1, ymm2, yword [rax]		; 00h
vcmpltps ymm1, ymm2, yword [rax]		; 01h
vcmpleps ymm1, ymm2, yword [rax]		; 02h
vcmpunordps ymm1, ymm2, yword [rax]		; 03h
vcmpneqps ymm1, ymm2, yword [rax]		; 04h
vcmpnltps ymm1, ymm2, yword [rax]		; 05h
vcmpnleps ymm1, ymm2, yword [rax]		; 06h
vcmpordps ymm1, ymm2, yword [rax]		; 07h

vcmpeq_uqps ymm1, ymm2, yword [rax]		; 08h
vcmpngeps ymm1, ymm2, yword [rax]		; 09h
vcmpngtps ymm1, ymm2, yword [rax]		; 0Ah
vcmpfalseps ymm1, ymm2, yword [rax]		; 0Bh
vcmpneq_oqps ymm1, ymm2, yword [rax]		; 0Ch
vcmpgeps ymm1, ymm2, yword [rax]		; 0Dh
vcmpgtps ymm1, ymm2, yword [rax]		; 0Eh
vcmptrueps ymm1, ymm2, yword [rax]		; 0Fh

vcmpeq_osps ymm1, ymm2, yword [rax]		; 10h
vcmplt_oqps ymm1, ymm2, yword [rax]		; 11h
vcmple_oqps ymm1, ymm2, yword [rax]		; 12h
vcmpunord_sps ymm1, ymm2, yword [rax]		; 13h
vcmpneq_usps ymm1, ymm2, yword [rax]		; 14h
vcmpnlt_uqps ymm1, ymm2, yword [rax]		; 15h
vcmpnle_uqps ymm1, ymm2, yword [rax]		; 16h
vcmpord_sps ymm1, ymm2, yword [rax]		; 17h

vcmpeq_usps ymm1, ymm2, yword [rax]		; 18h
vcmpnge_uqps ymm1, ymm2, yword [rax]		; 19h
vcmpngt_uqps ymm1, ymm2, yword [rax]		; 1Ah
vcmpfalse_osps ymm1, ymm2, yword [rax]		; 1Bh
vcmpneq_osps ymm1, ymm2, yword [rax]		; 1Ch
vcmpge_oqps ymm1, ymm2, yword [rax]		; 1Dh
vcmpgt_oqps ymm1, ymm2, yword [rax]		; 1Eh
vcmptrue_usps ymm1, ymm2, yword [rax]		; 1Fh

;-----------------------------------------------------------------------------

cmpeqsd xmm1, xmm2			; 00h
cmpltsd xmm1, xmm2			; 01h
cmplesd xmm1, xmm2			; 02h
cmpunordsd xmm1, xmm2			; 03h
cmpneqsd xmm1, xmm2			; 04h
cmpnltsd xmm1, xmm2			; 05h
cmpnlesd xmm1, xmm2			; 06h
cmpordsd xmm1, xmm2			; 07h

vcmpeqsd xmm1, xmm2			; 00h
vcmpltsd xmm1, xmm2			; 01h
vcmplesd xmm1, xmm2			; 02h
vcmpunordsd xmm1, xmm2			; 03h
vcmpneqsd xmm1, xmm2			; 04h
vcmpnltsd xmm1, xmm2			; 05h
vcmpnlesd xmm1, xmm2			; 06h
vcmpordsd xmm1, xmm2			; 07h

vcmpeqsd xmm1, xmm2, xmm3		; 00h
vcmpltsd xmm1, xmm2, xmm3		; 01h
vcmplesd xmm1, xmm2, xmm3		; 02h
vcmpunordsd xmm1, xmm2, xmm3		; 03h
vcmpneqsd xmm1, xmm2, xmm3		; 04h
vcmpnltsd xmm1, xmm2, xmm3		; 05h
vcmpnlesd xmm1, xmm2, xmm3		; 06h
vcmpordsd xmm1, xmm2, xmm3		; 07h

vcmpeq_uqsd xmm1, xmm2, xmm3		; 08h
vcmpngesd xmm1, xmm2, xmm3		; 09h
vcmpngtsd xmm1, xmm2, xmm3		; 0Ah
vcmpfalsesd xmm1, xmm2, xmm3		; 0Bh
vcmpneq_oqsd xmm1, xmm2, xmm3		; 0Ch
vcmpgesd xmm1, xmm2, xmm3		; 0Dh
vcmpgtsd xmm1, xmm2, xmm3		; 0Eh
vcmptruesd xmm1, xmm2, xmm3		; 0Fh

vcmpeq_ossd xmm1, xmm2, xmm3		; 10h
vcmplt_oqsd xmm1, xmm2, xmm3		; 11h
vcmple_oqsd xmm1, xmm2, xmm3		; 12h
vcmpunord_ssd xmm1, xmm2, xmm3		; 13h
vcmpneq_ussd xmm1, xmm2, xmm3		; 14h
vcmpnlt_uqsd xmm1, xmm2, xmm3		; 15h
vcmpnle_uqsd xmm1, xmm2, xmm3		; 16h
vcmpord_ssd xmm1, xmm2, xmm3		; 17h

vcmpeq_ussd xmm1, xmm2, xmm3		; 18h
vcmpnge_uqsd xmm1, xmm2, xmm3		; 19h
vcmpngt_uqsd xmm1, xmm2, xmm3		; 1Ah
vcmpfalse_ossd xmm1, xmm2, xmm3		; 1Bh
vcmpneq_ossd xmm1, xmm2, xmm3		; 1Ch
vcmpge_oqsd xmm1, xmm2, xmm3		; 1Dh
vcmpgt_oqsd xmm1, xmm2, xmm3		; 1Eh
vcmptrue_ussd xmm1, xmm2, xmm3		; 1Fh

cmpeqsd xmm1, [rax]			; 00h
cmpltsd xmm1, [rax]			; 01h
cmplesd xmm1, [rax]			; 02h
cmpunordsd xmm1, [rax]			; 03h
cmpneqsd xmm1, [rax]			; 04h
cmpnltsd xmm1, [rax]			; 05h
cmpnlesd xmm1, [rax]			; 06h
cmpordsd xmm1, [rax]			; 07h

vcmpeqsd xmm1, [rax]			; 00h
vcmpltsd xmm1, [rax]			; 01h
vcmplesd xmm1, [rax]			; 02h
vcmpunordsd xmm1, [rax]			; 03h
vcmpneqsd xmm1, [rax]			; 04h
vcmpnltsd xmm1, [rax]			; 05h
vcmpnlesd xmm1, [rax]			; 06h
vcmpordsd xmm1, [rax]			; 07h

vcmpeqsd xmm1, xmm2, [rax]		; 00h
vcmpltsd xmm1, xmm2, [rax]		; 01h
vcmplesd xmm1, xmm2, [rax]		; 02h
vcmpunordsd xmm1, xmm2, [rax]		; 03h
vcmpneqsd xmm1, xmm2, [rax]		; 04h
vcmpnltsd xmm1, xmm2, [rax]		; 05h
vcmpnlesd xmm1, xmm2, [rax]		; 06h
vcmpordsd xmm1, xmm2, [rax]		; 07h

vcmpeq_uqsd xmm1, xmm2, [rax]		; 08h
vcmpngesd xmm1, xmm2, [rax]		; 09h
vcmpngtsd xmm1, xmm2, [rax]		; 0Ah
vcmpfalsesd xmm1, xmm2, [rax]		; 0Bh
vcmpneq_oqsd xmm1, xmm2, [rax]		; 0Ch
vcmpgesd xmm1, xmm2, [rax]		; 0Dh
vcmpgtsd xmm1, xmm2, [rax]		; 0Eh
vcmptruesd xmm1, xmm2, [rax]		; 0Fh

vcmpeq_ossd xmm1, xmm2, [rax]		; 10h
vcmplt_oqsd xmm1, xmm2, [rax]		; 11h
vcmple_oqsd xmm1, xmm2, [rax]		; 12h
vcmpunord_ssd xmm1, xmm2, [rax]		; 13h
vcmpneq_ussd xmm1, xmm2, [rax]		; 14h
vcmpnlt_uqsd xmm1, xmm2, [rax]		; 15h
vcmpnle_uqsd xmm1, xmm2, [rax]		; 16h
vcmpord_ssd xmm1, xmm2, [rax]		; 17h

vcmpeq_ussd xmm1, xmm2, [rax]		; 18h
vcmpnge_uqsd xmm1, xmm2, [rax]		; 19h
vcmpngt_uqsd xmm1, xmm2, [rax]		; 1Ah
vcmpfalse_ossd xmm1, xmm2, [rax]	; 1Bh
vcmpneq_ossd xmm1, xmm2, [rax]		; 1Ch
vcmpge_oqsd xmm1, xmm2, [rax]		; 1Dh
vcmpgt_oqsd xmm1, xmm2, [rax]		; 1Eh
vcmptrue_ussd xmm1, xmm2, [rax]		; 1Fh

cmpeqsd xmm1, qword [rax]			; 00h
cmpltsd xmm1, qword [rax]			; 01h
cmplesd xmm1, qword [rax]			; 02h
cmpunordsd xmm1, qword [rax]			; 03h
cmpneqsd xmm1, qword [rax]			; 04h
cmpnltsd xmm1, qword [rax]			; 05h
cmpnlesd xmm1, qword [rax]			; 06h
cmpordsd xmm1, qword [rax]			; 07h

vcmpeqsd xmm1, qword [rax]			; 00h
vcmpltsd xmm1, qword [rax]			; 01h
vcmplesd xmm1, qword [rax]			; 02h
vcmpunordsd xmm1, qword [rax]			; 03h
vcmpneqsd xmm1, qword [rax]			; 04h
vcmpnltsd xmm1, qword [rax]			; 05h
vcmpnlesd xmm1, qword [rax]			; 06h
vcmpordsd xmm1, qword [rax]			; 07h

vcmpeqsd xmm1, xmm2, qword [rax]		; 00h
vcmpltsd xmm1, xmm2, qword [rax]		; 01h
vcmplesd xmm1, xmm2, qword [rax]		; 02h
vcmpunordsd xmm1, xmm2, qword [rax]		; 03h
vcmpneqsd xmm1, xmm2, qword [rax]		; 04h
vcmpnltsd xmm1, xmm2, qword [rax]		; 05h
vcmpnlesd xmm1, xmm2, qword [rax]		; 06h
vcmpordsd xmm1, xmm2, qword [rax]		; 07h

vcmpeq_uqsd xmm1, xmm2, qword [rax]		; 08h
vcmpngesd xmm1, xmm2, qword [rax]		; 09h
vcmpngtsd xmm1, xmm2, qword [rax]		; 0Ah
vcmpfalsesd xmm1, xmm2, qword [rax]		; 0Bh
vcmpneq_oqsd xmm1, xmm2, qword [rax]		; 0Ch
vcmpgesd xmm1, xmm2, qword [rax]		; 0Dh
vcmpgtsd xmm1, xmm2, qword [rax]		; 0Eh
vcmptruesd xmm1, xmm2, qword [rax]		; 0Fh

vcmpeq_ossd xmm1, xmm2, qword [rax]		; 10h
vcmplt_oqsd xmm1, xmm2, qword [rax]		; 11h
vcmple_oqsd xmm1, xmm2, qword [rax]		; 12h
vcmpunord_ssd xmm1, xmm2, qword [rax]		; 13h
vcmpneq_ussd xmm1, xmm2, qword [rax]		; 14h
vcmpnlt_uqsd xmm1, xmm2, qword [rax]		; 15h
vcmpnle_uqsd xmm1, xmm2, qword [rax]		; 16h
vcmpord_ssd xmm1, xmm2, qword [rax]		; 17h

vcmpeq_ussd xmm1, xmm2, qword [rax]		; 18h
vcmpnge_uqsd xmm1, xmm2, qword [rax]		; 19h
vcmpngt_uqsd xmm1, xmm2, qword [rax]		; 1Ah
vcmpfalse_ossd xmm1, xmm2, qword [rax]		; 1Bh
vcmpneq_ossd xmm1, xmm2, qword [rax]		; 1Ch
vcmpge_oqsd xmm1, xmm2, qword [rax]		; 1Dh
vcmpgt_oqsd xmm1, xmm2, qword [rax]		; 1Eh
vcmptrue_ussd xmm1, xmm2, qword [rax]		; 1Fh

;-----------------------------------------------------------------------------

cmpeqss xmm1, xmm2			; 00h
cmpltss xmm1, xmm2			; 01h
cmpless xmm1, xmm2			; 02h
cmpunordss xmm1, xmm2			; 03h
cmpneqss xmm1, xmm2			; 04h
cmpnltss xmm1, xmm2			; 05h
cmpnless xmm1, xmm2			; 06h
cmpordss xmm1, xmm2			; 07h

vcmpeqss xmm1, xmm2			; 00h
vcmpltss xmm1, xmm2			; 01h
vcmpless xmm1, xmm2			; 02h
vcmpunordss xmm1, xmm2			; 03h
vcmpneqss xmm1, xmm2			; 04h
vcmpnltss xmm1, xmm2			; 05h
vcmpnless xmm1, xmm2			; 06h
vcmpordss xmm1, xmm2			; 07h

vcmpeqss xmm1, xmm2, xmm3		; 00h
vcmpltss xmm1, xmm2, xmm3		; 01h
vcmpless xmm1, xmm2, xmm3		; 02h
vcmpunordss xmm1, xmm2, xmm3		; 03h
vcmpneqss xmm1, xmm2, xmm3		; 04h
vcmpnltss xmm1, xmm2, xmm3		; 05h
vcmpnless xmm1, xmm2, xmm3		; 06h
vcmpordss xmm1, xmm2, xmm3		; 07h

vcmpeq_uqss xmm1, xmm2, xmm3		; 08h
vcmpngess xmm1, xmm2, xmm3		; 09h
vcmpngtss xmm1, xmm2, xmm3		; 0Ah
vcmpfalsess xmm1, xmm2, xmm3		; 0Bh
vcmpneq_oqss xmm1, xmm2, xmm3		; 0Ch
vcmpgess xmm1, xmm2, xmm3		; 0Dh
vcmpgtss xmm1, xmm2, xmm3		; 0Eh
vcmptruess xmm1, xmm2, xmm3		; 0Fh

vcmpeq_osss xmm1, xmm2, xmm3		; 10h
vcmplt_oqss xmm1, xmm2, xmm3		; 11h
vcmple_oqss xmm1, xmm2, xmm3		; 12h
vcmpunord_sss xmm1, xmm2, xmm3		; 13h
vcmpneq_usss xmm1, xmm2, xmm3		; 14h
vcmpnlt_uqss xmm1, xmm2, xmm3		; 15h
vcmpnle_uqss xmm1, xmm2, xmm3		; 16h
vcmpord_sss xmm1, xmm2, xmm3		; 17h

vcmpeq_usss xmm1, xmm2, xmm3		; 18h
vcmpnge_uqss xmm1, xmm2, xmm3		; 19h
vcmpngt_uqss xmm1, xmm2, xmm3		; 1Ah
vcmpfalse_osss xmm1, xmm2, xmm3		; 1Bh
vcmpneq_osss xmm1, xmm2, xmm3		; 1Ch
vcmpge_oqss xmm1, xmm2, xmm3		; 1Dh
vcmpgt_oqss xmm1, xmm2, xmm3		; 1Eh
vcmptrue_usss xmm1, xmm2, xmm3		; 1Fh

cmpeqss xmm1, [rax]			; 00h
cmpltss xmm1, [rax]			; 01h
cmpless xmm1, [rax]			; 02h
cmpunordss xmm1, [rax]			; 03h
cmpneqss xmm1, [rax]			; 04h
cmpnltss xmm1, [rax]			; 05h
cmpnless xmm1, [rax]			; 06h
cmpordss xmm1, [rax]			; 07h

vcmpeqss xmm1, [rax]			; 00h
vcmpltss xmm1, [rax]			; 01h
vcmpless xmm1, [rax]			; 02h
vcmpunordss xmm1, [rax]			; 03h
vcmpneqss xmm1, [rax]			; 04h
vcmpnltss xmm1, [rax]			; 05h
vcmpnless xmm1, [rax]			; 06h
vcmpordss xmm1, [rax]			; 07h

vcmpeqss xmm1, xmm2, [rax]		; 00h
vcmpltss xmm1, xmm2, [rax]		; 01h
vcmpless xmm1, xmm2, [rax]		; 02h
vcmpunordss xmm1, xmm2, [rax]		; 03h
vcmpneqss xmm1, xmm2, [rax]		; 04h
vcmpnltss xmm1, xmm2, [rax]		; 05h
vcmpnless xmm1, xmm2, [rax]		; 06h
vcmpordss xmm1, xmm2, [rax]		; 07h

vcmpeq_uqss xmm1, xmm2, [rax]		; 08h
vcmpngess xmm1, xmm2, [rax]		; 09h
vcmpngtss xmm1, xmm2, [rax]		; 0Ah
vcmpfalsess xmm1, xmm2, [rax]		; 0Bh
vcmpneq_oqss xmm1, xmm2, [rax]		; 0Ch
vcmpgess xmm1, xmm2, [rax]		; 0Dh
vcmpgtss xmm1, xmm2, [rax]		; 0Eh
vcmptruess xmm1, xmm2, [rax]		; 0Fh

vcmpeq_osss xmm1, xmm2, [rax]		; 10h
vcmplt_oqss xmm1, xmm2, [rax]		; 11h
vcmple_oqss xmm1, xmm2, [rax]		; 12h
vcmpunord_sss xmm1, xmm2, [rax]		; 13h
vcmpneq_usss xmm1, xmm2, [rax]		; 14h
vcmpnlt_uqss xmm1, xmm2, [rax]		; 15h
vcmpnle_uqss xmm1, xmm2, [rax]		; 16h
vcmpord_sss xmm1, xmm2, [rax]		; 17h

vcmpeq_usss xmm1, xmm2, [rax]		; 18h
vcmpnge_uqss xmm1, xmm2, [rax]		; 19h
vcmpngt_uqss xmm1, xmm2, [rax]		; 1Ah
vcmpfalse_osss xmm1, xmm2, [rax]	; 1Bh
vcmpneq_osss xmm1, xmm2, [rax]		; 1Ch
vcmpge_oqss xmm1, xmm2, [rax]		; 1Dh
vcmpgt_oqss xmm1, xmm2, [rax]		; 1Eh
vcmptrue_usss xmm1, xmm2, [rax]		; 1Fh

cmpeqss xmm1, dword [rax]			; 00h
cmpltss xmm1, dword [rax]			; 01h
cmpless xmm1, dword [rax]			; 02h
cmpunordss xmm1, dword [rax]			; 03h
cmpneqss xmm1, dword [rax]			; 04h
cmpnltss xmm1, dword [rax]			; 05h
cmpnless xmm1, dword [rax]			; 06h
cmpordss xmm1, dword [rax]			; 07h

vcmpeqss xmm1, dword [rax]			; 00h
vcmpltss xmm1, dword [rax]			; 01h
vcmpless xmm1, dword [rax]			; 02h
vcmpunordss xmm1, dword [rax]			; 03h
vcmpneqss xmm1, dword [rax]			; 04h
vcmpnltss xmm1, dword [rax]			; 05h
vcmpnless xmm1, dword [rax]			; 06h
vcmpordss xmm1, dword [rax]			; 07h

vcmpeqss xmm1, xmm2, dword [rax]		; 00h
vcmpltss xmm1, xmm2, dword [rax]		; 01h
vcmpless xmm1, xmm2, dword [rax]		; 02h
vcmpunordss xmm1, xmm2, dword [rax]		; 03h
vcmpneqss xmm1, xmm2, dword [rax]		; 04h
vcmpnltss xmm1, xmm2, dword [rax]		; 05h
vcmpnless xmm1, xmm2, dword [rax]		; 06h
vcmpordss xmm1, xmm2, dword [rax]		; 07h

vcmpeq_uqss xmm1, xmm2, dword [rax]		; 08h
vcmpngess xmm1, xmm2, dword [rax]		; 09h
vcmpngtss xmm1, xmm2, dword [rax]		; 0Ah
vcmpfalsess xmm1, xmm2, dword [rax]		; 0Bh
vcmpneq_oqss xmm1, xmm2, dword [rax]		; 0Ch
vcmpgess xmm1, xmm2, dword [rax]		; 0Dh
vcmpgtss xmm1, xmm2, dword [rax]		; 0Eh
vcmptruess xmm1, xmm2, dword [rax]		; 0Fh

vcmpeq_osss xmm1, xmm2, dword [rax]		; 10h
vcmplt_oqss xmm1, xmm2, dword [rax]		; 11h
vcmple_oqss xmm1, xmm2, dword [rax]		; 12h
vcmpunord_sss xmm1, xmm2, dword [rax]		; 13h
vcmpneq_usss xmm1, xmm2, dword [rax]		; 14h
vcmpnlt_uqss xmm1, xmm2, dword [rax]		; 15h
vcmpnle_uqss xmm1, xmm2, dword [rax]		; 16h
vcmpord_sss xmm1, xmm2, dword [rax]		; 17h

vcmpeq_usss xmm1, xmm2, dword [rax]		; 18h
vcmpnge_uqss xmm1, xmm2, dword [rax]		; 19h
vcmpngt_uqss xmm1, xmm2, dword [rax]		; 1Ah
vcmpfalse_osss xmm1, xmm2, dword [rax]		; 1Bh
vcmpneq_osss xmm1, xmm2, dword [rax]		; 1Ch
vcmpge_oqss xmm1, xmm2, dword [rax]		; 1Dh
vcmpgt_oqss xmm1, xmm2, dword [rax]		; 1Eh
vcmptrue_usss xmm1, xmm2, dword [rax]		; 1Fh

