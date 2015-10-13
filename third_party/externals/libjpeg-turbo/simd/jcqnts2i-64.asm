;
; jcqnts2i-64.asm - sample data conversion and quantization (64-bit SSE2)
;
; Copyright 2009 Pierre Ossman <ossman@cendio.se> for Cendio AB
; Copyright 2009 D. R. Commander
;
; Based on
; x86 SIMD extension for IJG JPEG library
; Copyright (C) 1999-2006, MIYASAKA Masaru.
; For conditions of distribution and use, see copyright notice in jsimdext.inc
;
; This file should be assembled with NASM (Netwide Assembler),
; can *not* be assembled with Microsoft's MASM or any compatible
; assembler (including Borland's Turbo Assembler).
; NASM is available from http://nasm.sourceforge.net/ or
; http://sourceforge.net/project/showfiles.php?group_id=6208
;
; [TAB8]

%include "jsimdext.inc"
%include "jdct.inc"

; --------------------------------------------------------------------------
	SECTION	SEG_TEXT
	BITS	64
;
; Load data into workspace, applying unsigned->signed conversion
;
; GLOBAL(void)
; jsimd_convsamp_sse2 (JSAMPARRAY sample_data, JDIMENSION start_col,
;                      DCTELEM * workspace);
;

; r10 = JSAMPARRAY sample_data
; r11 = JDIMENSION start_col
; r12 = DCTELEM * workspace

	align	16
	global	EXTN(jsimd_convsamp_sse2) PRIVATE

EXTN(jsimd_convsamp_sse2):
	push	rbp
	mov	rax,rsp
	mov	rbp,rsp
	collect_args
	push	rbx

	pxor	xmm6,xmm6		; xmm6=(all 0's)
	pcmpeqw	xmm7,xmm7
	psllw	xmm7,7			; xmm7={0xFF80 0xFF80 0xFF80 0xFF80 ..}

	mov rsi, r10
	mov rax, r11
	mov rdi, r12
	mov	rcx, DCTSIZE/4
.convloop:
	mov	rbx, JSAMPROW [rsi+0*SIZEOF_JSAMPROW]	; (JSAMPLE *)
	mov rdx, JSAMPROW [rsi+1*SIZEOF_JSAMPROW]	; (JSAMPLE *)

	movq	xmm0, XMM_MMWORD [rbx+rax*SIZEOF_JSAMPLE]	; xmm0=(01234567)
	movq	xmm1, XMM_MMWORD [rdx+rax*SIZEOF_JSAMPLE]	; xmm1=(89ABCDEF)

	mov	rbx, JSAMPROW [rsi+2*SIZEOF_JSAMPROW]	; (JSAMPLE *)
	mov	rdx, JSAMPROW [rsi+3*SIZEOF_JSAMPROW]	; (JSAMPLE *)

	movq	xmm2, XMM_MMWORD [rbx+rax*SIZEOF_JSAMPLE]	; xmm2=(GHIJKLMN)
	movq	xmm3, XMM_MMWORD [rdx+rax*SIZEOF_JSAMPLE]	; xmm3=(OPQRSTUV)

	punpcklbw xmm0,xmm6		; xmm0=(01234567)
	punpcklbw xmm1,xmm6		; xmm1=(89ABCDEF)
	paddw     xmm0,xmm7
	paddw     xmm1,xmm7
	punpcklbw xmm2,xmm6		; xmm2=(GHIJKLMN)
	punpcklbw xmm3,xmm6		; xmm3=(OPQRSTUV)
	paddw     xmm2,xmm7
	paddw     xmm3,xmm7

	movdqa	XMMWORD [XMMBLOCK(0,0,rdi,SIZEOF_DCTELEM)], xmm0
	movdqa	XMMWORD [XMMBLOCK(1,0,rdi,SIZEOF_DCTELEM)], xmm1
	movdqa	XMMWORD [XMMBLOCK(2,0,rdi,SIZEOF_DCTELEM)], xmm2
	movdqa	XMMWORD [XMMBLOCK(3,0,rdi,SIZEOF_DCTELEM)], xmm3

	add	rsi, byte 4*SIZEOF_JSAMPROW
	add	rdi, byte 4*DCTSIZE*SIZEOF_DCTELEM
	dec	rcx
	jnz	short .convloop

	pop	rbx
	uncollect_args
	pop	rbp
	ret

; --------------------------------------------------------------------------
;
; Quantize/descale the coefficients, and store into coef_block
;
; This implementation is based on an algorithm described in
;   "How to optimize for the Pentium family of microprocessors"
;   (http://www.agner.org/assem/).
;
; GLOBAL(void)
; jsimd_quantize_sse2 (JCOEFPTR coef_block, DCTELEM * divisors,
;                      DCTELEM * workspace);
;

%define RECIPROCAL(m,n,b) XMMBLOCK(DCTSIZE*0+(m),(n),(b),SIZEOF_DCTELEM)
%define CORRECTION(m,n,b) XMMBLOCK(DCTSIZE*1+(m),(n),(b),SIZEOF_DCTELEM)
%define SCALE(m,n,b)      XMMBLOCK(DCTSIZE*2+(m),(n),(b),SIZEOF_DCTELEM)

; r10 = JCOEFPTR coef_block
; r11 = DCTELEM * divisors
; r12 = DCTELEM * workspace

	align	16
	global	EXTN(jsimd_quantize_sse2) PRIVATE

EXTN(jsimd_quantize_sse2):
	push	rbp
	mov	rax,rsp
	mov	rbp,rsp
	collect_args

	mov rsi, r12
	mov rdx, r11
	mov rdi, r10
	mov	rax, DCTSIZE2/32
.quantloop:
	movdqa	xmm4, XMMWORD [XMMBLOCK(0,0,rsi,SIZEOF_DCTELEM)]
	movdqa	xmm5, XMMWORD [XMMBLOCK(1,0,rsi,SIZEOF_DCTELEM)]
	movdqa	xmm6, XMMWORD [XMMBLOCK(2,0,rsi,SIZEOF_DCTELEM)]
	movdqa	xmm7, XMMWORD [XMMBLOCK(3,0,rsi,SIZEOF_DCTELEM)]
	movdqa	xmm0,xmm4
	movdqa	xmm1,xmm5
	movdqa	xmm2,xmm6
	movdqa	xmm3,xmm7
	psraw	xmm4,(WORD_BIT-1)
	psraw	xmm5,(WORD_BIT-1)
	psraw	xmm6,(WORD_BIT-1)
	psraw	xmm7,(WORD_BIT-1)
	pxor	xmm0,xmm4
	pxor	xmm1,xmm5
	pxor	xmm2,xmm6
	pxor	xmm3,xmm7
	psubw	xmm0,xmm4		; if (xmm0 < 0) xmm0 = -xmm0;
	psubw	xmm1,xmm5		; if (xmm1 < 0) xmm1 = -xmm1;
	psubw	xmm2,xmm6		; if (xmm2 < 0) xmm2 = -xmm2;
	psubw	xmm3,xmm7		; if (xmm3 < 0) xmm3 = -xmm3;

	paddw	xmm0, XMMWORD [CORRECTION(0,0,rdx)]  ; correction + roundfactor
	paddw	xmm1, XMMWORD [CORRECTION(1,0,rdx)]
	paddw	xmm2, XMMWORD [CORRECTION(2,0,rdx)]
	paddw	xmm3, XMMWORD [CORRECTION(3,0,rdx)]
	pmulhuw	xmm0, XMMWORD [RECIPROCAL(0,0,rdx)]  ; reciprocal
	pmulhuw	xmm1, XMMWORD [RECIPROCAL(1,0,rdx)]
	pmulhuw	xmm2, XMMWORD [RECIPROCAL(2,0,rdx)]
	pmulhuw	xmm3, XMMWORD [RECIPROCAL(3,0,rdx)]
	pmulhuw	xmm0, XMMWORD [SCALE(0,0,rdx)]	; scale
	pmulhuw	xmm1, XMMWORD [SCALE(1,0,rdx)]
	pmulhuw	xmm2, XMMWORD [SCALE(2,0,rdx)]
	pmulhuw	xmm3, XMMWORD [SCALE(3,0,rdx)]

	pxor	xmm0,xmm4
	pxor	xmm1,xmm5
	pxor	xmm2,xmm6
	pxor	xmm3,xmm7
	psubw	xmm0,xmm4
	psubw	xmm1,xmm5
	psubw	xmm2,xmm6
	psubw	xmm3,xmm7
	movdqa	XMMWORD [XMMBLOCK(0,0,rdi,SIZEOF_DCTELEM)], xmm0
	movdqa	XMMWORD [XMMBLOCK(1,0,rdi,SIZEOF_DCTELEM)], xmm1
	movdqa	XMMWORD [XMMBLOCK(2,0,rdi,SIZEOF_DCTELEM)], xmm2
	movdqa	XMMWORD [XMMBLOCK(3,0,rdi,SIZEOF_DCTELEM)], xmm3

	add	rsi, byte 32*SIZEOF_DCTELEM
	add	rdx, byte 32*SIZEOF_DCTELEM
	add	rdi, byte 32*SIZEOF_JCOEF
	dec	rax
	jnz	near .quantloop

	uncollect_args
	pop	rbp
	ret

; For some reason, the OS X linker does not honor the request to align the
; segment unless we do this.
	align	16
