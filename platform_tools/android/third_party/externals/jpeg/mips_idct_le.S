#
# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# IDCT implementation using the MIPS DSP ASE (little endian version)
#
# See MIPS Technologies Inc documents:
# "JPEG Decoder Optimization for MIPS32(R) Cores"  MD00483
#
# "MIPS32(R) Architecture for Programmers Volume IV-e: The MIPS(R) DSP
#       Application Specifice Extension to the MIPS32(R) Architecture" MD00374
#

        .set            noreorder
        .set            nomacro
        .set            noat

# This table has been moved to mips_jidctfst.c to avoid having to mess
# with the global pointer to make this code PIC.
#       .rdata
#
# mips_idct_coefs:
#       # Constant table of scaled IDCT coefficients.
#
#       .word           0x45464546              # FIX( 1.082392200 / 2) =  17734 = 0x4546
#       .word           0x5A825A82              # FIX( 1.414213562 / 2) =  23170 = 0x5A82
#       .word           0x76427642              # FIX( 1.847759065 / 2) =  30274 = 0x7642
#       .word           0xAC61AC61              # FIX(-2.613125930 / 4) = -21407 = 0xAC61

        .text

        .global         mips_idct_columns
        .ent            mips_idct_columns

# void mips_idct_columns(JCOEF * inptr, IFAST_MULT_TYPE * quantptr,
#                        DCTELEM * wsptr, const int * mips_idct_coefs);

mips_idct_columns:

# $a0   - inptr
# $a1   - quantptr
# $a2   - wsptr
# $a3, $at   - mips_idct_coefs
# $t0:7 - simd data
# $t8   - coefficients, temp
# $t9   - loop end address
# $s0:3 - simd quantization factors
# $s4:7 - temp results
# $v0:1 - temp results

        addiu           $sp, $sp, -32           # reserve stack space for s0-s7

        sw              $s0, 28($sp)
        sw              $s1, 24($sp)
        sw              $s2, 20($sp)
        sw              $s3, 16($sp)
        sw              $s4, 12($sp)
        sw              $s5,  8($sp)
        sw              $s6,  4($sp)
        sw              $s7,  0($sp)

        addiu           $t9, $a0, 16            # end address

        #lui            $at, %hi(mips_idct_coefs)
        #ori            $at, %lo(mips_idct_coefs)
        # move mips_idct_coefs address from $a3 into $at where the rest of this code expects it
        or              $at, $a3, $zero

loop_columns:

        lw              $s0, 0($a1)             # quantptr[DCTSIZE*0]

        lw              $t0, 0($a0)             # inptr[DCTSIZE*0]
        lw              $t1, 16($a0)            # inptr[DCTSIZE*1]

        muleq_s.w.phl   $v0, $t0, $s0           # tmp0 ...

        lw              $t2, 32($a0)            # inptr[DCTSIZE*2]
        lw              $t3, 48($a0)            # inptr[DCTSIZE*3]
        lw              $t4, 64($a0)            # inptr[DCTSIZE*4]
        lw              $t5, 80($a0)            # inptr[DCTSIZE*5]

        muleq_s.w.phr   $t0, $t0, $s0           # ... tmp0 ...

        lw              $t6, 96($a0)            # inptr[DCTSIZE*6]
        lw              $t7, 112($a0)           # inptr[DCTSIZE*7]

        or              $s4, $t1, $t2
        or              $s5, $t3, $t4

        bnez            $s4, full_column
        ins             $t0, $v0, 16, 16        # ... tmp0

        bnez            $s5, full_column
        or              $s6, $t5, $t6
        or              $s6, $s6, $t7
        bnez            $s6, full_column

        sw              $t0, 0($a2)             # wsptr[DCTSIZE*0]
        sw              $t0, 16($a2)            # wsptr[DCTSIZE*1]
        sw              $t0, 32($a2)            # wsptr[DCTSIZE*2]
        sw              $t0, 48($a2)            # wsptr[DCTSIZE*3]
        sw              $t0, 64($a2)            # wsptr[DCTSIZE*4]
        sw              $t0, 80($a2)            # wsptr[DCTSIZE*5]
        sw              $t0, 96($a2)            # wsptr[DCTSIZE*6]
        sw              $t0, 112($a2)           # wsptr[DCTSIZE*7]

        addiu           $a0, $a0, 4

        b               continue_columns
        addiu           $a1, $a1, 4


