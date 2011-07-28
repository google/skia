/***************************************************************************
 * Copyright (c) 2009,2010, Code Aurora Forum. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 ***************************************************************************/

/***************************************************************************
  Neon memset: Attempts to do a memset with Neon registers if possible,
     Inputs:
        s: The buffer to write to
        c: The integer data to write to the buffer
        n: The size_t count.
     Outputs:

***************************************************************************/

        .code 32
        .fpu neon
        .align 4
        .globl memset16_neon
        .func

memset16_neon:
        cmp             r2, #0
        bxeq            lr

        /* Keep in mind that r2 -- the count argument -- is for the
         * number of 16-bit items to copy.
         */
        lsl             r2, r2, #1

        push            {r0}

        /* If we have < 8 bytes, just do a quick loop to handle that */
        cmp             r2, #8
        bgt             memset_gt4
memset_smallcopy_loop:
        strh            r1, [r0], #2
        subs            r2, r2, #2
        bne             memset_smallcopy_loop
memset_smallcopy_done:
        pop             {r0}
        bx              lr

memset_gt4:
        /*
         * Duplicate the r1 lowest 16-bits across r1. The idea is to have
         * a register with two 16-bit-values we can copy. We do this by
         * duplicating lowest 16-bits of r1 to upper 16-bits.
         */
        orr             r1, r1, r1, lsl #16
        /*
         * If we're copying > 64 bytes, then we may want to get
         * onto a 16-byte boundary to improve speed even more.
         */
        cmp             r2, #64
        blt             memset_route
        ands            r12, r0, #0xf
        beq             memset_route
        /*
         * Determine the number of bytes to move forward to get to the 16-byte
         * boundary.  Note that this will be a multiple of 4, since we
         * already are word-aligned.
         */
        rsb             r12, r12, #16
        sub             r2, r2, r12
        lsls            r12, r12, #29
        strmi           r1, [r0], #4
        strcs           r1, [r0], #4
        strcs           r1, [r0], #4
        lsls            r12, r12, #2
        strcsh          r1, [r0], #2
memset_route:
        /*
         * Decide where to route for the maximum copy sizes.  Note that we
         * build q0 and q1 depending on if we'll need it, so that's
         * interwoven here as well.
         */
        vdup.u32        d0, r1
        cmp             r2, #16
        blt             memset_8
        vmov            d1, d0
        cmp             r2, #64
        blt             memset_16
        vmov            q1, q0
        cmp             r2, #128
        blt             memset_32
memset_128:
        mov             r12, r2, lsr #7
memset_128_loop:
        vst1.64         {q0, q1}, [r0]!
        vst1.64         {q0, q1}, [r0]!
        vst1.64         {q0, q1}, [r0]!
        vst1.64         {q0, q1}, [r0]!
        subs            r12, r12, #1
        bne             memset_128_loop
        ands            r2, r2, #0x7f
        beq             memset_end
memset_32:
        movs            r12, r2, lsr #5
        beq             memset_16
memset_32_loop:
        subs            r12, r12, #1
        vst1.64         {q0, q1}, [r0]!
        bne             memset_32_loop
        ands            r2, r2, #0x1f
        beq             memset_end
memset_16:
        movs            r12, r2, lsr #4
        beq             memset_8
memset_16_loop:
        subs            r12, r12, #1
        vst1.32         {q0}, [r0]!
        bne             memset_16_loop
        ands            r2, r2, #0xf
        beq             memset_end
        /*
         * memset_8 isn't a loop, since we try to do our loops at 16
         * bytes and above.  We should loop there, then drop down here
         * to finish the <16-byte versions.  Same for memset_4 and
         * memset_1.
         */
memset_8:
        cmp             r2, #8
        blt             memset_4
        subs            r2, r2, #8
        vst1.32         {d0}, [r0]!
memset_4:
        cmp             r2, #4
        blt             memset_2
        subs            r2, r2, #4
        str             r1, [r0], #4
memset_2:
        cmp             r2, #0
        ble             memset_end
        strh            r1, [r0], #2
memset_end:
        pop             {r0}
        bx              lr

        .endfunc
        .end
