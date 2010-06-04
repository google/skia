/***************************************************************************
 Copyright (c) 2009,2010, Code Aurora Forum. All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License"); you
 may not use this file except in compliance with the License.  You may
 obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 implied.  See the License for the specific language governing
 permissions and limitations under the License.
 ***************************************************************************/

	.code 32
	.fpu neon
	.align 4
	.globl	memset32_neon
	.func

	/* r0 = buffer, r1 = value, r2 = times to write */
memset32_neon:
	cmp		r2, #1
	streq		r1, [r0], #4
	bxeq		lr

	cmp		r2, #4
	bgt		memset32_neon_start
	cmp		r2, #0
	bxeq		lr
memset32_neon_small:
	str		r1, [r0], #4
	subs		r2, r2, #1
	bne		memset32_neon_small
	bx		lr
memset32_neon_start:
	cmp		r2, #16
	blt		memset32_dropthru
	vdup.32		q0, r1
	vmov		q1, q0
	cmp		r2, #32
	blt		memset32_16
	cmp		r2, #64
	blt		memset32_32
	cmp		r2, #128
	blt		memset32_64
memset32_128:
	movs		r12, r2, lsr #7
memset32_loop128:
	subs		r12, r12, #1
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	bne		memset32_loop128
	ands		r2, r2, #0x7f
	bxeq		lr
memset32_64:
	movs		r12, r2, lsr #6
	beq		memset32_32
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	ands		r2, r2, #0x3f
	bxeq		lr
memset32_32:
	movs		r12, r2, lsr #5
	beq		memset32_16
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
	ands		r2, r2, #0x1f
	bxeq		lr
memset32_16:
	movs		r12, r2, lsr #4
	beq		memset32_dropthru
	and		r2, r2, #0xf
	vst1.64		{q0, q1}, [r0]!
	vst1.64		{q0, q1}, [r0]!
memset32_dropthru:
	rsb		r2, r2, #15
	add		pc, pc, r2, lsl #2
	nop
	str		r1, [r0, #56]
	str		r1, [r0, #52]
	str		r1, [r0, #48]
	str		r1, [r0, #44]
	str		r1, [r0, #40]
	str		r1, [r0, #36]
	str		r1, [r0, #32]
	str		r1, [r0, #28]
	str		r1, [r0, #24]
	str		r1, [r0, #20]
	str		r1, [r0, #16]
	str		r1, [r0, #12]
	str		r1, [r0, #8]
	str		r1, [r0, #4]
	str		r1, [r0, #0]
	bx		lr

	.endfunc
	.end