full_column:

        lw              $s1, 32($a1)            # quantptr[DCTSIZE*2]
        lw              $s2, 64($a1)            # quantptr[DCTSIZE*4]

        muleq_s.w.phl   $v0, $t2, $s1           # tmp1 ...
        muleq_s.w.phr   $t2, $t2, $s1           # ... tmp1 ...

        lw              $s0, 16($a1)            # quantptr[DCTSIZE*1]
        lw              $s1, 48($a1)            # quantptr[DCTSIZE*3]
        lw              $s3, 96($a1)            # quantptr[DCTSIZE*6]

        muleq_s.w.phl   $v1, $t4, $s2           # tmp2 ...
        muleq_s.w.phr   $t4, $t4, $s2           # ... tmp2 ...

        lw              $s2, 80($a1)            # quantptr[DCTSIZE*5]
        lw              $t8, 4($at)             # FIX(1.414213562)
        ins             $t2, $v0, 16, 16        # ... tmp1

        muleq_s.w.phl   $v0, $t6, $s3           # tmp3 ...
        muleq_s.w.phr   $t6, $t6, $s3           # ... tmp3 ...

        ins             $t4, $v1, 16, 16        # ... tmp2

        addq.ph         $s4, $t0, $t4           # tmp10
        subq.ph         $s5, $t0, $t4           # tmp11

        ins             $t6, $v0, 16, 16        # ... tmp3

        subq.ph         $s6, $t2, $t6           # tmp12 ...
        addq.ph         $s7, $t2, $t6           # tmp13

        mulq_rs.ph      $s6, $s6, $t8           # ... tmp12 ...

        addq.ph         $t0, $s4, $s7           # tmp0
        subq.ph         $t6, $s4, $s7           # tmp3

################

        muleq_s.w.phl   $v0, $t1, $s0           # tmp4 ...
        muleq_s.w.phr   $t1, $t1, $s0           # ... tmp4 ...

        shll_s.ph       $s6, $s6, 1             # x2

        lw              $s3, 112($a1)           # quantptr[DCTSIZE*7]

        subq.ph         $s6, $s6, $s7           # ... tmp12

        muleq_s.w.phl   $v1, $t7, $s3           # tmp7 ...
        muleq_s.w.phr   $t7, $t7, $s3           # ... tmp7 ...

        ins             $t1, $v0, 16, 16        # ... tmp4

        addq.ph         $t2, $s5, $s6           # tmp1
        subq.ph         $t4, $s5, $s6           # tmp2

        muleq_s.w.phl   $v0, $t5, $s2           # tmp6 ...
        muleq_s.w.phr   $t5, $t5, $s2           # ... tmp6 ...

        ins             $t7, $v1, 16, 16        # ... tmp7

        addq.ph         $s5, $t1, $t7           # z11
        subq.ph         $s6, $t1, $t7           # z12

        muleq_s.w.phl   $v1, $t3, $s1           # tmp5 ...
        muleq_s.w.phr   $t3, $t3, $s1           # ... tmp5 ...

        ins             $t5, $v0, 16, 16        # ... tmp6

