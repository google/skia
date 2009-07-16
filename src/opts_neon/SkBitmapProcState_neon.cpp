#include <arm_neon.h>
#include "SkBitmapProcState.h"
#include "SkColorPriv.h"
#include "SkFilterProc.h"

void S16_D16_filter_DX_neon(const SkBitmapProcState& s,
                           const uint32_t* SK_RESTRICT xy,
                           int count, uint16_t* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);

    const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
    unsigned rb = s.fBitmap->rowBytes();
    unsigned subY;
    const uint16_t* SK_RESTRICT row0;
    const uint16_t* SK_RESTRICT row1;
    unsigned int rowgap;
    const uint32_t c7ffe = 0x7ffe;

    // setup row ptrs and update proc_table
    {
        uint32_t XY = *xy++;
        unsigned y0 = XY >> 14;
        row0 = (const uint16_t*)(srcAddr + (y0 >> 4) * rb);
        row1 = (const uint16_t*)(srcAddr + (XY & 0x3FFF) * rb);
        rowgap = (unsigned int)row1 - (unsigned int)row0;
        subY = y0 & 0xF;
    }

    unsigned int count4 = ((count >> 2) << 4) | subY;
    count &= 3;

    asm volatile (
        "and            r4, %[count4], #0xF             \n\t"   // mask off subY
        "vmov.u16       d2[0], r4                       \n\t"   // move subY to Neon
        "rsb            r4, r4, #16                     \n\t"   // r4 = 16-subY
        "vmov.u16       d2[1], r4                       \n\t"   // move 16-subY to Neon
        "movs           %[count4], %[count4], lsr #4    \n\t"   // shift count down, lose subY
        "vmov.u16       d3, #16                         \n\t"   // create constant
        "vmov.u16       q2, #31                         \n\t"   // set up blue mask
        "beq            2f                              \n\t"   // if count4 == 0, exit

    "1:                                                 \n\t"
        "ldmia          %[xy]!, {r4, r5, r6, r7}        \n\t"   // load four xy values
                                                                // xy = [ x0:14 | subX:4 | x1:14 ]
        // extract subX for iter 0-3
        "vmov           d0, r4, r5                      \n\t"   // move xy to Neon, iter 0-1
        "vmov           d1, r6, r7                      \n\t"   // move xy to Neon, iter 2-3

        // Load 16 pixels for four filter iterations from memory.
        // Because the source pixels are potentially scattered, each lane
        // of each vector is loaded separately. Also, the X sub pixel
        // offset is extracted.

        // iter 0
        "mov            r8, r4, lsr #18                 \n\t"   // extract x0
        "and            r4, %[c7ffe], r4, lsl #1        \n\t"   // extract x1 and make byte offset
        "add            r8, %[row0], r8, lsl #1         \n\t"   // calculate address of row0[x0]
        "add            r4, %[row0], r4                 \n\t"   // calculate address of row0[x1]
        "vld1.u16       {d16[0]}, [r8], %[rowgap]       \n\t"   // load row0[x0] and move ptr to row1
        "vld1.u16       {d17[0]}, [r4], %[rowgap]       \n\t"   // load row0[x1] and move ptr to row1
        "vld1.u16       {d18[0]}, [r8]                  \n\t"   // load row1[x0]
        "vld1.u16       {d19[0]}, [r4]                  \n\t"   // load row1[x1]

        // iter 1
        "mov            r8, r5, lsr #18                 \n\t"   // extract x0
        "and            r5, %[c7ffe], r5, lsl #1        \n\t"   // extract x1 and make byte offset
        "add            r8, %[row0], r8, lsl #1         \n\t"   // calculate address of row0[x0]
        "add            r5, %[row0], r5                 \n\t"   // calculate address of row0[x1]
        "vld1.u16       {d16[1]}, [r8], %[rowgap]       \n\t"   // load row0[x0] and move ptr to row1
        "vld1.u16       {d17[1]}, [r5], %[rowgap]       \n\t"   // load row0[x1] and move ptr to row1
        "vld1.u16       {d18[1]}, [r8]                  \n\t"   // load row1[x0]
        "vld1.u16       {d19[1]}, [r5]                  \n\t"   // load row1[x1]

        "vshrn.u32      d0, q0, #2                      \n\t"   // shift right subX by 2 and narrow
        // iter 2
        "mov            r8, r6, lsr #18                 \n\t"   // extract x0
        "and            r6, %[c7ffe], r6, lsl #1        \n\t"   // extract x1 and make byte offset
        "add            r8, %[row0], r8, lsl #1         \n\t"   // calculate address of row0[x0]
        "add            r6, %[row0], r6                 \n\t"   // calculate address of row0[x1]
        "vld1.u16       {d16[2]}, [r8], %[rowgap]       \n\t"   // load row0[x0] and move ptr to row1
        "vld1.u16       {d17[2]}, [r6], %[rowgap]       \n\t"   // load row0[x1] and move ptr to row1
        "vld1.u16       {d18[2]}, [r8]                  \n\t"   // load row1[x0]
        "vld1.u16       {d19[2]}, [r6]                  \n\t"   // load row1[x1]

        "vshr.u16       d0, d0, #12                     \n\t"   // shift right subX to bottom 4 bits
        // iter 3
        "mov            r8, r7, lsr #18                 \n\t"   // extract x0
        "and            r7, %[c7ffe], r7, lsl #1        \n\t"   // extract x1 and make byte offset
        "add            r8, %[row0], r8, lsl #1         \n\t"   // calculate address of row0[x0]
        "add            r7, %[row0], r7                 \n\t"   // calculate address of row0[x1]
        "vld1.u16       {d16[3]}, [r8], %[rowgap]       \n\t"   // load row0[x0] and move ptr to row1
        "vld1.u16       {d17[3]}, [r7], %[rowgap]       \n\t"   // load row0[x1] and move ptr to row1
        "vld1.u16       {d18[3]}, [r8]                  \n\t"   // load row1[x0]
        "vld1.u16       {d19[3]}, [r7]                  \n\t"   // load row1[x1]

        // Registers d16-d19 now contain pixels a00-a11 for 4 iterations:
        //   d16 = [ a00_3 | a00_2 | a00_1 | a00_0 ]
        //   d17 = [ a01_3 | a01_2 | a01_1 | a01_0 ]
        //   d18 = [ a10_3 | a10_2 | a10_1 | a10_0 ]
        //   d19 = [ a11_3 | a11_2 | a11_1 | a11_0 ]
        //
        // Extract RGB channels from each 565 pixel.

        "vshl.i16       q11, q8, #5                     \n\t"   // shift greens to top of each lane
        "vand           q12, q8, q2                     \n\t"   // mask blues
        "vshr.u16       q10, q8, #11                    \n\t"   // shift reds to bottom of each lane
        "vshr.u16       q11, q11, #10                   \n\t"   // shift greens to bottom of each lane
        "vshl.i16       q14, q9, #5                     \n\t"   // shift greens to top of each lane
        "vand           q15, q9, q2                     \n\t"   // mask blues
        "vshr.u16       q13, q9, #11                    \n\t"   // shift reds to bottom of each lane
        "vshr.u16       q14, q14, #10                   \n\t"   // shift greens to bottom of each lane

        // There are now six Q regs, containing
        //   q10 = [ a01r3 | a01r2 | a01r1 | a01r0 | a00r3 | a00r2 | a00r1 | a00r0 ]
        //   q11 = [ a01g3 | a01g2 | a01g1 | a01g0 | a00g3 | a00g2 | a00g1 | a00g0 ]
        //   q12 = [ a01b3 | a01b2 | a01b1 | a01b0 | a00b3 | a00b2 | a00b1 | a00b0 ]
        //   q13 = [ a11r3 | a11r2 | a11r1 | a11r0 | a01r3 | a01r2 | a01r1 | a01r0 ]
        //   q14 = [ a11g3 | a11g2 | a11g1 | a11g0 | a01g3 | a01g2 | a01g1 | a01g0 ]
        //   q15 = [ a11b3 | a11b2 | a11b1 | a11b0 | a01b3 | a01b2 | a01b1 | a01b0 ]
        // where aXXyZ: XX = pixel position, y = colour channel, Z = iteration
        //   d0 = subX, d1 = 16-subX
        //   d2[0] = subY, d2[1] = 16-subY
        //   d3 = 16, q2(d4d5) = 31

        // The filter:
        //
        //           |        |
        //     ---- a00 ---- a01 ----> * (16-y)
        //           |        |
        //     -----a10 ---- a11 ----> * y
        //           |        |
        //           V        V
        //        * (16-x)   * x
        //
        // result = (a00.(16-y).(16-x) + a01.(16-y).x + a10.(16-x).y + a11.x.y) >> 8
        //

        "vsub.u16       d1, d3, d0                      \n\t"   // calculate 16-subX
        // multiply top pixel pair by (16-y)
        "vmul.i16       q10, q10, d2[1]                 \n\t"   // top reds multiplied by (16-y)
        "vmul.i16       q11, q11, d2[1]                 \n\t"   // top greens multiplied by (16-y)
        "vmul.i16       q12, q12, d2[1]                 \n\t"   // top blues multiplied by (16-y)
        // multiply bottom pixel pair by y
        "vmul.i16       q13, q13, d2[0]                 \n\t"   // bottom reds multiplied by y
        "vmul.i16       q14, q14, d2[0]                 \n\t"   // bottom greens multiplied by y
        "vmul.i16       q15, q15, d2[0]                 \n\t"   // bottom blues multiplied by y
        // mul/acc left pixels by (16-x)
        "vmul.i16       d16, d20, d1                    \n\t"   // resultr  = a00r * (16-x)
        "vmul.i16       d17, d22, d1                    \n\t"   // resultg  = a00g * (16-x)
        "vmul.i16       d18, d24, d1                    \n\t"   // resultb  = a00b * (16-x)
        "vmla.i16       d16, d26, d1                    \n\t"   // resultr += a00r * (16-x)
        "vmla.i16       d17, d28, d1                    \n\t"   // resultg += a00g * (16-x)
        "vmla.i16       d18, d30, d1                    \n\t"   // resultb += a00b * (16-x)
        // mul/acc right pixels by x
        "vmla.i16       d16, d21, d0                    \n\t"   // resultr += a01r * x
        "vmla.i16       d17, d23, d0                    \n\t"   // resultg += a01g * x
        "vmla.i16       d18, d25, d0                    \n\t"   // resultb += a01b * x
        "vmla.i16       d16, d27, d0                    \n\t"   // resultr += a11r * x
        "vmla.i16       d17, d29, d0                    \n\t"   // resultg += a11g * x
        "vmla.i16       d18, d31, d0                    \n\t"   // resultb += a11b * x
        "subs           %[count4], %[count4], #1        \n\t"   // decrement counter
        // shift results down 8 bits
        "vshr.u16       q8, q8, #8                      \n\t"   // resultr >>= 8, resultg >>=8
        "vshr.u16       d18, d18, #8                    \n\t"   // resultb >>= 8
        // put rgb into 565
        "vsli.i16       d18, d17, #5                    \n\t"   // shift greens into blues
        "vsli.i16       d18, d16, #11                   \n\t"   // shift reds into greens and blues
        "vst1.i16       {d18}, [%[colors]]!             \n\t"   // store result
        "bgt            1b                              \n\t"   // if counter > 0, loop
    "2:                                                 \n\t"   // exit
        : [xy] "+r" (xy), [count4] "+r" (count4), [colors] "+r" (colors)
        : [row0] "r" (row0), [rowgap] "r" (rowgap), [c7ffe] "r" (c7ffe)
        : "cc", "memory", "r4", "r5", "r6", "r7", "r8", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"
    );

    while(count != 0) {
        uint32_t XX = *xy++;    // x0:14 | subX:4 | x1:14
        unsigned x0 = XX >> 14;
        unsigned x1 = XX & 0x3FFF;
        unsigned subX = x0 & 0xF;
        x0 >>= 4;

        uint32_t a00 = SkExpand_rgb_16(row0[x0]);
        uint32_t a01 = SkExpand_rgb_16(row0[x1]);
        uint32_t a10 = SkExpand_rgb_16(row1[x0]);
        uint32_t a11 = SkExpand_rgb_16(row1[x1]);

        int xy = subX * subY >> 3;
        uint32_t c = a00 * (32 - 2*subY - 2*subX + xy) +
                     a01 * (2*subX - xy) +
                     a10 * (2*subY - xy) +
                     a11 * xy;

        *colors++ = SkCompact_rgb_16(c>>5);
        count--;
    }
}

