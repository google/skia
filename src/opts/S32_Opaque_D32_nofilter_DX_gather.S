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
    .global S32_Opaque_D32_nofilter_DX_gather_neon
    .func S32_Opaque_D32_nofilter_DX_gather_neon
S32_Opaque_D32_nofilter_DX_gather_neon:
// void S32_Opaque_D32_nofilter_DX_gather_neon(
//    SkPMColor* SK_RESTRICT colors,
//    const SkPMColor* SK_RESTRICT srcAddr,
//    int count,
//    const uint32_t* SK_RESTRICT xy);
// r0: destination color buffer
// r1: src color buffer
// r2: count
// r3: index array
//
// This function will read indexes from index array xy, and then read srcAddr
// with the index, and put the read-out value into dest color buffer.  In
// another word, it will "gather" the value from srcAddr, indexed by xy, into
// dest array colors.
//
// We tried a version using Neon instructions.  We also tried to hand
// optimize the compiler generated assembly (non-neon) code.  The latter seems
// to have better gain:  we get about 6% improvements

          PUSH     {r0-r11,lr}
          ASR      r0,r2,#3
          SUB      sp,sp,#4              //23
          CMP      r0,#0
          STR      r0,[sp,#0] //r0 = count >> 3
          BLE      .L1_140
          LDR      r4,[sp,#4] //r4 = r0 (dst)
          MOV      r0,r3
          ADD      r12,r3,#4
          ASR      r8,r2,#3
.L1_52:
          LDM      r3!, {r0,r6,r9,r11}
          LSR      r5,r0,#16            //30
          LDR      r5,[r1,r5,LSL #2]   //30
          LSR      r7,r6,#16            //32
          LDR      r7,[r1,r7,LSL #2]     //31
          UXTH     r0,r0                 //34
          LDR      r0,[r1,r0,LSL #2]     //34
          UXTH     r6,r6                 //31
          LDR      r6,[r1,r6,LSL #2]   //32
          //STM      r4!, {r0,r5,r6,r7}         ;35
          LSR      r10,r9,#16            //30
          LDR      r10,[r1,r10,LSL #2]   //30
          LSR      lr,r11,#16            //32
          LDR      lr,[r1,lr,LSL #2]     //31
          UXTH     r9,r9                 //34
          LDR      r9,[r1,r9,LSL #2]     //34
          UXTH     r11,r11                 //31
          LDR      r11,[r1,r11,LSL #2]   //32
          SUBS     r8,r8,#1
          STM      r4!, {r0,r5,r6,r7,r9,r10,r11,lr}         //35

          BNE      .L1_52

          LDR      r0,[sp,#0]  // count >> 3
          MOV      r12,r0
          LDR      r0,[sp,#4]  //r0 = dst
          ADD      r0,r0,r12,LSL #5 //dst += count >>3 << 5
          STR      r0,[sp,#4]  //save r0 into stack again
.L1_140:
//;;39         const uint16_t* SK_RESTRICT xx = (const uint16_t*)(xy);
//;;40         for (i = (count & 7); i > 0; --i) {
          TST      r2,#7
          BEQ      .L1_184
          LDR      r0,[sp,#4]  //r0 = currnt dst
          AND      r2,r2,#7
.L1_156:
//;;41             //SkASSERT(*xx < (unsigned)s.fBitmap->width());
//;;42             src = srcAddr[*xx++]; *colors++ = RETURNDST(src);
          LDRH     r4,[r3],#2
          ADD      r12,r0,#4
//;;43         }
          SUBS     r2,r2,#1
          LDR      r4,[r1,r4,LSL #2]     //42
          STR      r4,[r0,#0]            //42
          MOV      r0,r12                //42
          BNE      .L1_156
.L1_184:
//;;44     }
          ADD      sp,sp,#0x14
          POP      {r4-r11,pc}

.endfunc
.size S32_Opaque_D32_nofilter_DX_gather_neon, .-S32_Opaque_D32_nofilter_DX_gather_neon

//    ENDFUNC
//    END