# stalls

        ins             $t3, $v1, 16, 16        # ... tmp5


        addq.ph         $s7, $t5, $t3           # z13
        subq.ph         $v0, $t5, $t3           # z10

        addq.ph         $t7, $s5, $s7           # tmp7
        subq.ph         $s5, $s5, $s7           # tmp11 ...

        addq.ph         $v1, $v0, $s6           # z5 ...

        mulq_rs.ph      $s5, $s5, $t8           # ... tmp11

        lw              $t8, 8($at)             # FIX(1.847759065)
        lw              $s4, 0($at)             # FIX(1.082392200)

        addq.ph         $s0, $t0, $t7
        subq.ph         $s1, $t0, $t7

        mulq_rs.ph      $v1, $v1, $t8           # ... z5

        shll_s.ph       $s5, $s5, 1             # x2

        lw              $t8, 12($at)            # FIX(-2.613125930)
        sw              $s0, 0($a2)             # wsptr[DCTSIZE*0]

        mulq_rs.ph      $v0, $v0, $t8           # tmp12 ...
        mulq_rs.ph      $s4, $s6, $s4           # tmp10 ...

        shll_s.ph       $v1, $v1, 1             # x2

        addiu           $a0, $a0, 4
        addiu           $a1, $a1, 4

        sw              $s1, 112($a2)           # wsptr[DCTSIZE*7]

        shll_s.ph       $s6, $v0, 2             # x4
        shll_s.ph       $s4, $s4, 1             # x2
        addq.ph         $s6, $s6, $v1           # ... tmp12

        subq.ph         $t5, $s6, $t7           # tmp6
        subq.ph         $s4, $s4, $v1           # ... tmp10
        subq.ph         $t3, $s5, $t5           # tmp5
        addq.ph         $s2, $t2, $t5
        addq.ph         $t1, $s4, $t3           # tmp4
        subq.ph         $s3, $t2, $t5

        sw              $s2, 16($a2)            # wsptr[DCTSIZE*1]
        sw              $s3, 96($a2)            # wsptr[DCTSIZE*6]

        addq.ph         $v0, $t4, $t3
        subq.ph         $v1, $t4, $t3

        sw              $v0, 32($a2)            # wsptr[DCTSIZE*2]
        sw              $v1, 80($a2)            # wsptr[DCTSIZE*5]

        addq.ph         $v0, $t6, $t1
        subq.ph         $v1, $t6, $t1

        sw              $v0, 64($a2)            # wsptr[DCTSIZE*4]
        sw              $v1, 48($a2)            # wsptr[DCTSIZE*3]

continue_columns:

        bne             $a0, $t9, loop_columns
        addiu           $a2, $a2, 4


        lw              $s0, 28($sp)
        lw              $s1, 24($sp)
        lw              $s2, 20($sp)
        lw              $s3, 16($sp)
        lw              $s4, 12($sp)
        lw              $s5,  8($sp)
        lw              $s6,  4($sp)
        lw              $s7,  0($sp)

        jr              $ra
        addiu           $sp, $sp, 32


        .end            mips_idct_columns


##################################################################


        .global         mips_idct_rows
        .ent            mips_idct_rows

# void mips_idct_rows(DCTELEM * wsptr, JSAMPARRAY output_buf,
#                     JDIMENSION output_col, const int * mips_idct_coefs);

mips_idct_rows:

# $a0   - wsptr
# $a1   - output_buf
# $a2   - output_col
# $a3   - outptr
# $a3, $at   - mips_idct_coefs
# $t0:7 - simd data
# $t8   - coefficients, temp
# $t9   - loop end address
# $s0:3 - simd quantization factors
# $s4:7 - temp results
# s8    - const 0x80808080
# $v0:1 - temp results

SHIFT   =               2

        addiu           $sp, $sp, -48           # reserve stack space for s0-s8

        # save $a3 (mips_idct_coefs) because it might get clobbered below
        sw              $a3, 36($sp)

        sw              $s0, 32($sp)
        sw              $s1, 28($sp)
        sw              $s2, 24($sp)
        sw              $s3, 20($sp)
        sw              $s4, 16($sp)
        sw              $s5, 12($sp)
        sw              $s6,  8($sp)
        sw              $s7,  4($sp)
        sw              $s8,  0($sp)

        addiu           $t9, $a0, 128           # end address

        lui             $s8, 0x8080
        ori             $s8, $s8, 0x8080

