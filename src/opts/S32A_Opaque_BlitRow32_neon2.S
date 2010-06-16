/***************************************************************************
 Copyright (c) 2010, Code Aurora Forum. All rights reserved.

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

    .text
    .fpu neon
    .global S32A_Opaque_BlitRow32_neon2
    .func S32A_Opaque_BlitRow32_neon2
S32A_Opaque_BlitRow32_neon2:
// extern "C" void S32A_Opaque_BlitRow32_neon2(SkPMColor* SK_RESTRICT dst,
//                                             const SkPMColor* SK_RESTRICT src,
//                                             int count, U8CPU alpha);
// r0 dst
// r1 src
// r2 alpha
//
// Take advantage of vld4 to work on 8 channels at a time instead of 4 as
// original neon version.
//
// For the bytes that are in the last 8 bytes (len%8) we use none-Neon assembly
// dst = src + SkAlphaMulQ(dst, SkAlpha255To256(255 - SkGetPackedA32(src)))
//
// For the bytes in the middle, we use
// dst = src + (dst * ((255-a) + (255-a)>>7)) >> 8
//
// We also take advantage of the software pipelining, working on the current 8
// channels while loading the next 8 channels.
//
// Some better technique as register buffer can be used for the last 8 bytes...
//
    PUSH     {r4-r11}
    CMP      r3,#0xff
    BNE      .Lto_exit
    CMP      r2,#0
    BLE      .Lto_exit
    CMP      r2,#24
    BLT      .Lless_than_24

    VPUSH    {Q4-Q7}

    VMOV.I16 q14,#0xff               //;Q4.16 = 255
//prefix
    vld4.8  {d0, d1, d2, d3}, [r1]! //d0,d1,d2,d3 = sourc rgb(0,1,2,3) A(0,1,2,3)
                                    //update source ptr but not dst ptr
    vld4.8  {d4, d5, d6, d7}, [r0]  //d4,d5,d6,d7 = dst rgb(0,1,2,3) A(0,1,2,3)
    add      r3, r0, #32 // minus 16 to pretend the last round
    mov      r5, #64
    SUB      r2,r2,#8
.Lloop:
    SUB      r2,r2,#16
    VSUBW.U8 q4,q14,d3               //Q4.16 = 255-d3
                                    //update source ptr but not dst ptr

    //It has to be 24 since we pre-load 8 word for the next rounds
    CMP      r2,#16

    VSRA.U16 q4,q4,#7               //Q4 = Q4.16+Q4 >> 7 ; A(0,1,2,3)

    VMOVL.U8 q6,d4                  //Q6 = vmovl.u8 d4
    VMOVL.U8 q7,d5                  //Q7 = vmovl.u8 d5
    VMOVL.U8 q8,d6                  //Q8 = vmovl.u8 d6
    VMOVL.U8 q9,d7                  //Q9 = vmovl.u8 d7


    VMUL.I16 q6,q6,q4               //Q6 = Q6 * Q4
    VMUL.I16 q7,q7,q4               //Q7 = Q7 * Q4

    vld4.8  {d20, d21, d22, d23}, [r1]! //d0,d1,d2,d3 = sourc rgb(0,1,2,3) A(0,1,2,3)

    VMUL.I16 q8,q8,q4               //Q8 = Q8 * Q4
    VMUL.I16 q9,q9,q4               //Q9 = Q9 * Q4

    vld4.8  {d24, d25, d26, d27}, [r3]  //d4,d5,d6,d7 = dst rgb(0,1,2,3) A(0,1,2,3)

    VSHRN.I16 d4,q6,#8              //d4 = Q6.16 shrn 8
    VSHRN.I16 d5,q7,#8              //d5 = Q7.16 shrn 8
    VSHRN.I16 d6,q8,#8              //d6 = Q8.16 shrn 8
    VSHRN.I16 d7,q9,#8              //d7 = Q9.16 shrn 8

    VADD.I8  d4,d4,d0               //d4 = d4+d0
    VADD.I8  d5,d5,d1               //d5 = d5+d1
    VADD.I8  d6,d6,d2               //d6 = d6+d2
    VADD.I8  d7,d7,d3               //d7 = d7+d3

    vst4.8  {d4, d5, d6, d7}, [r0], r5 //dst rgb(0,1,2,3) A(0,1,2,3) = d4,d5,d6,d7
    //add r0, r0, r5

    //The next 4 words
//    vld4.8  {d20, d21, d22, d23}, [r1]! ;d0,d1,d2,d3 = sourc rgb(0,1,2,3) A(0,1,2,3)
//                                    ;update source ptr but not dst ptr
//    vld4.8  {d24, d25, d26, d27}, [r0]  ;d4,d5,d6,d7 = dst rgb(0,1,2,3) A(0,1,2,3)

                                    //update source ptr but not dst ptr
    VSUBW.U8 q4,q14,d23               //Q4.16 = 255-d3

    VSRA.U16 q4,q4,#7               //Q4 = Q4.16+Q4 >> 7 ; A(0,1,2,3)

    VMOVL.U8 q6,d24                  //Q6 = vmovl.u8 d4
    VMOVL.U8 q7,d25                  //Q7 = vmovl.u8 d5
    VMOVL.U8 q8,d26                  //Q8 = vmovl.u8 d6
    VMOVL.U8 q9,d27                  //Q9 = vmovl.u8 d7

    VMUL.I16 q6,q6,q4               //Q6 = Q6 * Q4
    VMUL.I16 q7,q7,q4               //Q7 = Q7 * Q4

    vld4.8  {d0, d1, d2, d3}, [r1]! //d0,d1,d2,d3 = sourc rgb(0,1,2,3) A(0,1,2,3)

    VMUL.I16 q8,q8,q4               //Q8 = Q8 * Q4
    VMUL.I16 q9,q9,q4               //Q9 = Q9 * Q4

    vld4.8  {d4, d5, d6, d7}, [r0]  //d4,d5,d6,d7 = dst rgb(0,1,2,3) A(0,1,2,3)
    VSHRN.I16 d24,q6,#8              //d4 = Q6.16 shrn 8
    VSHRN.I16 d25,q7,#8              //d5 = Q7.16 shrn 8
    VSHRN.I16 d26,q8,#8              //d6 = Q8.16 shrn 8
    VSHRN.I16 d27,q9,#8              //d7 = Q9.16 shrn 8

    VADD.I8  d24,d24,d20               //d4 = d4+d0
    VADD.I8  d25,d25,d21               //d5 = d5+d1
    VADD.I8  d26,d26,d22               //d6 = d6+d2
    VADD.I8  d27,d27,d23               //d7 = d7+d3

    vst4.8  {d24, d25, d26, d27}, [r3], r5 //dst rgb(0,1,2,3) A(0,1,2,3) = d4,d5,d6,d7
    //add r3, r3, r5

    BGE      .Lloop

//postfix:
//There are 8 words left unprocessed from previous round
    VMOV.I16 q4,#0xff               //Q4.16 = 255
    VSUBW.U8 q4,q4,d3               //Q4.16 = 255-d3

    CMP      r2,#8

    VSHR.U16 q5,q4,#7               //Q5.16 = Q4 >> 7
    VADD.I16 q4,q4,q5               //Q4 = Q4.16+Q5.16 ; A(0,1,2,3)

    VMOVL.U8 q6,d4                  //Q6 = vmovl.u8 d4
    VMOVL.U8 q7,d5                  //Q7 = vmovl.u8 d5
    VMOVL.U8 q8,d6                  //Q8 = vmovl.u8 d6
    VMOVL.U8 q9,d7                  //Q9 = vmovl.u8 d7

    VMUL.I16 q6,q6,q4               //Q6 = Q6 * Q4
    VMUL.I16 q7,q7,q4               //Q7 = Q7 * Q4
    VMUL.I16 q8,q8,q4               //Q8 = Q8 * Q4
    VMUL.I16 q9,q9,q4               //Q9 = Q9 * Q4

    VSHRN.I16 d4,q6,#8              //d4 = Q6.16 shrn 8
    VSHRN.I16 d5,q7,#8              //d5 = Q7.16 shrn 8
    VSHRN.I16 d6,q8,#8              //d6 = Q8.16 shrn 8
    VSHRN.I16 d7,q9,#8              //d7 = Q9.16 shrn 8

    VADD.I8  d4,d4,d0               //d4 = d4+d0
    VADD.I8  d5,d5,d1               //d5 = d5+d1
    VADD.I8  d6,d6,d2               //d6 = d6+d2
    VADD.I8  d7,d7,d3               //d7 = d7+d3

    vst4.8  {d4, d5, d6, d7}, [r0]! //dst rgb(0,1,2,3) A(0,1,2,3) = d4,d5,d6,d7

.Lless_than_16:
    CMP      r2,#8
    BLT      .Lless_than_8

    SUB      r2,r2,#8

    vld4.8  {d0, d1, d2, d3}, [r1]! //d0,d1,d2,d3 = sourc rgb(0,1,2,3) A(0,1,2,3)
                                    //update source ptr but not dst ptr
    vld4.8  {d4, d5, d6, d7}, [r0]  //d4,d5,d6,d7 = dst rgb(0,1,2,3) A(0,1,2,3)

    VMOV.I16 q4,#0xff               //Q4.16 = 255
    VSUBW.U8 q4,q4,d3               //Q4.16 = 255-d3

    CMP      r2,#8

    VSHR.U16 q5,q4,#7               //Q5.16 = Q4 >> 7
    VADD.I16 q4,q4,q5               //Q4 = Q4.16+Q5.16 ; A(0,1,2,3)

    VMOVL.U8 q6,d4                  //Q6 = vmovl.u8 d4
    VMOVL.U8 q7,d5                  //Q7 = vmovl.u8 d5
    VMOVL.U8 q8,d6                  //Q8 = vmovl.u8 d6
    VMOVL.U8 q9,d7                  //Q9 = vmovl.u8 d7

    VMUL.I16 q6,q6,q4               //Q6 = Q6 * Q4
    VMUL.I16 q7,q7,q4               //Q7 = Q7 * Q4
    VMUL.I16 q8,q8,q4               //Q8 = Q8 * Q4
    VMUL.I16 q9,q9,q4               //Q9 = Q9 * Q4

    VSHRN.I16 d4,q6,#8              //d4 = Q6.16 shrn 8
    VSHRN.I16 d5,q7,#8              //d5 = Q7.16 shrn 8
    VSHRN.I16 d6,q8,#8              //d6 = Q8.16 shrn 8
    VSHRN.I16 d7,q9,#8              //d7 = Q9.16 shrn 8

    VADD.I8  d4,d4,d0               //d4 = d4+d0
    VADD.I8  d5,d5,d1               //d5 = d5+d1
    VADD.I8  d6,d6,d2               //d6 = d6+d2
    VADD.I8  d7,d7,d3               //d7 = d7+d3

    vst4.8  {d4, d5, d6, d7}, [r0]! //dst rgb(0,1,2,3) A(0,1,2,3) = d4,d5,d6,d7

    //It will be guaranteed to be less than 8
    //BGE      loop
.Lless_than_8:
    VPOP     {Q4-Q7}

.Lless_than_4:
    SUBS     r4,r2,#1
    BMI      .Lto_exit  // S32A_Opaque_BlitRow32_neon2 + 268
    MOV      r8,#0xff
    MVN      r10,#0xff00
    ORR      r9,r8,r8,LSL #16
    LSL      r11,r9,#8
.Lresidual_loop:
    LDR      r3,[r1,#0]
    LDR      r12,[r0,#0]
    ADD      r1,r1,#4
    SUB      r2,r8,r3,LSR #24
    AND      r5,r12,r9
    CMP      r2,r2
    ADD      r2,r2,#1
    AND      r12,r10,r12,LSR #8
    STRNE    r6,[r7,#0xeef]
    MUL      r5,r5,r2
    MUL      r2,r12,r2
    STRNE    r6,[r7,#0xeef]
    SUBS     r4,r4,#1
    AND      r12,r9,r5,LSR #8
    AND      r2,r2,r11
    ORR      r2,r2,r12
    ADD      r2,r2,r3
    STR      r2,[r0],#4
    BPL      .Lresidual_loop  // S32A_Opaque_BlitRow32_neon2 + 192

.Lto_exit:
    POP      {r4-r11}
    BX       lr

.Lless_than_24:
    CMP      r2,#8
    BLT      .Lless_than_4

.Lloop_8:
    SUB      r2,r2,#8
    // We already read the 8 words from the previous pipe line
    vld4.8  {d0, d1, d2, d3}, [r1]! //d0,d1,d2,d3 = sourc rgb(0,1,2,3) A(0,1,2,3)
                                    //update source ptr but not dst ptr
    vld4.8  {d4, d5, d6, d7}, [r0]  //d4,d5,d6,d7 = dst rgb(0,1,2,3) A(0,1,2,3)

    VMOV.I16 q10,#0xff               //Q4.16 = 255
    VSUBW.U8 q10,q10,d3               //Q4.16 = 255-d3

    CMP      r2,#8

    VSHR.U16 q11,q10,#7               //Q5.16 = Q4 >> 7
    VADD.I16 q10,q10,q11               //Q4 = Q4.16+Q5.16 ; A(0,1,2,3)

    VMOVL.U8 q12,d4                  //Q6 = vmovl.u8 d4
    VMOVL.U8 q13,d5                  //Q7 = vmovl.u8 d5
    VMOVL.U8 q8,d6                  //Q8 = vmovl.u8 d6
    VMOVL.U8 q9,d7                  //Q9 = vmovl.u8 d7

    VMUL.I16 q12,q12,q10               //Q6 = Q6 * Q4
    VMUL.I16 q13,q13,q10               //Q7 = Q7 * Q4
    VMUL.I16 q8,q8,q10               //Q8 = Q8 * Q4
    VMUL.I16 q9,q9,q10               //Q9 = Q9 * Q4

    VSHRN.I16 d4,q12,#8              //d4 = Q6.16 shrn 8
    VSHRN.I16 d5,q13,#8              //d5 = Q7.16 shrn 8
    VSHRN.I16 d6,q8,#8              //d6 = Q8.16 shrn 8
    VSHRN.I16 d7,q9,#8              //d7 = Q9.16 shrn 8

    VADD.I8  d4,d4,d0               //d4 = d4+d0
    VADD.I8  d5,d5,d1               //d5 = d5+d1
    VADD.I8  d6,d6,d2               //d6 = d6+d2
    VADD.I8  d7,d7,d3               //d7 = d7+d3

    vst4.8  {d4, d5, d6, d7}, [r0]! //dst rgb(0,1,2,3) A(0,1,2,3) = d4,d5,d6,d7

    BGE      .Lloop_8
    B        .Lless_than_4
.endfunc
.size S32A_Opaque_BlitRow32_neon2, .-S32A_Opaque_BlitRow32_neon2
