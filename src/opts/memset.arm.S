/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Changes:
 * 2010-08-11 Steve McIntyre <steve.mcintyre@arm.com>
 *    Added small changes to the two functions to make them work on the
 *    specified number of 16- or 32-bit values rather than the original
 *    code which was specified as a count of bytes. More verbose comments
 *    to aid future maintenance.
 */

    .text
    .align

    .global arm_memset32
    .type   arm_memset32, %function
    .global arm_memset16
    .type   arm_memset16, %function

/*
 * Optimized memset functions for ARM.
 *
 * void arm_memset16(uint16_t* dst, uint16_t value, int count);
 * void arm_memset32(uint32_t* dst, uint32_t value, int count);
 *
 */
arm_memset16:
        .fnstart
        push        {lr}

        /* expand the data to 32 bits */
        orr         r1, r1, lsl #16

        /* align to 32 bits */
        tst         r0, #2
        strneh      r1, [r0], #2
        subne       r2, r2, #2

        /* Multiply count by 2 - go from the number of 16-bit shorts
         * to the number of bytes desired. */
        mov         r2, r2, lsl #1

        /* Now jump into the main loop below. */
        b           .Lwork_32
        .fnend

arm_memset32:
        .fnstart
        push        {lr}

        /* Multiply count by 4 - go from the number of 32-bit words to
         * the number of bytes desired. */
        mov         r2, r2, lsl #2

.Lwork_32:
        /* Set up registers ready for writing them out. */
        mov         ip, r1
        mov         lr, r1

        /* Try to align the destination to a cache line. Assume 32
         * byte (8 word) cache lines, it's the common case. */
        rsb         r3, r0, #0
        ands        r3, r3, #0x1C
        beq         .Laligned32
        cmp         r3, r2
        andhi       r3, r2, #0x1C
        sub         r2, r2, r3

        /* (Optionally) write any unaligned leading bytes.
         * (0-28 bytes, length in r3) */
        movs        r3, r3, lsl #28
        stmcsia     r0!, {r1, lr}
        stmcsia     r0!, {r1, lr}
        stmmiia     r0!, {r1, lr}
        movs        r3, r3, lsl #2
        strcs       r1, [r0], #4

        /* Now quickly loop through the cache-aligned data. */
.Laligned32:
        mov         r3, r1
1:      subs        r2, r2, #32
        stmhsia     r0!, {r1,r3,ip,lr}
        stmhsia     r0!, {r1,r3,ip,lr}
        bhs         1b
        add         r2, r2, #32

        /* (Optionally) store any remaining trailing bytes.
         * (0-30 bytes, length in r2) */
        movs        r2, r2, lsl #28
        stmcsia     r0!, {r1,r3,ip,lr}
        stmmiia     r0!, {r1,lr}
        movs        r2, r2, lsl #2
        strcs       r1, [r0], #4
        strmih      lr, [r0], #2

        pop         {pc}
        .fnend