loop_rows:

        lw              $at, 36($sp)            # restore saved $a3 (mips_idct_coefs)

        lw              $t0, 0+0($a0)           # wsptr[DCTSIZE*0+0/1]  b a
        lw              $s0, 16+0($a0)          # wsptr[DCTSIZE*1+0/1]  B A
        lw              $t2, 0+4($a0)           # wsptr[DCTSIZE*0+2/3]  d c
        lw              $s2, 16+4($a0)          # wsptr[DCTSIZE*1+2/3]  D C
        lw              $t4, 0+8($a0)           # wsptr[DCTSIZE*0+4/5]  f e
        lw              $s4, 16+8($a0)          # wsptr[DCTSIZE*1+4/5]  F E
        lw              $t6, 0+12($a0)          # wsptr[DCTSIZE*0+6/7]  h g
        lw              $s6, 16+12($a0)         # wsptr[DCTSIZE*1+6/7]  H G

        precrq.ph.w     $t1, $s0, $t0           # B b
        ins             $t0, $s0, 16, 16        # A a

        bnez            $t1, full_row
        or              $s0, $t2, $s2
        bnez            $s0, full_row
        or              $s0, $t4, $s4
        bnez            $s0, full_row
        or              $s0, $t6, $s6
        bnez            $s0, full_row

        shll_s.ph       $s0, $t0, SHIFT         # A a

        lw              $a3, 0($a1)
        lw              $at, 4($a1)

        precrq.ph.w     $t0, $s0, $s0           # A A
        ins             $s0, $s0, 16, 16        # a a

        addu            $a3, $a3, $a2
        addu            $at, $at, $a2

        precrq.qb.ph    $t0, $t0, $t0           # A A A A
        precrq.qb.ph    $s0, $s0, $s0           # a a a a


        addu.qb         $s0, $s0, $s8
        addu.qb         $t0, $t0, $s8


        sw              $s0, 0($a3)
        sw              $s0, 4($a3)

        sw              $t0, 0($at)
        sw              $t0, 4($at)


        addiu           $a0, $a0, 32

        bne             $a0, $t9, loop_rows
        addiu           $a1, $a1, 8

        b               exit_rows
        nop


full_row:

        precrq.ph.w     $t3, $s2, $t2
        ins             $t2, $s2, 16, 16

        precrq.ph.w     $t5, $s4, $t4
        ins             $t4, $s4, 16, 16

        precrq.ph.w     $t7, $s6, $t6
        ins             $t6, $s6, 16, 16


        lw              $t8, 4($at)             # FIX(1.414213562)

        addq.ph         $s4, $t0, $t4           # tmp10
        subq.ph         $s5, $t0, $t4           # tmp11

        subq.ph         $s6, $t2, $t6           # tmp12 ...
        addq.ph         $s7, $t2, $t6           # tmp13

        mulq_rs.ph      $s6, $s6, $t8           # ... tmp12 ...

        addq.ph         $t0, $s4, $s7           # tmp0
        subq.ph         $t6, $s4, $s7           # tmp3

        shll_s.ph       $s6, $s6, 1             # x2

        subq.ph         $s6, $s6, $s7           # ... tmp12

        addq.ph         $t2, $s5, $s6           # tmp1
        subq.ph         $t4, $s5, $s6           # tmp2

################

        addq.ph         $s5, $t1, $t7           # z11
        subq.ph         $s6, $t1, $t7           # z12

        addq.ph         $s7, $t5, $t3           # z13
        subq.ph         $v0, $t5, $t3           # z10

        addq.ph         $t7, $s5, $s7           # tmp7
        subq.ph         $s5, $s5, $s7           # tmp11 ...

        addq.ph         $v1, $v0, $s6           # z5 ...

        mulq_rs.ph      $s5, $s5, $t8           # ... tmp11

        lw              $t8, 8($at)             # FIX(1.847759065)
        lw              $s4, 0($at)             # FIX(1.082392200)

        addq.ph         $s0, $t0, $t7           # tmp0 + tmp7
        subq.ph         $s7, $t0, $t7           # tmp0 - tmp7

        mulq_rs.ph      $v1, $v1, $t8           # ... z5

        lw              $a3, 0($a1)
        lw              $t8, 12($at)            # FIX(-2.613125930)

        shll_s.ph       $s5, $s5, 1             # x2

        addu            $a3, $a3, $a2

        mulq_rs.ph      $v0, $v0, $t8           # tmp12 ...
        mulq_rs.ph      $s4, $s6, $s4           # tmp10 ...

        shll_s.ph       $v1, $v1, 1             # x2

        addiu           $a0, $a0, 32
        addiu           $a1, $a1, 8


        shll_s.ph       $s6, $v0, 2             # x4
        shll_s.ph       $s4, $s4, 1             # x2
        addq.ph         $s6, $s6, $v1           # ... tmp12

        shll_s.ph       $s0, $s0, SHIFT

        subq.ph         $t5, $s6, $t7           # tmp6
        subq.ph         $s4, $s4, $v1           # ... tmp10
        subq.ph         $t3, $s5, $t5           # tmp5

        shll_s.ph       $s7, $s7, SHIFT

        addq.ph         $t1, $s4, $t3           # tmp4


        addq.ph         $s1, $t2, $t5           # tmp1 + tmp6
        subq.ph         $s6, $t2, $t5           # tmp1 - tmp6

        addq.ph         $s2, $t4, $t3           # tmp2 + tmp5
        subq.ph         $s5, $t4, $t3           # tmp2 - tmp5

        addq.ph         $s4, $t6, $t1           # tmp3 + tmp4
        subq.ph         $s3, $t6, $t1           # tmp3 - tmp4


        shll_s.ph       $s1, $s1, SHIFT
        shll_s.ph       $s2, $s2, SHIFT
        shll_s.ph       $s3, $s3, SHIFT
        shll_s.ph       $s4, $s4, SHIFT
        shll_s.ph       $s5, $s5, SHIFT
        shll_s.ph       $s6, $s6, SHIFT


        precrq.ph.w     $t0, $s1, $s0           # B A
        ins             $s0, $s1, 16, 16        # b a

        precrq.ph.w     $t2, $s3, $s2           # D C
        ins             $s2, $s3, 16, 16        # d c

        precrq.ph.w     $t4, $s5, $s4           # F E
        ins             $s4, $s5, 16, 16        # f e

        precrq.ph.w     $t6, $s7, $s6           # H G
        ins             $s6, $s7, 16, 16        # h g

        precrq.qb.ph    $t0, $t2, $t0           # D C B A
        precrq.qb.ph    $s0, $s2, $s0           # d c b a

        precrq.qb.ph    $t4, $t6, $t4           # H G F E
        precrq.qb.ph    $s4, $s6, $s4           # h g f e


        addu.qb         $s0, $s0, $s8
        addu.qb         $s4, $s4, $s8


        sw              $s0, 0($a3)             # outptr[0/1/2/3]       d c b a
        sw              $s4, 4($a3)             # outptr[4/5/6/7]       h g f e

        lw              $a3, -4($a1)

        addu.qb         $t0, $t0, $s8

        addu            $a3, $a3, $a2

        addu.qb         $t4, $t4, $s8


        sw              $t0, 0($a3)             # outptr[0/1/2/3]       D C B A

        bne             $a0, $t9, loop_rows
        sw              $t4, 4($a3)             # outptr[4/5/6/7]       H G F E


exit_rows:

        lw              $s0, 32($sp)
        lw              $s1, 28($sp)
        lw              $s2, 24($sp)
        lw              $s3, 20($sp)
        lw              $s4, 16($sp)
        lw              $s5, 12($sp)
        lw              $s6,  8($sp)
        lw              $s7,  4($sp)
        lw              $s8,  0($sp)

        jr              $ra
        addiu           $sp, $sp, 48


        .end            mips_idct_rows
