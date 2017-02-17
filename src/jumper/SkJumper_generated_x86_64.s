# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is generated semi-automatically with this command:
#   $ src/jumper/build_stages.py

.text

.globl _sk_start_pipeline_hsw
_sk_start_pipeline_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xfc,0x57,0xc0                             # vxorps        %ymm0,%ymm0,%ymm0
  .byte  0xc5,0xf4,0x57,0xc9                             # vxorps        %ymm1,%ymm1,%ymm1
  .byte  0xc5,0xec,0x57,0xd2                             # vxorps        %ymm2,%ymm2,%ymm2
  .byte  0xc5,0xe4,0x57,0xdb                             # vxorps        %ymm3,%ymm3,%ymm3
  .byte  0xc5,0xdc,0x57,0xe4                             # vxorps        %ymm4,%ymm4,%ymm4
  .byte  0xc5,0xd4,0x57,0xed                             # vxorps        %ymm5,%ymm5,%ymm5
  .byte  0xc5,0xcc,0x57,0xf6                             # vxorps        %ymm6,%ymm6,%ymm6
  .byte  0xc5,0xc4,0x57,0xff                             # vxorps        %ymm7,%ymm7,%ymm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_start_pipeline_ms_hsw
_sk_start_pipeline_ms_hsw:
  .byte  0x56                                            # push          %rsi
  .byte  0x57                                            # push          %rdi
  .byte  0x48,0x81,0xec,0xa8,0x00,0x00,0x00              # sub           $0xa8,%rsp
  .byte  0xc5,0x78,0x29,0xbc,0x24,0x90,0x00,0x00,0x00    # vmovaps       %xmm15,0x90(%rsp)
  .byte  0xc5,0x78,0x29,0xb4,0x24,0x80,0x00,0x00,0x00    # vmovaps       %xmm14,0x80(%rsp)
  .byte  0xc5,0x78,0x29,0x6c,0x24,0x70                   # vmovaps       %xmm13,0x70(%rsp)
  .byte  0xc5,0x78,0x29,0x64,0x24,0x60                   # vmovaps       %xmm12,0x60(%rsp)
  .byte  0xc5,0x78,0x29,0x5c,0x24,0x50                   # vmovaps       %xmm11,0x50(%rsp)
  .byte  0xc5,0x78,0x29,0x54,0x24,0x40                   # vmovaps       %xmm10,0x40(%rsp)
  .byte  0xc5,0x78,0x29,0x4c,0x24,0x30                   # vmovaps       %xmm9,0x30(%rsp)
  .byte  0xc5,0x78,0x29,0x44,0x24,0x20                   # vmovaps       %xmm8,0x20(%rsp)
  .byte  0xc5,0xf8,0x29,0x7c,0x24,0x10                   # vmovaps       %xmm7,0x10(%rsp)
  .byte  0xc5,0xf8,0x29,0x34,0x24                        # vmovaps       %xmm6,(%rsp)
  .byte  0x48,0x89,0xd6                                  # mov           %rdx,%rsi
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xfc,0x57,0xc0                             # vxorps        %ymm0,%ymm0,%ymm0
  .byte  0xc5,0xf4,0x57,0xc9                             # vxorps        %ymm1,%ymm1,%ymm1
  .byte  0xc5,0xec,0x57,0xd2                             # vxorps        %ymm2,%ymm2,%ymm2
  .byte  0xc5,0xe4,0x57,0xdb                             # vxorps        %ymm3,%ymm3,%ymm3
  .byte  0xc5,0xdc,0x57,0xe4                             # vxorps        %ymm4,%ymm4,%ymm4
  .byte  0xc5,0xd4,0x57,0xed                             # vxorps        %ymm5,%ymm5,%ymm5
  .byte  0xc5,0xcc,0x57,0xf6                             # vxorps        %ymm6,%ymm6,%ymm6
  .byte  0xc5,0xc4,0x57,0xff                             # vxorps        %ymm7,%ymm7,%ymm7
  .byte  0x48,0x89,0xcf                                  # mov           %rcx,%rdi
  .byte  0x4c,0x89,0xc2                                  # mov           %r8,%rdx
  .byte  0xff,0xd0                                       # callq         *%rax
  .byte  0xc5,0xf8,0x28,0x34,0x24                        # vmovaps       (%rsp),%xmm6
  .byte  0xc5,0xf8,0x28,0x7c,0x24,0x10                   # vmovaps       0x10(%rsp),%xmm7
  .byte  0xc5,0x78,0x28,0x44,0x24,0x20                   # vmovaps       0x20(%rsp),%xmm8
  .byte  0xc5,0x78,0x28,0x4c,0x24,0x30                   # vmovaps       0x30(%rsp),%xmm9
  .byte  0xc5,0x78,0x28,0x54,0x24,0x40                   # vmovaps       0x40(%rsp),%xmm10
  .byte  0xc5,0x78,0x28,0x5c,0x24,0x50                   # vmovaps       0x50(%rsp),%xmm11
  .byte  0xc5,0x78,0x28,0x64,0x24,0x60                   # vmovaps       0x60(%rsp),%xmm12
  .byte  0xc5,0x78,0x28,0x6c,0x24,0x70                   # vmovaps       0x70(%rsp),%xmm13
  .byte  0xc5,0x78,0x28,0xb4,0x24,0x80,0x00,0x00,0x00    # vmovaps       0x80(%rsp),%xmm14
  .byte  0xc5,0x78,0x28,0xbc,0x24,0x90,0x00,0x00,0x00    # vmovaps       0x90(%rsp),%xmm15
  .byte  0x48,0x81,0xc4,0xa8,0x00,0x00,0x00              # add           $0xa8,%rsp
  .byte  0x5f                                            # pop           %rdi
  .byte  0x5e                                            # pop           %rsi
  .byte  0xc5,0xf8,0x77                                  # vzeroupper
  .byte  0xc3                                            # retq

.globl _sk_just_return_hsw
_sk_just_return_hsw:
  .byte  0xc5,0xf8,0x77                                  # vzeroupper
  .byte  0xc3                                            # retq

.globl _sk_seed_shader_hsw
_sk_seed_shader_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xf9,0x6e,0xc7                             # vmovd         %edi,%xmm0
  .byte  0xc4,0xe2,0x7d,0x18,0xc0                        # vbroadcastss  %xmm0,%ymm0
  .byte  0xc5,0xfc,0x5b,0xc0                             # vcvtdq2ps     %ymm0,%ymm0
  .byte  0xc4,0xe2,0x7d,0x18,0x4a,0x04                   # vbroadcastss  0x4(%rdx),%ymm1
  .byte  0xc5,0xfc,0x58,0xc1                             # vaddps        %ymm1,%ymm0,%ymm0
  .byte  0xc5,0xfc,0x58,0x42,0x14                        # vaddps        0x14(%rdx),%ymm0,%ymm0
  .byte  0xc4,0xe2,0x7d,0x18,0x10                        # vbroadcastss  (%rax),%ymm2
  .byte  0xc5,0xfc,0x5b,0xd2                             # vcvtdq2ps     %ymm2,%ymm2
  .byte  0xc5,0xec,0x58,0xc9                             # vaddps        %ymm1,%ymm2,%ymm1
  .byte  0xc4,0xe2,0x7d,0x18,0x12                        # vbroadcastss  (%rdx),%ymm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xe4,0x57,0xdb                             # vxorps        %ymm3,%ymm3,%ymm3
  .byte  0xc5,0xdc,0x57,0xe4                             # vxorps        %ymm4,%ymm4,%ymm4
  .byte  0xc5,0xd4,0x57,0xed                             # vxorps        %ymm5,%ymm5,%ymm5
  .byte  0xc5,0xcc,0x57,0xf6                             # vxorps        %ymm6,%ymm6,%ymm6
  .byte  0xc5,0xc4,0x57,0xff                             # vxorps        %ymm7,%ymm7,%ymm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_constant_color_hsw
_sk_constant_color_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0xe2,0x7d,0x18,0x00                        # vbroadcastss  (%rax),%ymm0
  .byte  0xc4,0xe2,0x7d,0x18,0x48,0x04                   # vbroadcastss  0x4(%rax),%ymm1
  .byte  0xc4,0xe2,0x7d,0x18,0x50,0x08                   # vbroadcastss  0x8(%rax),%ymm2
  .byte  0xc4,0xe2,0x7d,0x18,0x58,0x0c                   # vbroadcastss  0xc(%rax),%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clear_hsw
_sk_clear_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xfc,0x57,0xc0                             # vxorps        %ymm0,%ymm0,%ymm0
  .byte  0xc5,0xf4,0x57,0xc9                             # vxorps        %ymm1,%ymm1,%ymm1
  .byte  0xc5,0xec,0x57,0xd2                             # vxorps        %ymm2,%ymm2,%ymm2
  .byte  0xc5,0xe4,0x57,0xdb                             # vxorps        %ymm3,%ymm3,%ymm3
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_plus__hsw
_sk_plus__hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xfc,0x58,0xc4                             # vaddps        %ymm4,%ymm0,%ymm0
  .byte  0xc5,0xf4,0x58,0xcd                             # vaddps        %ymm5,%ymm1,%ymm1
  .byte  0xc5,0xec,0x58,0xd6                             # vaddps        %ymm6,%ymm2,%ymm2
  .byte  0xc5,0xe4,0x58,0xdf                             # vaddps        %ymm7,%ymm3,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_srcover_hsw
_sk_srcover_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x62,0x7d,0x18,0x02                        # vbroadcastss  (%rdx),%ymm8
  .byte  0xc5,0x3c,0x5c,0xc3                             # vsubps        %ymm3,%ymm8,%ymm8
  .byte  0xc4,0xc2,0x5d,0xb8,0xc0                        # vfmadd231ps   %ymm8,%ymm4,%ymm0
  .byte  0xc4,0xc2,0x55,0xb8,0xc8                        # vfmadd231ps   %ymm8,%ymm5,%ymm1
  .byte  0xc4,0xc2,0x4d,0xb8,0xd0                        # vfmadd231ps   %ymm8,%ymm6,%ymm2
  .byte  0xc4,0xc2,0x45,0xb8,0xd8                        # vfmadd231ps   %ymm8,%ymm7,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_dstover_hsw
_sk_dstover_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x62,0x7d,0x18,0x02                        # vbroadcastss  (%rdx),%ymm8
  .byte  0xc5,0x3c,0x5c,0xc7                             # vsubps        %ymm7,%ymm8,%ymm8
  .byte  0xc4,0xe2,0x3d,0xa8,0xc4                        # vfmadd213ps   %ymm4,%ymm8,%ymm0
  .byte  0xc4,0xe2,0x3d,0xa8,0xcd                        # vfmadd213ps   %ymm5,%ymm8,%ymm1
  .byte  0xc4,0xe2,0x3d,0xa8,0xd6                        # vfmadd213ps   %ymm6,%ymm8,%ymm2
  .byte  0xc4,0xe2,0x3d,0xa8,0xdf                        # vfmadd213ps   %ymm7,%ymm8,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_0_hsw
_sk_clamp_0_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x41,0x3c,0x57,0xc0                        # vxorps        %ymm8,%ymm8,%ymm8
  .byte  0xc4,0xc1,0x7c,0x5f,0xc0                        # vmaxps        %ymm8,%ymm0,%ymm0
  .byte  0xc4,0xc1,0x74,0x5f,0xc8                        # vmaxps        %ymm8,%ymm1,%ymm1
  .byte  0xc4,0xc1,0x6c,0x5f,0xd0                        # vmaxps        %ymm8,%ymm2,%ymm2
  .byte  0xc4,0xc1,0x64,0x5f,0xd8                        # vmaxps        %ymm8,%ymm3,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_1_hsw
_sk_clamp_1_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x62,0x7d,0x18,0x02                        # vbroadcastss  (%rdx),%ymm8
  .byte  0xc4,0xc1,0x7c,0x5d,0xc0                        # vminps        %ymm8,%ymm0,%ymm0
  .byte  0xc4,0xc1,0x74,0x5d,0xc8                        # vminps        %ymm8,%ymm1,%ymm1
  .byte  0xc4,0xc1,0x6c,0x5d,0xd0                        # vminps        %ymm8,%ymm2,%ymm2
  .byte  0xc4,0xc1,0x64,0x5d,0xd8                        # vminps        %ymm8,%ymm3,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_a_hsw
_sk_clamp_a_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x62,0x7d,0x18,0x02                        # vbroadcastss  (%rdx),%ymm8
  .byte  0xc4,0xc1,0x64,0x5d,0xd8                        # vminps        %ymm8,%ymm3,%ymm3
  .byte  0xc5,0xfc,0x5d,0xc3                             # vminps        %ymm3,%ymm0,%ymm0
  .byte  0xc5,0xf4,0x5d,0xcb                             # vminps        %ymm3,%ymm1,%ymm1
  .byte  0xc5,0xec,0x5d,0xd3                             # vminps        %ymm3,%ymm2,%ymm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_swap_hsw
_sk_swap_hsw:
  .byte  0xc5,0x7c,0x28,0xc3                             # vmovaps       %ymm3,%ymm8
  .byte  0xc5,0x7c,0x28,0xca                             # vmovaps       %ymm2,%ymm9
  .byte  0xc5,0x7c,0x28,0xd1                             # vmovaps       %ymm1,%ymm10
  .byte  0xc5,0x7c,0x28,0xd8                             # vmovaps       %ymm0,%ymm11
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xfc,0x28,0xc4                             # vmovaps       %ymm4,%ymm0
  .byte  0xc5,0xfc,0x28,0xcd                             # vmovaps       %ymm5,%ymm1
  .byte  0xc5,0xfc,0x28,0xd6                             # vmovaps       %ymm6,%ymm2
  .byte  0xc5,0xfc,0x28,0xdf                             # vmovaps       %ymm7,%ymm3
  .byte  0xc5,0x7c,0x29,0xdc                             # vmovaps       %ymm11,%ymm4
  .byte  0xc5,0x7c,0x29,0xd5                             # vmovaps       %ymm10,%ymm5
  .byte  0xc5,0x7c,0x29,0xce                             # vmovaps       %ymm9,%ymm6
  .byte  0xc5,0x7c,0x29,0xc7                             # vmovaps       %ymm8,%ymm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_move_src_dst_hsw
_sk_move_src_dst_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xfc,0x28,0xe0                             # vmovaps       %ymm0,%ymm4
  .byte  0xc5,0xfc,0x28,0xe9                             # vmovaps       %ymm1,%ymm5
  .byte  0xc5,0xfc,0x28,0xf2                             # vmovaps       %ymm2,%ymm6
  .byte  0xc5,0xfc,0x28,0xfb                             # vmovaps       %ymm3,%ymm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_move_dst_src_hsw
_sk_move_dst_src_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xfc,0x28,0xc4                             # vmovaps       %ymm4,%ymm0
  .byte  0xc5,0xfc,0x28,0xcd                             # vmovaps       %ymm5,%ymm1
  .byte  0xc5,0xfc,0x28,0xd6                             # vmovaps       %ymm6,%ymm2
  .byte  0xc5,0xfc,0x28,0xdf                             # vmovaps       %ymm7,%ymm3
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_premul_hsw
_sk_premul_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0xfc,0x59,0xc3                             # vmulps        %ymm3,%ymm0,%ymm0
  .byte  0xc5,0xf4,0x59,0xcb                             # vmulps        %ymm3,%ymm1,%ymm1
  .byte  0xc5,0xec,0x59,0xd3                             # vmulps        %ymm3,%ymm2,%ymm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_unpremul_hsw
_sk_unpremul_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x41,0x3c,0x57,0xc0                        # vxorps        %ymm8,%ymm8,%ymm8
  .byte  0xc4,0x41,0x64,0xc2,0xc8,0x00                   # vcmpeqps      %ymm8,%ymm3,%ymm9
  .byte  0xc4,0x62,0x7d,0x18,0x12                        # vbroadcastss  (%rdx),%ymm10
  .byte  0xc5,0x2c,0x5e,0xd3                             # vdivps        %ymm3,%ymm10,%ymm10
  .byte  0xc4,0x43,0x2d,0x4a,0xc0,0x90                   # vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  .byte  0xc5,0xbc,0x59,0xc0                             # vmulps        %ymm0,%ymm8,%ymm0
  .byte  0xc5,0xbc,0x59,0xc9                             # vmulps        %ymm1,%ymm8,%ymm1
  .byte  0xc5,0xbc,0x59,0xd2                             # vmulps        %ymm2,%ymm8,%ymm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_from_srgb_hsw
_sk_from_srgb_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x62,0x7d,0x18,0x42,0x40                   # vbroadcastss  0x40(%rdx),%ymm8
  .byte  0xc5,0x3c,0x59,0xc8                             # vmulps        %ymm0,%ymm8,%ymm9
  .byte  0xc5,0x7c,0x59,0xd0                             # vmulps        %ymm0,%ymm0,%ymm10
  .byte  0xc4,0x62,0x7d,0x18,0x5a,0x3c                   # vbroadcastss  0x3c(%rdx),%ymm11
  .byte  0xc4,0x62,0x7d,0x18,0x62,0x38                   # vbroadcastss  0x38(%rdx),%ymm12
  .byte  0xc4,0x41,0x7c,0x28,0xeb                        # vmovaps       %ymm11,%ymm13
  .byte  0xc4,0x42,0x7d,0xa8,0xec                        # vfmadd213ps   %ymm12,%ymm0,%ymm13
  .byte  0xc4,0x62,0x7d,0x18,0x72,0x34                   # vbroadcastss  0x34(%rdx),%ymm14
  .byte  0xc4,0x42,0x2d,0xa8,0xee                        # vfmadd213ps   %ymm14,%ymm10,%ymm13
  .byte  0xc4,0x62,0x7d,0x18,0x52,0x44                   # vbroadcastss  0x44(%rdx),%ymm10
  .byte  0xc4,0xc1,0x7c,0xc2,0xc2,0x01                   # vcmpltps      %ymm10,%ymm0,%ymm0
  .byte  0xc4,0xc3,0x15,0x4a,0xc1,0x00                   # vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
  .byte  0xc5,0x3c,0x59,0xc9                             # vmulps        %ymm1,%ymm8,%ymm9
  .byte  0xc5,0x74,0x59,0xe9                             # vmulps        %ymm1,%ymm1,%ymm13
  .byte  0xc4,0x41,0x7c,0x28,0xfb                        # vmovaps       %ymm11,%ymm15
  .byte  0xc4,0x42,0x75,0xa8,0xfc                        # vfmadd213ps   %ymm12,%ymm1,%ymm15
  .byte  0xc4,0x42,0x15,0xa8,0xfe                        # vfmadd213ps   %ymm14,%ymm13,%ymm15
  .byte  0xc4,0xc1,0x74,0xc2,0xca,0x01                   # vcmpltps      %ymm10,%ymm1,%ymm1
  .byte  0xc4,0xc3,0x05,0x4a,0xc9,0x10                   # vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
  .byte  0xc5,0x3c,0x59,0xc2                             # vmulps        %ymm2,%ymm8,%ymm8
  .byte  0xc5,0x6c,0x59,0xca                             # vmulps        %ymm2,%ymm2,%ymm9
  .byte  0xc4,0x42,0x6d,0xa8,0xdc                        # vfmadd213ps   %ymm12,%ymm2,%ymm11
  .byte  0xc4,0x42,0x35,0xa8,0xde                        # vfmadd213ps   %ymm14,%ymm9,%ymm11
  .byte  0xc4,0xc1,0x6c,0xc2,0xd2,0x01                   # vcmpltps      %ymm10,%ymm2,%ymm2
  .byte  0xc4,0xc3,0x25,0x4a,0xd0,0x20                   # vblendvps     %ymm2,%ymm8,%ymm11,%ymm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_to_srgb_hsw
_sk_to_srgb_hsw:
  .byte  0xc5,0x7c,0x52,0xc0                             # vrsqrtps      %ymm0,%ymm8
  .byte  0xc4,0x41,0x7c,0x53,0xc8                        # vrcpps        %ymm8,%ymm9
  .byte  0xc4,0x41,0x7c,0x52,0xd0                        # vrsqrtps      %ymm8,%ymm10
  .byte  0xc4,0x62,0x7d,0x18,0x42,0x48                   # vbroadcastss  0x48(%rdx),%ymm8
  .byte  0xc5,0x3c,0x59,0xd8                             # vmulps        %ymm0,%ymm8,%ymm11
  .byte  0xc4,0x62,0x7d,0x18,0x22                        # vbroadcastss  (%rdx),%ymm12
  .byte  0xc4,0x62,0x7d,0x18,0x6a,0x4c                   # vbroadcastss  0x4c(%rdx),%ymm13
  .byte  0xc4,0x62,0x7d,0x18,0x72,0x50                   # vbroadcastss  0x50(%rdx),%ymm14
  .byte  0xc4,0x62,0x7d,0x18,0x7a,0x54                   # vbroadcastss  0x54(%rdx),%ymm15
  .byte  0xc4,0x42,0x0d,0xa8,0xcf                        # vfmadd213ps   %ymm15,%ymm14,%ymm9
  .byte  0xc4,0x42,0x15,0xb8,0xca                        # vfmadd231ps   %ymm10,%ymm13,%ymm9
  .byte  0xc4,0x41,0x1c,0x5d,0xc9                        # vminps        %ymm9,%ymm12,%ymm9
  .byte  0xc4,0x62,0x7d,0x18,0x52,0x58                   # vbroadcastss  0x58(%rdx),%ymm10
  .byte  0xc4,0xc1,0x7c,0xc2,0xc2,0x01                   # vcmpltps      %ymm10,%ymm0,%ymm0
  .byte  0xc4,0xc3,0x35,0x4a,0xc3,0x00                   # vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  .byte  0xc5,0x7c,0x52,0xc9                             # vrsqrtps      %ymm1,%ymm9
  .byte  0xc4,0x41,0x7c,0x53,0xd9                        # vrcpps        %ymm9,%ymm11
  .byte  0xc4,0x41,0x7c,0x52,0xc9                        # vrsqrtps      %ymm9,%ymm9
  .byte  0xc4,0x42,0x0d,0xa8,0xdf                        # vfmadd213ps   %ymm15,%ymm14,%ymm11
  .byte  0xc4,0x42,0x15,0xb8,0xd9                        # vfmadd231ps   %ymm9,%ymm13,%ymm11
  .byte  0xc5,0x3c,0x59,0xc9                             # vmulps        %ymm1,%ymm8,%ymm9
  .byte  0xc4,0x41,0x1c,0x5d,0xdb                        # vminps        %ymm11,%ymm12,%ymm11
  .byte  0xc4,0xc1,0x74,0xc2,0xca,0x01                   # vcmpltps      %ymm10,%ymm1,%ymm1
  .byte  0xc4,0xc3,0x25,0x4a,0xc9,0x10                   # vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
  .byte  0xc5,0x7c,0x52,0xca                             # vrsqrtps      %ymm2,%ymm9
  .byte  0xc4,0x41,0x7c,0x53,0xd9                        # vrcpps        %ymm9,%ymm11
  .byte  0xc4,0x42,0x0d,0xa8,0xdf                        # vfmadd213ps   %ymm15,%ymm14,%ymm11
  .byte  0xc4,0x41,0x7c,0x52,0xc9                        # vrsqrtps      %ymm9,%ymm9
  .byte  0xc4,0x42,0x15,0xb8,0xd9                        # vfmadd231ps   %ymm9,%ymm13,%ymm11
  .byte  0xc4,0x41,0x1c,0x5d,0xcb                        # vminps        %ymm11,%ymm12,%ymm9
  .byte  0xc5,0x3c,0x59,0xc2                             # vmulps        %ymm2,%ymm8,%ymm8
  .byte  0xc4,0xc1,0x6c,0xc2,0xd2,0x01                   # vcmpltps      %ymm10,%ymm2,%ymm2
  .byte  0xc4,0xc3,0x35,0x4a,0xd0,0x20                   # vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_scale_u8_hsw
_sk_scale_u8_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xc4,0x62,0x7d,0x31,0x04,0x38                   # vpmovzxbd     (%rax,%rdi,1),%ymm8
  .byte  0xc4,0x41,0x7c,0x5b,0xc0                        # vcvtdq2ps     %ymm8,%ymm8
  .byte  0xc4,0x62,0x7d,0x18,0x4a,0x0c                   # vbroadcastss  0xc(%rdx),%ymm9
  .byte  0xc4,0x41,0x3c,0x59,0xc1                        # vmulps        %ymm9,%ymm8,%ymm8
  .byte  0xc5,0xbc,0x59,0xc0                             # vmulps        %ymm0,%ymm8,%ymm0
  .byte  0xc5,0xbc,0x59,0xc9                             # vmulps        %ymm1,%ymm8,%ymm1
  .byte  0xc5,0xbc,0x59,0xd2                             # vmulps        %ymm2,%ymm8,%ymm2
  .byte  0xc5,0xbc,0x59,0xdb                             # vmulps        %ymm3,%ymm8,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_load_tables_hsw
_sk_load_tables_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x08                                  # mov           (%rax),%rcx
  .byte  0x4c,0x8b,0x40,0x08                             # mov           0x8(%rax),%r8
  .byte  0xc5,0xfc,0x10,0x1c,0xb9                        # vmovups       (%rcx,%rdi,4),%ymm3
  .byte  0xc4,0xe2,0x7d,0x18,0x52,0x10                   # vbroadcastss  0x10(%rdx),%ymm2
  .byte  0xc5,0xec,0x54,0xcb                             # vandps        %ymm3,%ymm2,%ymm1
  .byte  0xc5,0xfc,0x57,0xc0                             # vxorps        %ymm0,%ymm0,%ymm0
  .byte  0xc5,0x7c,0xc2,0xc0,0x00                        # vcmpeqps      %ymm0,%ymm0,%ymm8
  .byte  0xc4,0x41,0x7c,0x28,0xc8                        # vmovaps       %ymm8,%ymm9
  .byte  0xc4,0xc2,0x35,0x92,0x04,0x88                   # vgatherdps    %ymm9,(%r8,%ymm1,4),%ymm0
  .byte  0x48,0x8b,0x48,0x10                             # mov           0x10(%rax),%rcx
  .byte  0xc5,0xf5,0x72,0xd3,0x08                        # vpsrld        $0x8,%ymm3,%ymm1
  .byte  0xc5,0x6c,0x54,0xc9                             # vandps        %ymm1,%ymm2,%ymm9
  .byte  0xc4,0x41,0x7c,0x28,0xd0                        # vmovaps       %ymm8,%ymm10
  .byte  0xc4,0xa2,0x2d,0x92,0x0c,0x89                   # vgatherdps    %ymm10,(%rcx,%ymm9,4),%ymm1
  .byte  0x48,0x8b,0x40,0x18                             # mov           0x18(%rax),%rax
  .byte  0xc5,0xb5,0x72,0xd3,0x10                        # vpsrld        $0x10,%ymm3,%ymm9
  .byte  0xc4,0x41,0x6c,0x54,0xc9                        # vandps        %ymm9,%ymm2,%ymm9
  .byte  0xc4,0xa2,0x3d,0x92,0x14,0x88                   # vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
  .byte  0xc5,0xe5,0x72,0xd3,0x18                        # vpsrld        $0x18,%ymm3,%ymm3
  .byte  0xc5,0xfc,0x5b,0xdb                             # vcvtdq2ps     %ymm3,%ymm3
  .byte  0xc4,0x62,0x7d,0x18,0x42,0x0c                   # vbroadcastss  0xc(%rdx),%ymm8
  .byte  0xc4,0xc1,0x64,0x59,0xd8                        # vmulps        %ymm8,%ymm3,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_load_8888_hsw
_sk_load_8888_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xc5,0xfc,0x10,0x1c,0xb8                        # vmovups       (%rax,%rdi,4),%ymm3
  .byte  0xc4,0xe2,0x7d,0x18,0x52,0x10                   # vbroadcastss  0x10(%rdx),%ymm2
  .byte  0xc5,0xec,0x54,0xc3                             # vandps        %ymm3,%ymm2,%ymm0
  .byte  0xc5,0xfc,0x5b,0xc0                             # vcvtdq2ps     %ymm0,%ymm0
  .byte  0xc4,0x62,0x7d,0x18,0x42,0x0c                   # vbroadcastss  0xc(%rdx),%ymm8
  .byte  0xc5,0xbc,0x59,0xc0                             # vmulps        %ymm0,%ymm8,%ymm0
  .byte  0xc5,0xf5,0x72,0xd3,0x08                        # vpsrld        $0x8,%ymm3,%ymm1
  .byte  0xc5,0xec,0x54,0xc9                             # vandps        %ymm1,%ymm2,%ymm1
  .byte  0xc5,0xfc,0x5b,0xc9                             # vcvtdq2ps     %ymm1,%ymm1
  .byte  0xc5,0xbc,0x59,0xc9                             # vmulps        %ymm1,%ymm8,%ymm1
  .byte  0xc5,0xb5,0x72,0xd3,0x10                        # vpsrld        $0x10,%ymm3,%ymm9
  .byte  0xc4,0xc1,0x6c,0x54,0xd1                        # vandps        %ymm9,%ymm2,%ymm2
  .byte  0xc5,0xfc,0x5b,0xd2                             # vcvtdq2ps     %ymm2,%ymm2
  .byte  0xc5,0xbc,0x59,0xd2                             # vmulps        %ymm2,%ymm8,%ymm2
  .byte  0xc5,0xe5,0x72,0xd3,0x18                        # vpsrld        $0x18,%ymm3,%ymm3
  .byte  0xc5,0xfc,0x5b,0xdb                             # vcvtdq2ps     %ymm3,%ymm3
  .byte  0xc4,0xc1,0x64,0x59,0xd8                        # vmulps        %ymm8,%ymm3,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_store_8888_hsw
_sk_store_8888_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xc4,0x62,0x7d,0x18,0x42,0x08                   # vbroadcastss  0x8(%rdx),%ymm8
  .byte  0xc5,0x3c,0x59,0xc8                             # vmulps        %ymm0,%ymm8,%ymm9
  .byte  0xc4,0x41,0x7d,0x5b,0xc9                        # vcvtps2dq     %ymm9,%ymm9
  .byte  0xc5,0x3c,0x59,0xd1                             # vmulps        %ymm1,%ymm8,%ymm10
  .byte  0xc4,0x41,0x7d,0x5b,0xd2                        # vcvtps2dq     %ymm10,%ymm10
  .byte  0xc4,0xc1,0x2d,0x72,0xf2,0x08                   # vpslld        $0x8,%ymm10,%ymm10
  .byte  0xc4,0x41,0x2d,0xeb,0xc9                        # vpor          %ymm9,%ymm10,%ymm9
  .byte  0xc5,0x3c,0x59,0xd2                             # vmulps        %ymm2,%ymm8,%ymm10
  .byte  0xc4,0x41,0x7d,0x5b,0xd2                        # vcvtps2dq     %ymm10,%ymm10
  .byte  0xc4,0xc1,0x2d,0x72,0xf2,0x10                   # vpslld        $0x10,%ymm10,%ymm10
  .byte  0xc5,0x3c,0x59,0xc3                             # vmulps        %ymm3,%ymm8,%ymm8
  .byte  0xc4,0x41,0x7d,0x5b,0xc0                        # vcvtps2dq     %ymm8,%ymm8
  .byte  0xc4,0xc1,0x3d,0x72,0xf0,0x18                   # vpslld        $0x18,%ymm8,%ymm8
  .byte  0xc4,0x41,0x2d,0xeb,0xc0                        # vpor          %ymm8,%ymm10,%ymm8
  .byte  0xc4,0x41,0x35,0xeb,0xc0                        # vpor          %ymm8,%ymm9,%ymm8
  .byte  0xc5,0x7e,0x7f,0x04,0xb8                        # vmovdqu       %ymm8,(%rax,%rdi,4)
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_load_f16_hsw
_sk_load_f16_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xc5,0xfa,0x6f,0x04,0xf8                        # vmovdqu       (%rax,%rdi,8),%xmm0
  .byte  0xc5,0xfa,0x6f,0x4c,0xf8,0x10                   # vmovdqu       0x10(%rax,%rdi,8),%xmm1
  .byte  0xc5,0xfa,0x6f,0x54,0xf8,0x20                   # vmovdqu       0x20(%rax,%rdi,8),%xmm2
  .byte  0xc5,0xfa,0x6f,0x5c,0xf8,0x30                   # vmovdqu       0x30(%rax,%rdi,8),%xmm3
  .byte  0xc5,0x79,0x61,0xc1                             # vpunpcklwd    %xmm1,%xmm0,%xmm8
  .byte  0xc5,0xf9,0x69,0xc1                             # vpunpckhwd    %xmm1,%xmm0,%xmm0
  .byte  0xc5,0xe9,0x61,0xcb                             # vpunpcklwd    %xmm3,%xmm2,%xmm1
  .byte  0xc5,0xe9,0x69,0xd3                             # vpunpckhwd    %xmm3,%xmm2,%xmm2
  .byte  0xc5,0x39,0x61,0xc8                             # vpunpcklwd    %xmm0,%xmm8,%xmm9
  .byte  0xc5,0x39,0x69,0xc0                             # vpunpckhwd    %xmm0,%xmm8,%xmm8
  .byte  0xc5,0xf1,0x61,0xda                             # vpunpcklwd    %xmm2,%xmm1,%xmm3
  .byte  0xc5,0x71,0x69,0xd2                             # vpunpckhwd    %xmm2,%xmm1,%xmm10
  .byte  0xc5,0xb1,0x6c,0xc3                             # vpunpcklqdq   %xmm3,%xmm9,%xmm0
  .byte  0xc4,0xe2,0x7d,0x13,0xc0                        # vcvtph2ps     %xmm0,%ymm0
  .byte  0xc5,0xb1,0x6d,0xcb                             # vpunpckhqdq   %xmm3,%xmm9,%xmm1
  .byte  0xc4,0xe2,0x7d,0x13,0xc9                        # vcvtph2ps     %xmm1,%ymm1
  .byte  0xc4,0xc1,0x39,0x6c,0xd2                        # vpunpcklqdq   %xmm10,%xmm8,%xmm2
  .byte  0xc4,0xe2,0x7d,0x13,0xd2                        # vcvtph2ps     %xmm2,%ymm2
  .byte  0xc4,0xc1,0x39,0x6d,0xda                        # vpunpckhqdq   %xmm10,%xmm8,%xmm3
  .byte  0xc4,0xe2,0x7d,0x13,0xdb                        # vcvtph2ps     %xmm3,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_store_f16_hsw
_sk_store_f16_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xc4,0xc3,0x7d,0x1d,0xc0,0x04                   # vcvtps2ph     $0x4,%ymm0,%xmm8
  .byte  0xc4,0xc3,0x7d,0x1d,0xc9,0x04                   # vcvtps2ph     $0x4,%ymm1,%xmm9
  .byte  0xc4,0xc3,0x7d,0x1d,0xd2,0x04                   # vcvtps2ph     $0x4,%ymm2,%xmm10
  .byte  0xc4,0xc3,0x7d,0x1d,0xdb,0x04                   # vcvtps2ph     $0x4,%ymm3,%xmm11
  .byte  0xc4,0x41,0x39,0x61,0xe1                        # vpunpcklwd    %xmm9,%xmm8,%xmm12
  .byte  0xc4,0x41,0x39,0x69,0xc1                        # vpunpckhwd    %xmm9,%xmm8,%xmm8
  .byte  0xc4,0x41,0x29,0x61,0xcb                        # vpunpcklwd    %xmm11,%xmm10,%xmm9
  .byte  0xc4,0x41,0x29,0x69,0xd3                        # vpunpckhwd    %xmm11,%xmm10,%xmm10
  .byte  0xc4,0x41,0x19,0x62,0xd9                        # vpunpckldq    %xmm9,%xmm12,%xmm11
  .byte  0xc5,0x7a,0x7f,0x1c,0xf8                        # vmovdqu       %xmm11,(%rax,%rdi,8)
  .byte  0xc4,0x41,0x19,0x6a,0xc9                        # vpunpckhdq    %xmm9,%xmm12,%xmm9
  .byte  0xc5,0x7a,0x7f,0x4c,0xf8,0x10                   # vmovdqu       %xmm9,0x10(%rax,%rdi,8)
  .byte  0xc4,0x41,0x39,0x62,0xca                        # vpunpckldq    %xmm10,%xmm8,%xmm9
  .byte  0xc5,0x7a,0x7f,0x4c,0xf8,0x20                   # vmovdqu       %xmm9,0x20(%rax,%rdi,8)
  .byte  0xc4,0x41,0x39,0x6a,0xc2                        # vpunpckhdq    %xmm10,%xmm8,%xmm8
  .byte  0xc5,0x7a,0x7f,0x44,0xf8,0x30                   # vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_x_hsw
_sk_clamp_x_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x62,0x7d,0x58,0x00                        # vpbroadcastd  (%rax),%ymm8
  .byte  0xc4,0x41,0x35,0x76,0xc9                        # vpcmpeqd      %ymm9,%ymm9,%ymm9
  .byte  0xc4,0x41,0x3d,0xfe,0xc1                        # vpaddd        %ymm9,%ymm8,%ymm8
  .byte  0xc4,0xc1,0x7c,0x5d,0xc0                        # vminps        %ymm8,%ymm0,%ymm0
  .byte  0xc4,0x41,0x3c,0x57,0xc0                        # vxorps        %ymm8,%ymm8,%ymm8
  .byte  0xc5,0xbc,0x5f,0xc0                             # vmaxps        %ymm0,%ymm8,%ymm0
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_y_hsw
_sk_clamp_y_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x62,0x7d,0x58,0x00                        # vpbroadcastd  (%rax),%ymm8
  .byte  0xc4,0x41,0x35,0x76,0xc9                        # vpcmpeqd      %ymm9,%ymm9,%ymm9
  .byte  0xc4,0x41,0x3d,0xfe,0xc1                        # vpaddd        %ymm9,%ymm8,%ymm8
  .byte  0xc4,0xc1,0x74,0x5d,0xc8                        # vminps        %ymm8,%ymm1,%ymm1
  .byte  0xc4,0x41,0x3c,0x57,0xc0                        # vxorps        %ymm8,%ymm8,%ymm8
  .byte  0xc5,0xbc,0x5f,0xc9                             # vmaxps        %ymm1,%ymm8,%ymm1
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_matrix_2x3_hsw
_sk_matrix_2x3_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x62,0x7d,0x18,0x08                        # vbroadcastss  (%rax),%ymm9
  .byte  0xc4,0x62,0x7d,0x18,0x50,0x08                   # vbroadcastss  0x8(%rax),%ymm10
  .byte  0xc4,0x62,0x7d,0x18,0x40,0x10                   # vbroadcastss  0x10(%rax),%ymm8
  .byte  0xc4,0x42,0x75,0xb8,0xc2                        # vfmadd231ps   %ymm10,%ymm1,%ymm8
  .byte  0xc4,0x42,0x7d,0xb8,0xc1                        # vfmadd231ps   %ymm9,%ymm0,%ymm8
  .byte  0xc4,0x62,0x7d,0x18,0x50,0x04                   # vbroadcastss  0x4(%rax),%ymm10
  .byte  0xc4,0x62,0x7d,0x18,0x58,0x0c                   # vbroadcastss  0xc(%rax),%ymm11
  .byte  0xc4,0x62,0x7d,0x18,0x48,0x14                   # vbroadcastss  0x14(%rax),%ymm9
  .byte  0xc4,0x42,0x75,0xb8,0xcb                        # vfmadd231ps   %ymm11,%ymm1,%ymm9
  .byte  0xc4,0x42,0x7d,0xb8,0xca                        # vfmadd231ps   %ymm10,%ymm0,%ymm9
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0x7c,0x29,0xc0                             # vmovaps       %ymm8,%ymm0
  .byte  0xc5,0x7c,0x29,0xc9                             # vmovaps       %ymm9,%ymm1
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_matrix_3x4_hsw
_sk_matrix_3x4_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0x62,0x7d,0x18,0x08                        # vbroadcastss  (%rax),%ymm9
  .byte  0xc4,0x62,0x7d,0x18,0x50,0x0c                   # vbroadcastss  0xc(%rax),%ymm10
  .byte  0xc4,0x62,0x7d,0x18,0x58,0x18                   # vbroadcastss  0x18(%rax),%ymm11
  .byte  0xc4,0x62,0x7d,0x18,0x40,0x24                   # vbroadcastss  0x24(%rax),%ymm8
  .byte  0xc4,0x42,0x6d,0xb8,0xc3                        # vfmadd231ps   %ymm11,%ymm2,%ymm8
  .byte  0xc4,0x42,0x75,0xb8,0xc2                        # vfmadd231ps   %ymm10,%ymm1,%ymm8
  .byte  0xc4,0x42,0x7d,0xb8,0xc1                        # vfmadd231ps   %ymm9,%ymm0,%ymm8
  .byte  0xc4,0x62,0x7d,0x18,0x50,0x04                   # vbroadcastss  0x4(%rax),%ymm10
  .byte  0xc4,0x62,0x7d,0x18,0x58,0x10                   # vbroadcastss  0x10(%rax),%ymm11
  .byte  0xc4,0x62,0x7d,0x18,0x60,0x1c                   # vbroadcastss  0x1c(%rax),%ymm12
  .byte  0xc4,0x62,0x7d,0x18,0x48,0x28                   # vbroadcastss  0x28(%rax),%ymm9
  .byte  0xc4,0x42,0x6d,0xb8,0xcc                        # vfmadd231ps   %ymm12,%ymm2,%ymm9
  .byte  0xc4,0x42,0x75,0xb8,0xcb                        # vfmadd231ps   %ymm11,%ymm1,%ymm9
  .byte  0xc4,0x42,0x7d,0xb8,0xca                        # vfmadd231ps   %ymm10,%ymm0,%ymm9
  .byte  0xc4,0x62,0x7d,0x18,0x58,0x08                   # vbroadcastss  0x8(%rax),%ymm11
  .byte  0xc4,0x62,0x7d,0x18,0x60,0x14                   # vbroadcastss  0x14(%rax),%ymm12
  .byte  0xc4,0x62,0x7d,0x18,0x68,0x20                   # vbroadcastss  0x20(%rax),%ymm13
  .byte  0xc4,0x62,0x7d,0x18,0x50,0x2c                   # vbroadcastss  0x2c(%rax),%ymm10
  .byte  0xc4,0x42,0x6d,0xb8,0xd5                        # vfmadd231ps   %ymm13,%ymm2,%ymm10
  .byte  0xc4,0x42,0x75,0xb8,0xd4                        # vfmadd231ps   %ymm12,%ymm1,%ymm10
  .byte  0xc4,0x42,0x7d,0xb8,0xd3                        # vfmadd231ps   %ymm11,%ymm0,%ymm10
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0x7c,0x29,0xc0                             # vmovaps       %ymm8,%ymm0
  .byte  0xc5,0x7c,0x29,0xc9                             # vmovaps       %ymm9,%ymm1
  .byte  0xc5,0x7c,0x29,0xd2                             # vmovaps       %ymm10,%ymm2
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_linear_gradient_2stops_hsw
_sk_linear_gradient_2stops_hsw:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc4,0xe2,0x7d,0x18,0x48,0x10                   # vbroadcastss  0x10(%rax),%ymm1
  .byte  0xc4,0x62,0x7d,0x18,0x00                        # vbroadcastss  (%rax),%ymm8
  .byte  0xc4,0x62,0x7d,0xb8,0xc1                        # vfmadd231ps   %ymm1,%ymm0,%ymm8
  .byte  0xc4,0xe2,0x7d,0x18,0x50,0x14                   # vbroadcastss  0x14(%rax),%ymm2
  .byte  0xc4,0xe2,0x7d,0x18,0x48,0x04                   # vbroadcastss  0x4(%rax),%ymm1
  .byte  0xc4,0xe2,0x7d,0xb8,0xca                        # vfmadd231ps   %ymm2,%ymm0,%ymm1
  .byte  0xc4,0xe2,0x7d,0x18,0x58,0x18                   # vbroadcastss  0x18(%rax),%ymm3
  .byte  0xc4,0xe2,0x7d,0x18,0x50,0x08                   # vbroadcastss  0x8(%rax),%ymm2
  .byte  0xc4,0xe2,0x7d,0xb8,0xd3                        # vfmadd231ps   %ymm3,%ymm0,%ymm2
  .byte  0xc4,0x62,0x7d,0x18,0x48,0x1c                   # vbroadcastss  0x1c(%rax),%ymm9
  .byte  0xc4,0xe2,0x7d,0x18,0x58,0x0c                   # vbroadcastss  0xc(%rax),%ymm3
  .byte  0xc4,0xc2,0x7d,0xb8,0xd9                        # vfmadd231ps   %ymm9,%ymm0,%ymm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xc5,0x7c,0x29,0xc0                             # vmovaps       %ymm8,%ymm0
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_start_pipeline_sse41
_sk_start_pipeline_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x57,0xc0                                  # xorps         %xmm0,%xmm0
  .byte  0x0f,0x57,0xc9                                  # xorps         %xmm1,%xmm1
  .byte  0x0f,0x57,0xd2                                  # xorps         %xmm2,%xmm2
  .byte  0x0f,0x57,0xdb                                  # xorps         %xmm3,%xmm3
  .byte  0x0f,0x57,0xe4                                  # xorps         %xmm4,%xmm4
  .byte  0x0f,0x57,0xed                                  # xorps         %xmm5,%xmm5
  .byte  0x0f,0x57,0xf6                                  # xorps         %xmm6,%xmm6
  .byte  0x0f,0x57,0xff                                  # xorps         %xmm7,%xmm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_start_pipeline_ms_sse41
_sk_start_pipeline_ms_sse41:
  .byte  0x56                                            # push          %rsi
  .byte  0x57                                            # push          %rdi
  .byte  0x48,0x81,0xec,0xa8,0x00,0x00,0x00              # sub           $0xa8,%rsp
  .byte  0x44,0x0f,0x29,0xbc,0x24,0x90,0x00,0x00,0x00    # movaps        %xmm15,0x90(%rsp)
  .byte  0x44,0x0f,0x29,0xb4,0x24,0x80,0x00,0x00,0x00    # movaps        %xmm14,0x80(%rsp)
  .byte  0x44,0x0f,0x29,0x6c,0x24,0x70                   # movaps        %xmm13,0x70(%rsp)
  .byte  0x44,0x0f,0x29,0x64,0x24,0x60                   # movaps        %xmm12,0x60(%rsp)
  .byte  0x44,0x0f,0x29,0x5c,0x24,0x50                   # movaps        %xmm11,0x50(%rsp)
  .byte  0x44,0x0f,0x29,0x54,0x24,0x40                   # movaps        %xmm10,0x40(%rsp)
  .byte  0x44,0x0f,0x29,0x4c,0x24,0x30                   # movaps        %xmm9,0x30(%rsp)
  .byte  0x44,0x0f,0x29,0x44,0x24,0x20                   # movaps        %xmm8,0x20(%rsp)
  .byte  0x0f,0x29,0x7c,0x24,0x10                        # movaps        %xmm7,0x10(%rsp)
  .byte  0x0f,0x29,0x34,0x24                             # movaps        %xmm6,(%rsp)
  .byte  0x48,0x89,0xd6                                  # mov           %rdx,%rsi
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x57,0xc0                                  # xorps         %xmm0,%xmm0
  .byte  0x0f,0x57,0xc9                                  # xorps         %xmm1,%xmm1
  .byte  0x0f,0x57,0xd2                                  # xorps         %xmm2,%xmm2
  .byte  0x0f,0x57,0xdb                                  # xorps         %xmm3,%xmm3
  .byte  0x0f,0x57,0xe4                                  # xorps         %xmm4,%xmm4
  .byte  0x0f,0x57,0xed                                  # xorps         %xmm5,%xmm5
  .byte  0x0f,0x57,0xf6                                  # xorps         %xmm6,%xmm6
  .byte  0x0f,0x57,0xff                                  # xorps         %xmm7,%xmm7
  .byte  0x48,0x89,0xcf                                  # mov           %rcx,%rdi
  .byte  0x4c,0x89,0xc2                                  # mov           %r8,%rdx
  .byte  0xff,0xd0                                       # callq         *%rax
  .byte  0x0f,0x28,0x34,0x24                             # movaps        (%rsp),%xmm6
  .byte  0x0f,0x28,0x7c,0x24,0x10                        # movaps        0x10(%rsp),%xmm7
  .byte  0x44,0x0f,0x28,0x44,0x24,0x20                   # movaps        0x20(%rsp),%xmm8
  .byte  0x44,0x0f,0x28,0x4c,0x24,0x30                   # movaps        0x30(%rsp),%xmm9
  .byte  0x44,0x0f,0x28,0x54,0x24,0x40                   # movaps        0x40(%rsp),%xmm10
  .byte  0x44,0x0f,0x28,0x5c,0x24,0x50                   # movaps        0x50(%rsp),%xmm11
  .byte  0x44,0x0f,0x28,0x64,0x24,0x60                   # movaps        0x60(%rsp),%xmm12
  .byte  0x44,0x0f,0x28,0x6c,0x24,0x70                   # movaps        0x70(%rsp),%xmm13
  .byte  0x44,0x0f,0x28,0xb4,0x24,0x80,0x00,0x00,0x00    # movaps        0x80(%rsp),%xmm14
  .byte  0x44,0x0f,0x28,0xbc,0x24,0x90,0x00,0x00,0x00    # movaps        0x90(%rsp),%xmm15
  .byte  0x48,0x81,0xc4,0xa8,0x00,0x00,0x00              # add           $0xa8,%rsp
  .byte  0x5f                                            # pop           %rdi
  .byte  0x5e                                            # pop           %rsi
  .byte  0xc3                                            # retq

.globl _sk_just_return_sse41
_sk_just_return_sse41:
  .byte  0xc3                                            # retq

.globl _sk_seed_shader_sse41
_sk_seed_shader_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x66,0x0f,0x6e,0xc7                             # movd          %edi,%xmm0
  .byte  0x66,0x0f,0x70,0xc0,0x00                        # pshufd        $0x0,%xmm0,%xmm0
  .byte  0x0f,0x5b,0xc8                                  # cvtdq2ps      %xmm0,%xmm1
  .byte  0xf3,0x0f,0x10,0x12                             # movss         (%rdx),%xmm2
  .byte  0xf3,0x0f,0x10,0x5a,0x04                        # movss         0x4(%rdx),%xmm3
  .byte  0x0f,0xc6,0xdb,0x00                             # shufps        $0x0,%xmm3,%xmm3
  .byte  0x0f,0x58,0xcb                                  # addps         %xmm3,%xmm1
  .byte  0x0f,0x10,0x42,0x14                             # movups        0x14(%rdx),%xmm0
  .byte  0x0f,0x58,0xc1                                  # addps         %xmm1,%xmm0
  .byte  0x66,0x0f,0x6e,0x08                             # movd          (%rax),%xmm1
  .byte  0x66,0x0f,0x70,0xc9,0x00                        # pshufd        $0x0,%xmm1,%xmm1
  .byte  0x0f,0x5b,0xc9                                  # cvtdq2ps      %xmm1,%xmm1
  .byte  0x0f,0x58,0xcb                                  # addps         %xmm3,%xmm1
  .byte  0x0f,0xc6,0xd2,0x00                             # shufps        $0x0,%xmm2,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x57,0xdb                                  # xorps         %xmm3,%xmm3
  .byte  0x0f,0x57,0xe4                                  # xorps         %xmm4,%xmm4
  .byte  0x0f,0x57,0xed                                  # xorps         %xmm5,%xmm5
  .byte  0x0f,0x57,0xf6                                  # xorps         %xmm6,%xmm6
  .byte  0x0f,0x57,0xff                                  # xorps         %xmm7,%xmm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_constant_color_sse41
_sk_constant_color_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x10,0x18                                  # movups        (%rax),%xmm3
  .byte  0x0f,0x28,0xc3                                  # movaps        %xmm3,%xmm0
  .byte  0x0f,0xc6,0xc0,0x00                             # shufps        $0x0,%xmm0,%xmm0
  .byte  0x0f,0x28,0xcb                                  # movaps        %xmm3,%xmm1
  .byte  0x0f,0xc6,0xc9,0x55                             # shufps        $0x55,%xmm1,%xmm1
  .byte  0x0f,0x28,0xd3                                  # movaps        %xmm3,%xmm2
  .byte  0x0f,0xc6,0xd2,0xaa                             # shufps        $0xaa,%xmm2,%xmm2
  .byte  0x0f,0xc6,0xdb,0xff                             # shufps        $0xff,%xmm3,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clear_sse41
_sk_clear_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x57,0xc0                                  # xorps         %xmm0,%xmm0
  .byte  0x0f,0x57,0xc9                                  # xorps         %xmm1,%xmm1
  .byte  0x0f,0x57,0xd2                                  # xorps         %xmm2,%xmm2
  .byte  0x0f,0x57,0xdb                                  # xorps         %xmm3,%xmm3
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_plus__sse41
_sk_plus__sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x58,0xc4                                  # addps         %xmm4,%xmm0
  .byte  0x0f,0x58,0xcd                                  # addps         %xmm5,%xmm1
  .byte  0x0f,0x58,0xd6                                  # addps         %xmm6,%xmm2
  .byte  0x0f,0x58,0xdf                                  # addps         %xmm7,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_srcover_sse41
_sk_srcover_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x02                        # movss         (%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x44,0x0f,0x5c,0xc3                             # subps         %xmm3,%xmm8
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xcc                             # mulps         %xmm4,%xmm9
  .byte  0x41,0x0f,0x58,0xc1                             # addps         %xmm9,%xmm0
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xcd                             # mulps         %xmm5,%xmm9
  .byte  0x41,0x0f,0x58,0xc9                             # addps         %xmm9,%xmm1
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xce                             # mulps         %xmm6,%xmm9
  .byte  0x41,0x0f,0x58,0xd1                             # addps         %xmm9,%xmm2
  .byte  0x44,0x0f,0x59,0xc7                             # mulps         %xmm7,%xmm8
  .byte  0x41,0x0f,0x58,0xd8                             # addps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_dstover_sse41
_sk_dstover_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x02                        # movss         (%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x44,0x0f,0x5c,0xc7                             # subps         %xmm7,%xmm8
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x0f,0x58,0xc4                                  # addps         %xmm4,%xmm0
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x0f,0x58,0xcd                                  # addps         %xmm5,%xmm1
  .byte  0x41,0x0f,0x59,0xd0                             # mulps         %xmm8,%xmm2
  .byte  0x0f,0x58,0xd6                                  # addps         %xmm6,%xmm2
  .byte  0x41,0x0f,0x59,0xd8                             # mulps         %xmm8,%xmm3
  .byte  0x0f,0x58,0xdf                                  # addps         %xmm7,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_0_sse41
_sk_clamp_0_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x45,0x0f,0x57,0xc0                             # xorps         %xmm8,%xmm8
  .byte  0x41,0x0f,0x5f,0xc0                             # maxps         %xmm8,%xmm0
  .byte  0x41,0x0f,0x5f,0xc8                             # maxps         %xmm8,%xmm1
  .byte  0x41,0x0f,0x5f,0xd0                             # maxps         %xmm8,%xmm2
  .byte  0x41,0x0f,0x5f,0xd8                             # maxps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_1_sse41
_sk_clamp_1_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x02                        # movss         (%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x41,0x0f,0x5d,0xc0                             # minps         %xmm8,%xmm0
  .byte  0x41,0x0f,0x5d,0xc8                             # minps         %xmm8,%xmm1
  .byte  0x41,0x0f,0x5d,0xd0                             # minps         %xmm8,%xmm2
  .byte  0x41,0x0f,0x5d,0xd8                             # minps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_a_sse41
_sk_clamp_a_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x02                        # movss         (%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x41,0x0f,0x5d,0xd8                             # minps         %xmm8,%xmm3
  .byte  0x0f,0x5d,0xc3                                  # minps         %xmm3,%xmm0
  .byte  0x0f,0x5d,0xcb                                  # minps         %xmm3,%xmm1
  .byte  0x0f,0x5d,0xd3                                  # minps         %xmm3,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_swap_sse41
_sk_swap_sse41:
  .byte  0x44,0x0f,0x28,0xc3                             # movaps        %xmm3,%xmm8
  .byte  0x44,0x0f,0x28,0xca                             # movaps        %xmm2,%xmm9
  .byte  0x44,0x0f,0x28,0xd1                             # movaps        %xmm1,%xmm10
  .byte  0x44,0x0f,0x28,0xd8                             # movaps        %xmm0,%xmm11
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x28,0xc4                                  # movaps        %xmm4,%xmm0
  .byte  0x0f,0x28,0xcd                                  # movaps        %xmm5,%xmm1
  .byte  0x0f,0x28,0xd6                                  # movaps        %xmm6,%xmm2
  .byte  0x0f,0x28,0xdf                                  # movaps        %xmm7,%xmm3
  .byte  0x41,0x0f,0x28,0xe3                             # movaps        %xmm11,%xmm4
  .byte  0x41,0x0f,0x28,0xea                             # movaps        %xmm10,%xmm5
  .byte  0x41,0x0f,0x28,0xf1                             # movaps        %xmm9,%xmm6
  .byte  0x41,0x0f,0x28,0xf8                             # movaps        %xmm8,%xmm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_move_src_dst_sse41
_sk_move_src_dst_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x28,0xe0                                  # movaps        %xmm0,%xmm4
  .byte  0x0f,0x28,0xe9                                  # movaps        %xmm1,%xmm5
  .byte  0x0f,0x28,0xf2                                  # movaps        %xmm2,%xmm6
  .byte  0x0f,0x28,0xfb                                  # movaps        %xmm3,%xmm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_move_dst_src_sse41
_sk_move_dst_src_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x28,0xc4                                  # movaps        %xmm4,%xmm0
  .byte  0x0f,0x28,0xcd                                  # movaps        %xmm5,%xmm1
  .byte  0x0f,0x28,0xd6                                  # movaps        %xmm6,%xmm2
  .byte  0x0f,0x28,0xdf                                  # movaps        %xmm7,%xmm3
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_premul_sse41
_sk_premul_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x59,0xc3                                  # mulps         %xmm3,%xmm0
  .byte  0x0f,0x59,0xcb                                  # mulps         %xmm3,%xmm1
  .byte  0x0f,0x59,0xd3                                  # mulps         %xmm3,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_unpremul_sse41
_sk_unpremul_sse41:
  .byte  0x44,0x0f,0x28,0xc0                             # movaps        %xmm0,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x45,0x0f,0x57,0xc9                             # xorps         %xmm9,%xmm9
  .byte  0xf3,0x44,0x0f,0x10,0x12                        # movss         (%rdx),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0x44,0x0f,0x5e,0xd3                             # divps         %xmm3,%xmm10
  .byte  0x0f,0x28,0xc3                                  # movaps        %xmm3,%xmm0
  .byte  0x41,0x0f,0xc2,0xc1,0x00                        # cmpeqps       %xmm9,%xmm0
  .byte  0x66,0x45,0x0f,0x38,0x14,0xd1                   # blendvps      %xmm0,%xmm9,%xmm10
  .byte  0x45,0x0f,0x59,0xc2                             # mulps         %xmm10,%xmm8
  .byte  0x41,0x0f,0x59,0xca                             # mulps         %xmm10,%xmm1
  .byte  0x41,0x0f,0x59,0xd2                             # mulps         %xmm10,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xc0                             # movaps        %xmm8,%xmm0
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_from_srgb_sse41
_sk_from_srgb_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x5a,0x40                   # movss         0x40(%rdx),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0x45,0x0f,0x28,0xd3                             # movaps        %xmm11,%xmm10
  .byte  0x44,0x0f,0x59,0xd0                             # mulps         %xmm0,%xmm10
  .byte  0x44,0x0f,0x28,0xf0                             # movaps        %xmm0,%xmm14
  .byte  0x45,0x0f,0x59,0xf6                             # mulps         %xmm14,%xmm14
  .byte  0xf3,0x44,0x0f,0x10,0x42,0x3c                   # movss         0x3c(%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0xf3,0x44,0x0f,0x10,0x62,0x34                   # movss         0x34(%rdx),%xmm12
  .byte  0xf3,0x44,0x0f,0x10,0x6a,0x38                   # movss         0x38(%rdx),%xmm13
  .byte  0x45,0x0f,0xc6,0xed,0x00                        # shufps        $0x0,%xmm13,%xmm13
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xc8                             # mulps         %xmm0,%xmm9
  .byte  0x45,0x0f,0x58,0xcd                             # addps         %xmm13,%xmm9
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0x45,0x0f,0x59,0xce                             # mulps         %xmm14,%xmm9
  .byte  0x45,0x0f,0x58,0xcc                             # addps         %xmm12,%xmm9
  .byte  0xf3,0x44,0x0f,0x10,0x72,0x44                   # movss         0x44(%rdx),%xmm14
  .byte  0x45,0x0f,0xc6,0xf6,0x00                        # shufps        $0x0,%xmm14,%xmm14
  .byte  0x41,0x0f,0xc2,0xc6,0x01                        # cmpltps       %xmm14,%xmm0
  .byte  0x66,0x45,0x0f,0x38,0x14,0xca                   # blendvps      %xmm0,%xmm10,%xmm9
  .byte  0x45,0x0f,0x28,0xfb                             # movaps        %xmm11,%xmm15
  .byte  0x44,0x0f,0x59,0xf9                             # mulps         %xmm1,%xmm15
  .byte  0x0f,0x28,0xc1                                  # movaps        %xmm1,%xmm0
  .byte  0x0f,0x59,0xc0                                  # mulps         %xmm0,%xmm0
  .byte  0x45,0x0f,0x28,0xd0                             # movaps        %xmm8,%xmm10
  .byte  0x44,0x0f,0x59,0xd1                             # mulps         %xmm1,%xmm10
  .byte  0x45,0x0f,0x58,0xd5                             # addps         %xmm13,%xmm10
  .byte  0x44,0x0f,0x59,0xd0                             # mulps         %xmm0,%xmm10
  .byte  0x45,0x0f,0x58,0xd4                             # addps         %xmm12,%xmm10
  .byte  0x41,0x0f,0xc2,0xce,0x01                        # cmpltps       %xmm14,%xmm1
  .byte  0x0f,0x28,0xc1                                  # movaps        %xmm1,%xmm0
  .byte  0x66,0x45,0x0f,0x38,0x14,0xd7                   # blendvps      %xmm0,%xmm15,%xmm10
  .byte  0x44,0x0f,0x59,0xda                             # mulps         %xmm2,%xmm11
  .byte  0x0f,0x28,0xc2                                  # movaps        %xmm2,%xmm0
  .byte  0x0f,0x59,0xc0                                  # mulps         %xmm0,%xmm0
  .byte  0x44,0x0f,0x59,0xc2                             # mulps         %xmm2,%xmm8
  .byte  0x45,0x0f,0x58,0xc5                             # addps         %xmm13,%xmm8
  .byte  0x44,0x0f,0x59,0xc0                             # mulps         %xmm0,%xmm8
  .byte  0x45,0x0f,0x58,0xc4                             # addps         %xmm12,%xmm8
  .byte  0x41,0x0f,0xc2,0xd6,0x01                        # cmpltps       %xmm14,%xmm2
  .byte  0x0f,0x28,0xc2                                  # movaps        %xmm2,%xmm0
  .byte  0x66,0x45,0x0f,0x38,0x14,0xc3                   # blendvps      %xmm0,%xmm11,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xc1                             # movaps        %xmm9,%xmm0
  .byte  0x41,0x0f,0x28,0xca                             # movaps        %xmm10,%xmm1
  .byte  0x41,0x0f,0x28,0xd0                             # movaps        %xmm8,%xmm2
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_to_srgb_sse41
_sk_to_srgb_sse41:
  .byte  0x48,0x83,0xec,0x18                             # sub           $0x18,%rsp
  .byte  0x0f,0x29,0x3c,0x24                             # movaps        %xmm7,(%rsp)
  .byte  0x0f,0x28,0xfe                                  # movaps        %xmm6,%xmm7
  .byte  0x0f,0x28,0xf5                                  # movaps        %xmm5,%xmm6
  .byte  0x0f,0x28,0xec                                  # movaps        %xmm4,%xmm5
  .byte  0x0f,0x28,0xe3                                  # movaps        %xmm3,%xmm4
  .byte  0x44,0x0f,0x28,0xc2                             # movaps        %xmm2,%xmm8
  .byte  0x0f,0x28,0xd9                                  # movaps        %xmm1,%xmm3
  .byte  0x0f,0x52,0xd0                                  # rsqrtps       %xmm0,%xmm2
  .byte  0x44,0x0f,0x53,0xca                             # rcpps         %xmm2,%xmm9
  .byte  0x44,0x0f,0x52,0xd2                             # rsqrtps       %xmm2,%xmm10
  .byte  0xf3,0x0f,0x10,0x12                             # movss         (%rdx),%xmm2
  .byte  0xf3,0x44,0x0f,0x10,0x5a,0x48                   # movss         0x48(%rdx),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0x41,0x0f,0x28,0xcb                             # movaps        %xmm11,%xmm1
  .byte  0x0f,0x59,0xc8                                  # mulps         %xmm0,%xmm1
  .byte  0x0f,0xc6,0xd2,0x00                             # shufps        $0x0,%xmm2,%xmm2
  .byte  0xf3,0x44,0x0f,0x10,0x62,0x4c                   # movss         0x4c(%rdx),%xmm12
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0xf3,0x44,0x0f,0x10,0x6a,0x50                   # movss         0x50(%rdx),%xmm13
  .byte  0x45,0x0f,0xc6,0xed,0x00                        # shufps        $0x0,%xmm13,%xmm13
  .byte  0xf3,0x44,0x0f,0x10,0x72,0x54                   # movss         0x54(%rdx),%xmm14
  .byte  0x45,0x0f,0xc6,0xf6,0x00                        # shufps        $0x0,%xmm14,%xmm14
  .byte  0x45,0x0f,0x59,0xcd                             # mulps         %xmm13,%xmm9
  .byte  0x45,0x0f,0x58,0xce                             # addps         %xmm14,%xmm9
  .byte  0x45,0x0f,0x59,0xd4                             # mulps         %xmm12,%xmm10
  .byte  0x45,0x0f,0x58,0xd1                             # addps         %xmm9,%xmm10
  .byte  0x44,0x0f,0x28,0xca                             # movaps        %xmm2,%xmm9
  .byte  0x45,0x0f,0x5d,0xca                             # minps         %xmm10,%xmm9
  .byte  0xf3,0x44,0x0f,0x10,0x7a,0x58                   # movss         0x58(%rdx),%xmm15
  .byte  0x45,0x0f,0xc6,0xff,0x00                        # shufps        $0x0,%xmm15,%xmm15
  .byte  0x41,0x0f,0xc2,0xc7,0x01                        # cmpltps       %xmm15,%xmm0
  .byte  0x66,0x44,0x0f,0x38,0x14,0xc9                   # blendvps      %xmm0,%xmm1,%xmm9
  .byte  0x0f,0x52,0xc3                                  # rsqrtps       %xmm3,%xmm0
  .byte  0x0f,0x53,0xc8                                  # rcpps         %xmm0,%xmm1
  .byte  0x0f,0x52,0xc0                                  # rsqrtps       %xmm0,%xmm0
  .byte  0x41,0x0f,0x59,0xcd                             # mulps         %xmm13,%xmm1
  .byte  0x41,0x0f,0x58,0xce                             # addps         %xmm14,%xmm1
  .byte  0x41,0x0f,0x59,0xc4                             # mulps         %xmm12,%xmm0
  .byte  0x0f,0x58,0xc1                                  # addps         %xmm1,%xmm0
  .byte  0x44,0x0f,0x28,0xd2                             # movaps        %xmm2,%xmm10
  .byte  0x44,0x0f,0x5d,0xd0                             # minps         %xmm0,%xmm10
  .byte  0x41,0x0f,0x28,0xcb                             # movaps        %xmm11,%xmm1
  .byte  0x0f,0x59,0xcb                                  # mulps         %xmm3,%xmm1
  .byte  0x41,0x0f,0xc2,0xdf,0x01                        # cmpltps       %xmm15,%xmm3
  .byte  0x0f,0x28,0xc3                                  # movaps        %xmm3,%xmm0
  .byte  0x66,0x44,0x0f,0x38,0x14,0xd1                   # blendvps      %xmm0,%xmm1,%xmm10
  .byte  0x41,0x0f,0x52,0xc0                             # rsqrtps       %xmm8,%xmm0
  .byte  0x0f,0x53,0xc8                                  # rcpps         %xmm0,%xmm1
  .byte  0x41,0x0f,0x59,0xcd                             # mulps         %xmm13,%xmm1
  .byte  0x41,0x0f,0x58,0xce                             # addps         %xmm14,%xmm1
  .byte  0x0f,0x52,0xc0                                  # rsqrtps       %xmm0,%xmm0
  .byte  0x41,0x0f,0x59,0xc4                             # mulps         %xmm12,%xmm0
  .byte  0x0f,0x58,0xc1                                  # addps         %xmm1,%xmm0
  .byte  0x0f,0x5d,0xd0                                  # minps         %xmm0,%xmm2
  .byte  0x45,0x0f,0x59,0xd8                             # mulps         %xmm8,%xmm11
  .byte  0x45,0x0f,0xc2,0xc7,0x01                        # cmpltps       %xmm15,%xmm8
  .byte  0x41,0x0f,0x28,0xc0                             # movaps        %xmm8,%xmm0
  .byte  0x66,0x41,0x0f,0x38,0x14,0xd3                   # blendvps      %xmm0,%xmm11,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xc1                             # movaps        %xmm9,%xmm0
  .byte  0x41,0x0f,0x28,0xca                             # movaps        %xmm10,%xmm1
  .byte  0x0f,0x28,0xdc                                  # movaps        %xmm4,%xmm3
  .byte  0x0f,0x28,0xe5                                  # movaps        %xmm5,%xmm4
  .byte  0x0f,0x28,0xee                                  # movaps        %xmm6,%xmm5
  .byte  0x0f,0x28,0xf7                                  # movaps        %xmm7,%xmm6
  .byte  0x0f,0x28,0x3c,0x24                             # movaps        (%rsp),%xmm7
  .byte  0x48,0x83,0xc4,0x18                             # add           $0x18,%rsp
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_scale_u8_sse41
_sk_scale_u8_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0x66,0x44,0x0f,0x38,0x31,0x04,0x38              # pmovzxbd      (%rax,%rdi,1),%xmm8
  .byte  0x45,0x0f,0x5b,0xc0                             # cvtdq2ps      %xmm8,%xmm8
  .byte  0xf3,0x44,0x0f,0x10,0x4a,0x0c                   # movss         0xc(%rdx),%xmm9
  .byte  0x45,0x0f,0xc6,0xc9,0x00                        # shufps        $0x0,%xmm9,%xmm9
  .byte  0x45,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm9
  .byte  0x41,0x0f,0x59,0xc1                             # mulps         %xmm9,%xmm0
  .byte  0x41,0x0f,0x59,0xc9                             # mulps         %xmm9,%xmm1
  .byte  0x41,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm2
  .byte  0x41,0x0f,0x59,0xd9                             # mulps         %xmm9,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_load_tables_sse41
_sk_load_tables_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x08                                  # mov           (%rax),%rcx
  .byte  0x4c,0x8b,0x40,0x08                             # mov           0x8(%rax),%r8
  .byte  0xf3,0x44,0x0f,0x6f,0x04,0xb9                   # movdqu        (%rcx,%rdi,4),%xmm8
  .byte  0x66,0x0f,0x6e,0x42,0x10                        # movd          0x10(%rdx),%xmm0
  .byte  0x66,0x0f,0x70,0xc0,0x00                        # pshufd        $0x0,%xmm0,%xmm0
  .byte  0x66,0x41,0x0f,0x6f,0xc8                        # movdqa        %xmm8,%xmm1
  .byte  0x66,0x0f,0x72,0xd1,0x08                        # psrld         $0x8,%xmm1
  .byte  0x66,0x0f,0xdb,0xc8                             # pand          %xmm0,%xmm1
  .byte  0x66,0x41,0x0f,0x6f,0xd0                        # movdqa        %xmm8,%xmm2
  .byte  0x66,0x0f,0x72,0xd2,0x10                        # psrld         $0x10,%xmm2
  .byte  0x66,0x0f,0xdb,0xd0                             # pand          %xmm0,%xmm2
  .byte  0x66,0x41,0x0f,0xdb,0xc0                        # pand          %xmm8,%xmm0
  .byte  0x66,0x48,0x0f,0x3a,0x16,0xc1,0x01              # pextrq        $0x1,%xmm0,%rcx
  .byte  0x41,0x89,0xc9                                  # mov           %ecx,%r9d
  .byte  0x48,0xc1,0xe9,0x20                             # shr           $0x20,%rcx
  .byte  0x66,0x49,0x0f,0x7e,0xc2                        # movq          %xmm0,%r10
  .byte  0x45,0x89,0xd3                                  # mov           %r10d,%r11d
  .byte  0x49,0xc1,0xea,0x20                             # shr           $0x20,%r10
  .byte  0xf3,0x43,0x0f,0x10,0x04,0x98                   # movss         (%r8,%r11,4),%xmm0
  .byte  0x66,0x43,0x0f,0x3a,0x21,0x04,0x90,0x10         # insertps      $0x10,(%r8,%r10,4),%xmm0
  .byte  0x66,0x43,0x0f,0x3a,0x21,0x04,0x88,0x20         # insertps      $0x20,(%r8,%r9,4),%xmm0
  .byte  0x66,0x41,0x0f,0x3a,0x21,0x04,0x88,0x30         # insertps      $0x30,(%r8,%rcx,4),%xmm0
  .byte  0x48,0x8b,0x48,0x10                             # mov           0x10(%rax),%rcx
  .byte  0x66,0x49,0x0f,0x3a,0x16,0xc8,0x01              # pextrq        $0x1,%xmm1,%r8
  .byte  0x45,0x89,0xc1                                  # mov           %r8d,%r9d
  .byte  0x49,0xc1,0xe8,0x20                             # shr           $0x20,%r8
  .byte  0x66,0x49,0x0f,0x7e,0xca                        # movq          %xmm1,%r10
  .byte  0x45,0x89,0xd3                                  # mov           %r10d,%r11d
  .byte  0x49,0xc1,0xea,0x20                             # shr           $0x20,%r10
  .byte  0xf3,0x42,0x0f,0x10,0x0c,0x99                   # movss         (%rcx,%r11,4),%xmm1
  .byte  0x66,0x42,0x0f,0x3a,0x21,0x0c,0x91,0x10         # insertps      $0x10,(%rcx,%r10,4),%xmm1
  .byte  0xf3,0x42,0x0f,0x10,0x1c,0x89                   # movss         (%rcx,%r9,4),%xmm3
  .byte  0x66,0x0f,0x3a,0x21,0xcb,0x20                   # insertps      $0x20,%xmm3,%xmm1
  .byte  0xf3,0x42,0x0f,0x10,0x1c,0x81                   # movss         (%rcx,%r8,4),%xmm3
  .byte  0x66,0x0f,0x3a,0x21,0xcb,0x30                   # insertps      $0x30,%xmm3,%xmm1
  .byte  0x48,0x8b,0x40,0x18                             # mov           0x18(%rax),%rax
  .byte  0x66,0x48,0x0f,0x3a,0x16,0xd1,0x01              # pextrq        $0x1,%xmm2,%rcx
  .byte  0x41,0x89,0xc8                                  # mov           %ecx,%r8d
  .byte  0x48,0xc1,0xe9,0x20                             # shr           $0x20,%rcx
  .byte  0x66,0x49,0x0f,0x7e,0xd1                        # movq          %xmm2,%r9
  .byte  0x45,0x89,0xca                                  # mov           %r9d,%r10d
  .byte  0x49,0xc1,0xe9,0x20                             # shr           $0x20,%r9
  .byte  0xf3,0x42,0x0f,0x10,0x14,0x90                   # movss         (%rax,%r10,4),%xmm2
  .byte  0x66,0x42,0x0f,0x3a,0x21,0x14,0x88,0x10         # insertps      $0x10,(%rax,%r9,4),%xmm2
  .byte  0xf3,0x42,0x0f,0x10,0x1c,0x80                   # movss         (%rax,%r8,4),%xmm3
  .byte  0x66,0x0f,0x3a,0x21,0xd3,0x20                   # insertps      $0x20,%xmm3,%xmm2
  .byte  0xf3,0x0f,0x10,0x1c,0x88                        # movss         (%rax,%rcx,4),%xmm3
  .byte  0x66,0x0f,0x3a,0x21,0xd3,0x30                   # insertps      $0x30,%xmm3,%xmm2
  .byte  0x66,0x41,0x0f,0x72,0xd0,0x18                   # psrld         $0x18,%xmm8
  .byte  0x45,0x0f,0x5b,0xc0                             # cvtdq2ps      %xmm8,%xmm8
  .byte  0xf3,0x0f,0x10,0x5a,0x0c                        # movss         0xc(%rdx),%xmm3
  .byte  0x0f,0xc6,0xdb,0x00                             # shufps        $0x0,%xmm3,%xmm3
  .byte  0x41,0x0f,0x59,0xd8                             # mulps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_load_8888_sse41
_sk_load_8888_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xf3,0x0f,0x6f,0x1c,0xb8                        # movdqu        (%rax,%rdi,4),%xmm3
  .byte  0x66,0x0f,0x6e,0x42,0x10                        # movd          0x10(%rdx),%xmm0
  .byte  0x66,0x0f,0x70,0xc0,0x00                        # pshufd        $0x0,%xmm0,%xmm0
  .byte  0x66,0x0f,0x6f,0xcb                             # movdqa        %xmm3,%xmm1
  .byte  0x66,0x0f,0x72,0xd1,0x08                        # psrld         $0x8,%xmm1
  .byte  0x66,0x0f,0xdb,0xc8                             # pand          %xmm0,%xmm1
  .byte  0x66,0x0f,0x6f,0xd3                             # movdqa        %xmm3,%xmm2
  .byte  0x66,0x0f,0x72,0xd2,0x10                        # psrld         $0x10,%xmm2
  .byte  0x66,0x0f,0xdb,0xd0                             # pand          %xmm0,%xmm2
  .byte  0x66,0x0f,0xdb,0xc3                             # pand          %xmm3,%xmm0
  .byte  0x0f,0x5b,0xc0                                  # cvtdq2ps      %xmm0,%xmm0
  .byte  0xf3,0x44,0x0f,0x10,0x42,0x0c                   # movss         0xc(%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x0f,0x5b,0xc9                                  # cvtdq2ps      %xmm1,%xmm1
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x0f,0x5b,0xd2                                  # cvtdq2ps      %xmm2,%xmm2
  .byte  0x41,0x0f,0x59,0xd0                             # mulps         %xmm8,%xmm2
  .byte  0x66,0x0f,0x72,0xd3,0x18                        # psrld         $0x18,%xmm3
  .byte  0x0f,0x5b,0xdb                                  # cvtdq2ps      %xmm3,%xmm3
  .byte  0x41,0x0f,0x59,0xd8                             # mulps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_store_8888_sse41
_sk_store_8888_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x42,0x08                   # movss         0x8(%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xc8                             # mulps         %xmm0,%xmm9
  .byte  0x66,0x45,0x0f,0x5b,0xc9                        # cvtps2dq      %xmm9,%xmm9
  .byte  0x45,0x0f,0x28,0xd0                             # movaps        %xmm8,%xmm10
  .byte  0x44,0x0f,0x59,0xd1                             # mulps         %xmm1,%xmm10
  .byte  0x66,0x45,0x0f,0x5b,0xd2                        # cvtps2dq      %xmm10,%xmm10
  .byte  0x66,0x41,0x0f,0x72,0xf2,0x08                   # pslld         $0x8,%xmm10
  .byte  0x66,0x45,0x0f,0xeb,0xd1                        # por           %xmm9,%xmm10
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xca                             # mulps         %xmm2,%xmm9
  .byte  0x66,0x45,0x0f,0x5b,0xc9                        # cvtps2dq      %xmm9,%xmm9
  .byte  0x66,0x41,0x0f,0x72,0xf1,0x10                   # pslld         $0x10,%xmm9
  .byte  0x44,0x0f,0x59,0xc3                             # mulps         %xmm3,%xmm8
  .byte  0x66,0x45,0x0f,0x5b,0xc0                        # cvtps2dq      %xmm8,%xmm8
  .byte  0x66,0x41,0x0f,0x72,0xf0,0x18                   # pslld         $0x18,%xmm8
  .byte  0x66,0x45,0x0f,0xeb,0xc1                        # por           %xmm9,%xmm8
  .byte  0x66,0x45,0x0f,0xeb,0xc2                        # por           %xmm10,%xmm8
  .byte  0xf3,0x44,0x0f,0x7f,0x04,0xb8                   # movdqu        %xmm8,(%rax,%rdi,4)
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_load_f16_sse41
_sk_load_f16_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xf3,0x0f,0x6f,0x04,0xf8                        # movdqu        (%rax,%rdi,8),%xmm0
  .byte  0xf3,0x0f,0x6f,0x4c,0xf8,0x10                   # movdqu        0x10(%rax,%rdi,8),%xmm1
  .byte  0x66,0x0f,0x6f,0xd0                             # movdqa        %xmm0,%xmm2
  .byte  0x66,0x0f,0x61,0xd1                             # punpcklwd     %xmm1,%xmm2
  .byte  0x66,0x0f,0x69,0xc1                             # punpckhwd     %xmm1,%xmm0
  .byte  0x66,0x44,0x0f,0x6f,0xc2                        # movdqa        %xmm2,%xmm8
  .byte  0x66,0x44,0x0f,0x61,0xc0                        # punpcklwd     %xmm0,%xmm8
  .byte  0x66,0x0f,0x69,0xd0                             # punpckhwd     %xmm0,%xmm2
  .byte  0x66,0x0f,0x6e,0x42,0x64                        # movd          0x64(%rdx),%xmm0
  .byte  0x66,0x0f,0x70,0xd8,0x00                        # pshufd        $0x0,%xmm0,%xmm3
  .byte  0x66,0x0f,0x6f,0xcb                             # movdqa        %xmm3,%xmm1
  .byte  0x66,0x41,0x0f,0x65,0xc8                        # pcmpgtw       %xmm8,%xmm1
  .byte  0x66,0x41,0x0f,0xdf,0xc8                        # pandn         %xmm8,%xmm1
  .byte  0x66,0x0f,0x65,0xda                             # pcmpgtw       %xmm2,%xmm3
  .byte  0x66,0x0f,0xdf,0xda                             # pandn         %xmm2,%xmm3
  .byte  0x66,0x0f,0x38,0x33,0xc1                        # pmovzxwd      %xmm1,%xmm0
  .byte  0x66,0x0f,0x72,0xf0,0x0d                        # pslld         $0xd,%xmm0
  .byte  0x66,0x0f,0x6e,0x52,0x5c                        # movd          0x5c(%rdx),%xmm2
  .byte  0x66,0x44,0x0f,0x70,0xc2,0x00                   # pshufd        $0x0,%xmm2,%xmm8
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x66,0x45,0x0f,0xef,0xc9                        # pxor          %xmm9,%xmm9
  .byte  0x66,0x41,0x0f,0x69,0xc9                        # punpckhwd     %xmm9,%xmm1
  .byte  0x66,0x0f,0x72,0xf1,0x0d                        # pslld         $0xd,%xmm1
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x66,0x0f,0x38,0x33,0xd3                        # pmovzxwd      %xmm3,%xmm2
  .byte  0x66,0x0f,0x72,0xf2,0x0d                        # pslld         $0xd,%xmm2
  .byte  0x41,0x0f,0x59,0xd0                             # mulps         %xmm8,%xmm2
  .byte  0x66,0x41,0x0f,0x69,0xd9                        # punpckhwd     %xmm9,%xmm3
  .byte  0x66,0x0f,0x72,0xf3,0x0d                        # pslld         $0xd,%xmm3
  .byte  0x41,0x0f,0x59,0xd8                             # mulps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_store_f16_sse41
_sk_store_f16_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0x66,0x44,0x0f,0x6e,0x42,0x60                   # movd          0x60(%rdx),%xmm8
  .byte  0x66,0x45,0x0f,0x70,0xc0,0x00                   # pshufd        $0x0,%xmm8,%xmm8
  .byte  0x66,0x45,0x0f,0x6f,0xc8                        # movdqa        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xc8                             # mulps         %xmm0,%xmm9
  .byte  0x66,0x41,0x0f,0x72,0xd1,0x0d                   # psrld         $0xd,%xmm9
  .byte  0x66,0x45,0x0f,0x6f,0xd0                        # movdqa        %xmm8,%xmm10
  .byte  0x44,0x0f,0x59,0xd1                             # mulps         %xmm1,%xmm10
  .byte  0x66,0x41,0x0f,0x72,0xd2,0x0d                   # psrld         $0xd,%xmm10
  .byte  0x66,0x45,0x0f,0x6f,0xd8                        # movdqa        %xmm8,%xmm11
  .byte  0x44,0x0f,0x59,0xda                             # mulps         %xmm2,%xmm11
  .byte  0x66,0x41,0x0f,0x72,0xd3,0x0d                   # psrld         $0xd,%xmm11
  .byte  0x44,0x0f,0x59,0xc3                             # mulps         %xmm3,%xmm8
  .byte  0x66,0x41,0x0f,0x72,0xd0,0x0d                   # psrld         $0xd,%xmm8
  .byte  0x66,0x41,0x0f,0x73,0xfa,0x02                   # pslldq        $0x2,%xmm10
  .byte  0x66,0x45,0x0f,0xeb,0xd1                        # por           %xmm9,%xmm10
  .byte  0x66,0x41,0x0f,0x73,0xf8,0x02                   # pslldq        $0x2,%xmm8
  .byte  0x66,0x45,0x0f,0xeb,0xc3                        # por           %xmm11,%xmm8
  .byte  0x66,0x45,0x0f,0x6f,0xca                        # movdqa        %xmm10,%xmm9
  .byte  0x66,0x45,0x0f,0x62,0xc8                        # punpckldq     %xmm8,%xmm9
  .byte  0xf3,0x44,0x0f,0x7f,0x0c,0xf8                   # movdqu        %xmm9,(%rax,%rdi,8)
  .byte  0x66,0x45,0x0f,0x6a,0xd0                        # punpckhdq     %xmm8,%xmm10
  .byte  0xf3,0x44,0x0f,0x7f,0x54,0xf8,0x10              # movdqu        %xmm10,0x10(%rax,%rdi,8)
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_x_sse41
_sk_clamp_x_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x00                        # movss         (%rax),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x66,0x45,0x0f,0x76,0xc9                        # pcmpeqd       %xmm9,%xmm9
  .byte  0x66,0x45,0x0f,0xfe,0xc8                        # paddd         %xmm8,%xmm9
  .byte  0x41,0x0f,0x5d,0xc1                             # minps         %xmm9,%xmm0
  .byte  0x45,0x0f,0x57,0xc0                             # xorps         %xmm8,%xmm8
  .byte  0x44,0x0f,0x5f,0xc0                             # maxps         %xmm0,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xc0                             # movaps        %xmm8,%xmm0
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_y_sse41
_sk_clamp_y_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x00                        # movss         (%rax),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x66,0x45,0x0f,0x76,0xc9                        # pcmpeqd       %xmm9,%xmm9
  .byte  0x66,0x45,0x0f,0xfe,0xc8                        # paddd         %xmm8,%xmm9
  .byte  0x41,0x0f,0x5d,0xc9                             # minps         %xmm9,%xmm1
  .byte  0x45,0x0f,0x57,0xc0                             # xorps         %xmm8,%xmm8
  .byte  0x44,0x0f,0x5f,0xc1                             # maxps         %xmm1,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm1
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_matrix_2x3_sse41
_sk_matrix_2x3_sse41:
  .byte  0x44,0x0f,0x28,0xc9                             # movaps        %xmm1,%xmm9
  .byte  0x44,0x0f,0x28,0xc0                             # movaps        %xmm0,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x0f,0x10,0x00                             # movss         (%rax),%xmm0
  .byte  0xf3,0x0f,0x10,0x48,0x04                        # movss         0x4(%rax),%xmm1
  .byte  0x0f,0xc6,0xc0,0x00                             # shufps        $0x0,%xmm0,%xmm0
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x08                   # movss         0x8(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x10                   # movss         0x10(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0x45,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x41,0x0f,0x58,0xc2                             # addps         %xmm10,%xmm0
  .byte  0x0f,0xc6,0xc9,0x00                             # shufps        $0x0,%xmm1,%xmm1
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x0c                   # movss         0xc(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x14                   # movss         0x14(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0x45,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x41,0x0f,0x58,0xca                             # addps         %xmm10,%xmm1
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_matrix_3x4_sse41
_sk_matrix_3x4_sse41:
  .byte  0x44,0x0f,0x28,0xc9                             # movaps        %xmm1,%xmm9
  .byte  0x44,0x0f,0x28,0xc0                             # movaps        %xmm0,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x0f,0x10,0x00                             # movss         (%rax),%xmm0
  .byte  0xf3,0x0f,0x10,0x48,0x04                        # movss         0x4(%rax),%xmm1
  .byte  0x0f,0xc6,0xc0,0x00                             # shufps        $0x0,%xmm0,%xmm0
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x0c                   # movss         0xc(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x18                   # movss         0x18(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0xf3,0x44,0x0f,0x10,0x60,0x24                   # movss         0x24(%rax),%xmm12
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0x44,0x0f,0x59,0xda                             # mulps         %xmm2,%xmm11
  .byte  0x45,0x0f,0x58,0xdc                             # addps         %xmm12,%xmm11
  .byte  0x45,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x41,0x0f,0x58,0xc2                             # addps         %xmm10,%xmm0
  .byte  0x0f,0xc6,0xc9,0x00                             # shufps        $0x0,%xmm1,%xmm1
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x10                   # movss         0x10(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x1c                   # movss         0x1c(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0xf3,0x44,0x0f,0x10,0x60,0x28                   # movss         0x28(%rax),%xmm12
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0x44,0x0f,0x59,0xda                             # mulps         %xmm2,%xmm11
  .byte  0x45,0x0f,0x58,0xdc                             # addps         %xmm12,%xmm11
  .byte  0x45,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x41,0x0f,0x58,0xca                             # addps         %xmm10,%xmm1
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x08                   # movss         0x8(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x14                   # movss         0x14(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0xf3,0x44,0x0f,0x10,0x60,0x20                   # movss         0x20(%rax),%xmm12
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0xf3,0x44,0x0f,0x10,0x68,0x2c                   # movss         0x2c(%rax),%xmm13
  .byte  0x45,0x0f,0xc6,0xed,0x00                        # shufps        $0x0,%xmm13,%xmm13
  .byte  0x44,0x0f,0x59,0xe2                             # mulps         %xmm2,%xmm12
  .byte  0x45,0x0f,0x58,0xe5                             # addps         %xmm13,%xmm12
  .byte  0x45,0x0f,0x59,0xd9                             # mulps         %xmm9,%xmm11
  .byte  0x45,0x0f,0x58,0xdc                             # addps         %xmm12,%xmm11
  .byte  0x45,0x0f,0x59,0xd0                             # mulps         %xmm8,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xd2                             # movaps        %xmm10,%xmm2
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_linear_gradient_2stops_sse41
_sk_linear_gradient_2stops_sse41:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x44,0x0f,0x10,0x08                             # movups        (%rax),%xmm9
  .byte  0x0f,0x10,0x58,0x10                             # movups        0x10(%rax),%xmm3
  .byte  0x44,0x0f,0x28,0xc3                             # movaps        %xmm3,%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x41,0x0f,0x28,0xc9                             # movaps        %xmm9,%xmm1
  .byte  0x0f,0xc6,0xc9,0x00                             # shufps        $0x0,%xmm1,%xmm1
  .byte  0x44,0x0f,0x59,0xc0                             # mulps         %xmm0,%xmm8
  .byte  0x44,0x0f,0x58,0xc1                             # addps         %xmm1,%xmm8
  .byte  0x0f,0x28,0xcb                                  # movaps        %xmm3,%xmm1
  .byte  0x0f,0xc6,0xc9,0x55                             # shufps        $0x55,%xmm1,%xmm1
  .byte  0x41,0x0f,0x28,0xd1                             # movaps        %xmm9,%xmm2
  .byte  0x0f,0xc6,0xd2,0x55                             # shufps        $0x55,%xmm2,%xmm2
  .byte  0x0f,0x59,0xc8                                  # mulps         %xmm0,%xmm1
  .byte  0x0f,0x58,0xca                                  # addps         %xmm2,%xmm1
  .byte  0x0f,0x28,0xd3                                  # movaps        %xmm3,%xmm2
  .byte  0x0f,0xc6,0xd2,0xaa                             # shufps        $0xaa,%xmm2,%xmm2
  .byte  0x45,0x0f,0x28,0xd1                             # movaps        %xmm9,%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0xaa                        # shufps        $0xaa,%xmm10,%xmm10
  .byte  0x0f,0x59,0xd0                                  # mulps         %xmm0,%xmm2
  .byte  0x41,0x0f,0x58,0xd2                             # addps         %xmm10,%xmm2
  .byte  0x0f,0xc6,0xdb,0xff                             # shufps        $0xff,%xmm3,%xmm3
  .byte  0x45,0x0f,0xc6,0xc9,0xff                        # shufps        $0xff,%xmm9,%xmm9
  .byte  0x0f,0x59,0xd8                                  # mulps         %xmm0,%xmm3
  .byte  0x41,0x0f,0x58,0xd9                             # addps         %xmm9,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xc0                             # movaps        %xmm8,%xmm0
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_start_pipeline_sse2
_sk_start_pipeline_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x57,0xc0                                  # xorps         %xmm0,%xmm0
  .byte  0x0f,0x57,0xc9                                  # xorps         %xmm1,%xmm1
  .byte  0x0f,0x57,0xd2                                  # xorps         %xmm2,%xmm2
  .byte  0x0f,0x57,0xdb                                  # xorps         %xmm3,%xmm3
  .byte  0x0f,0x57,0xe4                                  # xorps         %xmm4,%xmm4
  .byte  0x0f,0x57,0xed                                  # xorps         %xmm5,%xmm5
  .byte  0x0f,0x57,0xf6                                  # xorps         %xmm6,%xmm6
  .byte  0x0f,0x57,0xff                                  # xorps         %xmm7,%xmm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_start_pipeline_ms_sse2
_sk_start_pipeline_ms_sse2:
  .byte  0x56                                            # push          %rsi
  .byte  0x57                                            # push          %rdi
  .byte  0x48,0x81,0xec,0xa8,0x00,0x00,0x00              # sub           $0xa8,%rsp
  .byte  0x44,0x0f,0x29,0xbc,0x24,0x90,0x00,0x00,0x00    # movaps        %xmm15,0x90(%rsp)
  .byte  0x44,0x0f,0x29,0xb4,0x24,0x80,0x00,0x00,0x00    # movaps        %xmm14,0x80(%rsp)
  .byte  0x44,0x0f,0x29,0x6c,0x24,0x70                   # movaps        %xmm13,0x70(%rsp)
  .byte  0x44,0x0f,0x29,0x64,0x24,0x60                   # movaps        %xmm12,0x60(%rsp)
  .byte  0x44,0x0f,0x29,0x5c,0x24,0x50                   # movaps        %xmm11,0x50(%rsp)
  .byte  0x44,0x0f,0x29,0x54,0x24,0x40                   # movaps        %xmm10,0x40(%rsp)
  .byte  0x44,0x0f,0x29,0x4c,0x24,0x30                   # movaps        %xmm9,0x30(%rsp)
  .byte  0x44,0x0f,0x29,0x44,0x24,0x20                   # movaps        %xmm8,0x20(%rsp)
  .byte  0x0f,0x29,0x7c,0x24,0x10                        # movaps        %xmm7,0x10(%rsp)
  .byte  0x0f,0x29,0x34,0x24                             # movaps        %xmm6,(%rsp)
  .byte  0x48,0x89,0xd6                                  # mov           %rdx,%rsi
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x57,0xc0                                  # xorps         %xmm0,%xmm0
  .byte  0x0f,0x57,0xc9                                  # xorps         %xmm1,%xmm1
  .byte  0x0f,0x57,0xd2                                  # xorps         %xmm2,%xmm2
  .byte  0x0f,0x57,0xdb                                  # xorps         %xmm3,%xmm3
  .byte  0x0f,0x57,0xe4                                  # xorps         %xmm4,%xmm4
  .byte  0x0f,0x57,0xed                                  # xorps         %xmm5,%xmm5
  .byte  0x0f,0x57,0xf6                                  # xorps         %xmm6,%xmm6
  .byte  0x0f,0x57,0xff                                  # xorps         %xmm7,%xmm7
  .byte  0x48,0x89,0xcf                                  # mov           %rcx,%rdi
  .byte  0x4c,0x89,0xc2                                  # mov           %r8,%rdx
  .byte  0xff,0xd0                                       # callq         *%rax
  .byte  0x0f,0x28,0x34,0x24                             # movaps        (%rsp),%xmm6
  .byte  0x0f,0x28,0x7c,0x24,0x10                        # movaps        0x10(%rsp),%xmm7
  .byte  0x44,0x0f,0x28,0x44,0x24,0x20                   # movaps        0x20(%rsp),%xmm8
  .byte  0x44,0x0f,0x28,0x4c,0x24,0x30                   # movaps        0x30(%rsp),%xmm9
  .byte  0x44,0x0f,0x28,0x54,0x24,0x40                   # movaps        0x40(%rsp),%xmm10
  .byte  0x44,0x0f,0x28,0x5c,0x24,0x50                   # movaps        0x50(%rsp),%xmm11
  .byte  0x44,0x0f,0x28,0x64,0x24,0x60                   # movaps        0x60(%rsp),%xmm12
  .byte  0x44,0x0f,0x28,0x6c,0x24,0x70                   # movaps        0x70(%rsp),%xmm13
  .byte  0x44,0x0f,0x28,0xb4,0x24,0x80,0x00,0x00,0x00    # movaps        0x80(%rsp),%xmm14
  .byte  0x44,0x0f,0x28,0xbc,0x24,0x90,0x00,0x00,0x00    # movaps        0x90(%rsp),%xmm15
  .byte  0x48,0x81,0xc4,0xa8,0x00,0x00,0x00              # add           $0xa8,%rsp
  .byte  0x5f                                            # pop           %rdi
  .byte  0x5e                                            # pop           %rsi
  .byte  0xc3                                            # retq

.globl _sk_just_return_sse2
_sk_just_return_sse2:
  .byte  0xc3                                            # retq

.globl _sk_seed_shader_sse2
_sk_seed_shader_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x66,0x0f,0x6e,0xc7                             # movd          %edi,%xmm0
  .byte  0x66,0x0f,0x70,0xc0,0x00                        # pshufd        $0x0,%xmm0,%xmm0
  .byte  0x0f,0x5b,0xc8                                  # cvtdq2ps      %xmm0,%xmm1
  .byte  0xf3,0x0f,0x10,0x12                             # movss         (%rdx),%xmm2
  .byte  0xf3,0x0f,0x10,0x5a,0x04                        # movss         0x4(%rdx),%xmm3
  .byte  0x0f,0xc6,0xdb,0x00                             # shufps        $0x0,%xmm3,%xmm3
  .byte  0x0f,0x58,0xcb                                  # addps         %xmm3,%xmm1
  .byte  0x0f,0x10,0x42,0x14                             # movups        0x14(%rdx),%xmm0
  .byte  0x0f,0x58,0xc1                                  # addps         %xmm1,%xmm0
  .byte  0x66,0x0f,0x6e,0x08                             # movd          (%rax),%xmm1
  .byte  0x66,0x0f,0x70,0xc9,0x00                        # pshufd        $0x0,%xmm1,%xmm1
  .byte  0x0f,0x5b,0xc9                                  # cvtdq2ps      %xmm1,%xmm1
  .byte  0x0f,0x58,0xcb                                  # addps         %xmm3,%xmm1
  .byte  0x0f,0xc6,0xd2,0x00                             # shufps        $0x0,%xmm2,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x57,0xdb                                  # xorps         %xmm3,%xmm3
  .byte  0x0f,0x57,0xe4                                  # xorps         %xmm4,%xmm4
  .byte  0x0f,0x57,0xed                                  # xorps         %xmm5,%xmm5
  .byte  0x0f,0x57,0xf6                                  # xorps         %xmm6,%xmm6
  .byte  0x0f,0x57,0xff                                  # xorps         %xmm7,%xmm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_constant_color_sse2
_sk_constant_color_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x10,0x18                                  # movups        (%rax),%xmm3
  .byte  0x0f,0x28,0xc3                                  # movaps        %xmm3,%xmm0
  .byte  0x0f,0xc6,0xc0,0x00                             # shufps        $0x0,%xmm0,%xmm0
  .byte  0x0f,0x28,0xcb                                  # movaps        %xmm3,%xmm1
  .byte  0x0f,0xc6,0xc9,0x55                             # shufps        $0x55,%xmm1,%xmm1
  .byte  0x0f,0x28,0xd3                                  # movaps        %xmm3,%xmm2
  .byte  0x0f,0xc6,0xd2,0xaa                             # shufps        $0xaa,%xmm2,%xmm2
  .byte  0x0f,0xc6,0xdb,0xff                             # shufps        $0xff,%xmm3,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clear_sse2
_sk_clear_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x57,0xc0                                  # xorps         %xmm0,%xmm0
  .byte  0x0f,0x57,0xc9                                  # xorps         %xmm1,%xmm1
  .byte  0x0f,0x57,0xd2                                  # xorps         %xmm2,%xmm2
  .byte  0x0f,0x57,0xdb                                  # xorps         %xmm3,%xmm3
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_plus__sse2
_sk_plus__sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x58,0xc4                                  # addps         %xmm4,%xmm0
  .byte  0x0f,0x58,0xcd                                  # addps         %xmm5,%xmm1
  .byte  0x0f,0x58,0xd6                                  # addps         %xmm6,%xmm2
  .byte  0x0f,0x58,0xdf                                  # addps         %xmm7,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_srcover_sse2
_sk_srcover_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x02                        # movss         (%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x44,0x0f,0x5c,0xc3                             # subps         %xmm3,%xmm8
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xcc                             # mulps         %xmm4,%xmm9
  .byte  0x41,0x0f,0x58,0xc1                             # addps         %xmm9,%xmm0
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xcd                             # mulps         %xmm5,%xmm9
  .byte  0x41,0x0f,0x58,0xc9                             # addps         %xmm9,%xmm1
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xce                             # mulps         %xmm6,%xmm9
  .byte  0x41,0x0f,0x58,0xd1                             # addps         %xmm9,%xmm2
  .byte  0x44,0x0f,0x59,0xc7                             # mulps         %xmm7,%xmm8
  .byte  0x41,0x0f,0x58,0xd8                             # addps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_dstover_sse2
_sk_dstover_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x02                        # movss         (%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x44,0x0f,0x5c,0xc7                             # subps         %xmm7,%xmm8
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x0f,0x58,0xc4                                  # addps         %xmm4,%xmm0
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x0f,0x58,0xcd                                  # addps         %xmm5,%xmm1
  .byte  0x41,0x0f,0x59,0xd0                             # mulps         %xmm8,%xmm2
  .byte  0x0f,0x58,0xd6                                  # addps         %xmm6,%xmm2
  .byte  0x41,0x0f,0x59,0xd8                             # mulps         %xmm8,%xmm3
  .byte  0x0f,0x58,0xdf                                  # addps         %xmm7,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_0_sse2
_sk_clamp_0_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x45,0x0f,0x57,0xc0                             # xorps         %xmm8,%xmm8
  .byte  0x41,0x0f,0x5f,0xc0                             # maxps         %xmm8,%xmm0
  .byte  0x41,0x0f,0x5f,0xc8                             # maxps         %xmm8,%xmm1
  .byte  0x41,0x0f,0x5f,0xd0                             # maxps         %xmm8,%xmm2
  .byte  0x41,0x0f,0x5f,0xd8                             # maxps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_1_sse2
_sk_clamp_1_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x02                        # movss         (%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x41,0x0f,0x5d,0xc0                             # minps         %xmm8,%xmm0
  .byte  0x41,0x0f,0x5d,0xc8                             # minps         %xmm8,%xmm1
  .byte  0x41,0x0f,0x5d,0xd0                             # minps         %xmm8,%xmm2
  .byte  0x41,0x0f,0x5d,0xd8                             # minps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_a_sse2
_sk_clamp_a_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x02                        # movss         (%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x41,0x0f,0x5d,0xd8                             # minps         %xmm8,%xmm3
  .byte  0x0f,0x5d,0xc3                                  # minps         %xmm3,%xmm0
  .byte  0x0f,0x5d,0xcb                                  # minps         %xmm3,%xmm1
  .byte  0x0f,0x5d,0xd3                                  # minps         %xmm3,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_swap_sse2
_sk_swap_sse2:
  .byte  0x44,0x0f,0x28,0xc3                             # movaps        %xmm3,%xmm8
  .byte  0x44,0x0f,0x28,0xca                             # movaps        %xmm2,%xmm9
  .byte  0x44,0x0f,0x28,0xd1                             # movaps        %xmm1,%xmm10
  .byte  0x44,0x0f,0x28,0xd8                             # movaps        %xmm0,%xmm11
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x28,0xc4                                  # movaps        %xmm4,%xmm0
  .byte  0x0f,0x28,0xcd                                  # movaps        %xmm5,%xmm1
  .byte  0x0f,0x28,0xd6                                  # movaps        %xmm6,%xmm2
  .byte  0x0f,0x28,0xdf                                  # movaps        %xmm7,%xmm3
  .byte  0x41,0x0f,0x28,0xe3                             # movaps        %xmm11,%xmm4
  .byte  0x41,0x0f,0x28,0xea                             # movaps        %xmm10,%xmm5
  .byte  0x41,0x0f,0x28,0xf1                             # movaps        %xmm9,%xmm6
  .byte  0x41,0x0f,0x28,0xf8                             # movaps        %xmm8,%xmm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_move_src_dst_sse2
_sk_move_src_dst_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x28,0xe0                                  # movaps        %xmm0,%xmm4
  .byte  0x0f,0x28,0xe9                                  # movaps        %xmm1,%xmm5
  .byte  0x0f,0x28,0xf2                                  # movaps        %xmm2,%xmm6
  .byte  0x0f,0x28,0xfb                                  # movaps        %xmm3,%xmm7
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_move_dst_src_sse2
_sk_move_dst_src_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x28,0xc4                                  # movaps        %xmm4,%xmm0
  .byte  0x0f,0x28,0xcd                                  # movaps        %xmm5,%xmm1
  .byte  0x0f,0x28,0xd6                                  # movaps        %xmm6,%xmm2
  .byte  0x0f,0x28,0xdf                                  # movaps        %xmm7,%xmm3
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_premul_sse2
_sk_premul_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x59,0xc3                                  # mulps         %xmm3,%xmm0
  .byte  0x0f,0x59,0xcb                                  # mulps         %xmm3,%xmm1
  .byte  0x0f,0x59,0xd3                                  # mulps         %xmm3,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_unpremul_sse2
_sk_unpremul_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x45,0x0f,0x57,0xc0                             # xorps         %xmm8,%xmm8
  .byte  0x44,0x0f,0xc2,0xc3,0x00                        # cmpeqps       %xmm3,%xmm8
  .byte  0xf3,0x44,0x0f,0x10,0x0a                        # movss         (%rdx),%xmm9
  .byte  0x45,0x0f,0xc6,0xc9,0x00                        # shufps        $0x0,%xmm9,%xmm9
  .byte  0x44,0x0f,0x5e,0xcb                             # divps         %xmm3,%xmm9
  .byte  0x45,0x0f,0x55,0xc1                             # andnps        %xmm9,%xmm8
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x41,0x0f,0x59,0xd0                             # mulps         %xmm8,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_from_srgb_sse2
_sk_from_srgb_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x42,0x40                   # movss         0x40(%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x45,0x0f,0x28,0xe8                             # movaps        %xmm8,%xmm13
  .byte  0x44,0x0f,0x59,0xe8                             # mulps         %xmm0,%xmm13
  .byte  0x44,0x0f,0x28,0xe0                             # movaps        %xmm0,%xmm12
  .byte  0x45,0x0f,0x59,0xe4                             # mulps         %xmm12,%xmm12
  .byte  0xf3,0x44,0x0f,0x10,0x4a,0x3c                   # movss         0x3c(%rdx),%xmm9
  .byte  0x45,0x0f,0xc6,0xc9,0x00                        # shufps        $0x0,%xmm9,%xmm9
  .byte  0xf3,0x44,0x0f,0x10,0x52,0x34                   # movss         0x34(%rdx),%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x5a,0x38                   # movss         0x38(%rdx),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0x45,0x0f,0x28,0xf1                             # movaps        %xmm9,%xmm14
  .byte  0x44,0x0f,0x59,0xf0                             # mulps         %xmm0,%xmm14
  .byte  0x45,0x0f,0x58,0xf3                             # addps         %xmm11,%xmm14
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0x45,0x0f,0x59,0xf4                             # mulps         %xmm12,%xmm14
  .byte  0x45,0x0f,0x58,0xf2                             # addps         %xmm10,%xmm14
  .byte  0xf3,0x44,0x0f,0x10,0x62,0x44                   # movss         0x44(%rdx),%xmm12
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0x41,0x0f,0xc2,0xc4,0x01                        # cmpltps       %xmm12,%xmm0
  .byte  0x44,0x0f,0x54,0xe8                             # andps         %xmm0,%xmm13
  .byte  0x41,0x0f,0x55,0xc6                             # andnps        %xmm14,%xmm0
  .byte  0x41,0x0f,0x56,0xc5                             # orps          %xmm13,%xmm0
  .byte  0x45,0x0f,0x28,0xe8                             # movaps        %xmm8,%xmm13
  .byte  0x44,0x0f,0x59,0xe9                             # mulps         %xmm1,%xmm13
  .byte  0x44,0x0f,0x28,0xf1                             # movaps        %xmm1,%xmm14
  .byte  0x45,0x0f,0x59,0xf6                             # mulps         %xmm14,%xmm14
  .byte  0x45,0x0f,0x28,0xf9                             # movaps        %xmm9,%xmm15
  .byte  0x44,0x0f,0x59,0xf9                             # mulps         %xmm1,%xmm15
  .byte  0x45,0x0f,0x58,0xfb                             # addps         %xmm11,%xmm15
  .byte  0x45,0x0f,0x59,0xfe                             # mulps         %xmm14,%xmm15
  .byte  0x45,0x0f,0x58,0xfa                             # addps         %xmm10,%xmm15
  .byte  0x41,0x0f,0xc2,0xcc,0x01                        # cmpltps       %xmm12,%xmm1
  .byte  0x44,0x0f,0x54,0xe9                             # andps         %xmm1,%xmm13
  .byte  0x41,0x0f,0x55,0xcf                             # andnps        %xmm15,%xmm1
  .byte  0x41,0x0f,0x56,0xcd                             # orps          %xmm13,%xmm1
  .byte  0x44,0x0f,0x59,0xc2                             # mulps         %xmm2,%xmm8
  .byte  0x44,0x0f,0x28,0xea                             # movaps        %xmm2,%xmm13
  .byte  0x45,0x0f,0x59,0xed                             # mulps         %xmm13,%xmm13
  .byte  0x44,0x0f,0x59,0xca                             # mulps         %xmm2,%xmm9
  .byte  0x45,0x0f,0x58,0xcb                             # addps         %xmm11,%xmm9
  .byte  0x45,0x0f,0x59,0xcd                             # mulps         %xmm13,%xmm9
  .byte  0x45,0x0f,0x58,0xca                             # addps         %xmm10,%xmm9
  .byte  0x41,0x0f,0xc2,0xd4,0x01                        # cmpltps       %xmm12,%xmm2
  .byte  0x44,0x0f,0x54,0xc2                             # andps         %xmm2,%xmm8
  .byte  0x41,0x0f,0x55,0xd1                             # andnps        %xmm9,%xmm2
  .byte  0x41,0x0f,0x56,0xd0                             # orps          %xmm8,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_to_srgb_sse2
_sk_to_srgb_sse2:
  .byte  0x48,0x83,0xec,0x28                             # sub           $0x28,%rsp
  .byte  0x0f,0x29,0x7c,0x24,0x10                        # movaps        %xmm7,0x10(%rsp)
  .byte  0x0f,0x29,0x34,0x24                             # movaps        %xmm6,(%rsp)
  .byte  0x0f,0x28,0xf5                                  # movaps        %xmm5,%xmm6
  .byte  0x0f,0x28,0xec                                  # movaps        %xmm4,%xmm5
  .byte  0x0f,0x28,0xe3                                  # movaps        %xmm3,%xmm4
  .byte  0x44,0x0f,0x52,0xc0                             # rsqrtps       %xmm0,%xmm8
  .byte  0x45,0x0f,0x53,0xe8                             # rcpps         %xmm8,%xmm13
  .byte  0x45,0x0f,0x52,0xf8                             # rsqrtps       %xmm8,%xmm15
  .byte  0xf3,0x0f,0x10,0x1a                             # movss         (%rdx),%xmm3
  .byte  0xf3,0x44,0x0f,0x10,0x42,0x48                   # movss         0x48(%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x45,0x0f,0x28,0xf0                             # movaps        %xmm8,%xmm14
  .byte  0x44,0x0f,0x59,0xf0                             # mulps         %xmm0,%xmm14
  .byte  0x0f,0xc6,0xdb,0x00                             # shufps        $0x0,%xmm3,%xmm3
  .byte  0xf3,0x44,0x0f,0x10,0x52,0x4c                   # movss         0x4c(%rdx),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x5a,0x50                   # movss         0x50(%rdx),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0xf3,0x44,0x0f,0x10,0x62,0x54                   # movss         0x54(%rdx),%xmm12
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0x45,0x0f,0x59,0xeb                             # mulps         %xmm11,%xmm13
  .byte  0x45,0x0f,0x58,0xec                             # addps         %xmm12,%xmm13
  .byte  0x45,0x0f,0x59,0xfa                             # mulps         %xmm10,%xmm15
  .byte  0x45,0x0f,0x58,0xfd                             # addps         %xmm13,%xmm15
  .byte  0x44,0x0f,0x28,0xcb                             # movaps        %xmm3,%xmm9
  .byte  0x45,0x0f,0x5d,0xcf                             # minps         %xmm15,%xmm9
  .byte  0xf3,0x44,0x0f,0x10,0x6a,0x58                   # movss         0x58(%rdx),%xmm13
  .byte  0x45,0x0f,0xc6,0xed,0x00                        # shufps        $0x0,%xmm13,%xmm13
  .byte  0x41,0x0f,0xc2,0xc5,0x01                        # cmpltps       %xmm13,%xmm0
  .byte  0x44,0x0f,0x54,0xf0                             # andps         %xmm0,%xmm14
  .byte  0x41,0x0f,0x55,0xc1                             # andnps        %xmm9,%xmm0
  .byte  0x41,0x0f,0x56,0xc6                             # orps          %xmm14,%xmm0
  .byte  0x44,0x0f,0x52,0xc9                             # rsqrtps       %xmm1,%xmm9
  .byte  0x45,0x0f,0x53,0xf1                             # rcpps         %xmm9,%xmm14
  .byte  0x45,0x0f,0x52,0xc9                             # rsqrtps       %xmm9,%xmm9
  .byte  0x45,0x0f,0x59,0xf3                             # mulps         %xmm11,%xmm14
  .byte  0x45,0x0f,0x58,0xf4                             # addps         %xmm12,%xmm14
  .byte  0x45,0x0f,0x59,0xca                             # mulps         %xmm10,%xmm9
  .byte  0x45,0x0f,0x58,0xce                             # addps         %xmm14,%xmm9
  .byte  0x44,0x0f,0x28,0xf3                             # movaps        %xmm3,%xmm14
  .byte  0x45,0x0f,0x5d,0xf1                             # minps         %xmm9,%xmm14
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xc9                             # mulps         %xmm1,%xmm9
  .byte  0x41,0x0f,0xc2,0xcd,0x01                        # cmpltps       %xmm13,%xmm1
  .byte  0x44,0x0f,0x54,0xc9                             # andps         %xmm1,%xmm9
  .byte  0x41,0x0f,0x55,0xce                             # andnps        %xmm14,%xmm1
  .byte  0x41,0x0f,0x56,0xc9                             # orps          %xmm9,%xmm1
  .byte  0x44,0x0f,0x52,0xca                             # rsqrtps       %xmm2,%xmm9
  .byte  0x45,0x0f,0x53,0xf1                             # rcpps         %xmm9,%xmm14
  .byte  0x45,0x0f,0x59,0xf3                             # mulps         %xmm11,%xmm14
  .byte  0x45,0x0f,0x58,0xf4                             # addps         %xmm12,%xmm14
  .byte  0x41,0x0f,0x52,0xf9                             # rsqrtps       %xmm9,%xmm7
  .byte  0x41,0x0f,0x59,0xfa                             # mulps         %xmm10,%xmm7
  .byte  0x41,0x0f,0x58,0xfe                             # addps         %xmm14,%xmm7
  .byte  0x0f,0x5d,0xdf                                  # minps         %xmm7,%xmm3
  .byte  0x44,0x0f,0x59,0xc2                             # mulps         %xmm2,%xmm8
  .byte  0x41,0x0f,0xc2,0xd5,0x01                        # cmpltps       %xmm13,%xmm2
  .byte  0x44,0x0f,0x54,0xc2                             # andps         %xmm2,%xmm8
  .byte  0x0f,0x55,0xd3                                  # andnps        %xmm3,%xmm2
  .byte  0x41,0x0f,0x56,0xd0                             # orps          %xmm8,%xmm2
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x0f,0x28,0xdc                                  # movaps        %xmm4,%xmm3
  .byte  0x0f,0x28,0xe5                                  # movaps        %xmm5,%xmm4
  .byte  0x0f,0x28,0xee                                  # movaps        %xmm6,%xmm5
  .byte  0x0f,0x28,0x34,0x24                             # movaps        (%rsp),%xmm6
  .byte  0x0f,0x28,0x7c,0x24,0x10                        # movaps        0x10(%rsp),%xmm7
  .byte  0x48,0x83,0xc4,0x28                             # add           $0x28,%rsp
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_scale_u8_sse2
_sk_scale_u8_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0x66,0x44,0x0f,0x6e,0x04,0x38                   # movd          (%rax,%rdi,1),%xmm8
  .byte  0x66,0x45,0x0f,0xef,0xc9                        # pxor          %xmm9,%xmm9
  .byte  0x66,0x45,0x0f,0x60,0xc1                        # punpcklbw     %xmm9,%xmm8
  .byte  0x66,0x45,0x0f,0x61,0xc1                        # punpcklwd     %xmm9,%xmm8
  .byte  0x45,0x0f,0x5b,0xc0                             # cvtdq2ps      %xmm8,%xmm8
  .byte  0xf3,0x44,0x0f,0x10,0x4a,0x0c                   # movss         0xc(%rdx),%xmm9
  .byte  0x45,0x0f,0xc6,0xc9,0x00                        # shufps        $0x0,%xmm9,%xmm9
  .byte  0x45,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm9
  .byte  0x41,0x0f,0x59,0xc1                             # mulps         %xmm9,%xmm0
  .byte  0x41,0x0f,0x59,0xc9                             # mulps         %xmm9,%xmm1
  .byte  0x41,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm2
  .byte  0x41,0x0f,0x59,0xd9                             # mulps         %xmm9,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_load_tables_sse2
_sk_load_tables_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x08                                  # mov           (%rax),%rcx
  .byte  0x4c,0x8b,0x40,0x08                             # mov           0x8(%rax),%r8
  .byte  0xf3,0x44,0x0f,0x6f,0x04,0xb9                   # movdqu        (%rcx,%rdi,4),%xmm8
  .byte  0x66,0x0f,0x6e,0x42,0x10                        # movd          0x10(%rdx),%xmm0
  .byte  0x66,0x0f,0x70,0xc0,0x00                        # pshufd        $0x0,%xmm0,%xmm0
  .byte  0x66,0x45,0x0f,0x6f,0xc8                        # movdqa        %xmm8,%xmm9
  .byte  0x66,0x41,0x0f,0x72,0xd1,0x08                   # psrld         $0x8,%xmm9
  .byte  0x66,0x44,0x0f,0xdb,0xc8                        # pand          %xmm0,%xmm9
  .byte  0x66,0x45,0x0f,0x6f,0xd0                        # movdqa        %xmm8,%xmm10
  .byte  0x66,0x41,0x0f,0x72,0xd2,0x10                   # psrld         $0x10,%xmm10
  .byte  0x66,0x44,0x0f,0xdb,0xd0                        # pand          %xmm0,%xmm10
  .byte  0x66,0x41,0x0f,0xdb,0xc0                        # pand          %xmm8,%xmm0
  .byte  0x66,0x0f,0x70,0xd8,0x4e                        # pshufd        $0x4e,%xmm0,%xmm3
  .byte  0x66,0x48,0x0f,0x7e,0xd9                        # movq          %xmm3,%rcx
  .byte  0x41,0x89,0xc9                                  # mov           %ecx,%r9d
  .byte  0x48,0xc1,0xe9,0x20                             # shr           $0x20,%rcx
  .byte  0x66,0x49,0x0f,0x7e,0xc2                        # movq          %xmm0,%r10
  .byte  0x45,0x89,0xd3                                  # mov           %r10d,%r11d
  .byte  0x49,0xc1,0xea,0x20                             # shr           $0x20,%r10
  .byte  0xf3,0x43,0x0f,0x10,0x1c,0x90                   # movss         (%r8,%r10,4),%xmm3
  .byte  0xf3,0x41,0x0f,0x10,0x04,0x88                   # movss         (%r8,%rcx,4),%xmm0
  .byte  0x0f,0x14,0xd8                                  # unpcklps      %xmm0,%xmm3
  .byte  0xf3,0x43,0x0f,0x10,0x04,0x98                   # movss         (%r8,%r11,4),%xmm0
  .byte  0xf3,0x43,0x0f,0x10,0x0c,0x88                   # movss         (%r8,%r9,4),%xmm1
  .byte  0x0f,0x14,0xc1                                  # unpcklps      %xmm1,%xmm0
  .byte  0x0f,0x14,0xc3                                  # unpcklps      %xmm3,%xmm0
  .byte  0x48,0x8b,0x48,0x10                             # mov           0x10(%rax),%rcx
  .byte  0x66,0x41,0x0f,0x70,0xc9,0x4e                   # pshufd        $0x4e,%xmm9,%xmm1
  .byte  0x66,0x49,0x0f,0x7e,0xc8                        # movq          %xmm1,%r8
  .byte  0x45,0x89,0xc1                                  # mov           %r8d,%r9d
  .byte  0x49,0xc1,0xe8,0x20                             # shr           $0x20,%r8
  .byte  0x66,0x4d,0x0f,0x7e,0xca                        # movq          %xmm9,%r10
  .byte  0x45,0x89,0xd3                                  # mov           %r10d,%r11d
  .byte  0x49,0xc1,0xea,0x20                             # shr           $0x20,%r10
  .byte  0xf3,0x42,0x0f,0x10,0x1c,0x91                   # movss         (%rcx,%r10,4),%xmm3
  .byte  0xf3,0x42,0x0f,0x10,0x0c,0x81                   # movss         (%rcx,%r8,4),%xmm1
  .byte  0x0f,0x14,0xd9                                  # unpcklps      %xmm1,%xmm3
  .byte  0xf3,0x42,0x0f,0x10,0x0c,0x99                   # movss         (%rcx,%r11,4),%xmm1
  .byte  0xf3,0x42,0x0f,0x10,0x14,0x89                   # movss         (%rcx,%r9,4),%xmm2
  .byte  0x0f,0x14,0xca                                  # unpcklps      %xmm2,%xmm1
  .byte  0x0f,0x14,0xcb                                  # unpcklps      %xmm3,%xmm1
  .byte  0x48,0x8b,0x40,0x18                             # mov           0x18(%rax),%rax
  .byte  0x66,0x41,0x0f,0x70,0xd2,0x4e                   # pshufd        $0x4e,%xmm10,%xmm2
  .byte  0x66,0x48,0x0f,0x7e,0xd1                        # movq          %xmm2,%rcx
  .byte  0x41,0x89,0xc8                                  # mov           %ecx,%r8d
  .byte  0x48,0xc1,0xe9,0x20                             # shr           $0x20,%rcx
  .byte  0x66,0x4d,0x0f,0x7e,0xd1                        # movq          %xmm10,%r9
  .byte  0x45,0x89,0xca                                  # mov           %r9d,%r10d
  .byte  0x49,0xc1,0xe9,0x20                             # shr           $0x20,%r9
  .byte  0xf3,0x46,0x0f,0x10,0x0c,0x88                   # movss         (%rax,%r9,4),%xmm9
  .byte  0xf3,0x0f,0x10,0x14,0x88                        # movss         (%rax,%rcx,4),%xmm2
  .byte  0x44,0x0f,0x14,0xca                             # unpcklps      %xmm2,%xmm9
  .byte  0xf3,0x42,0x0f,0x10,0x14,0x90                   # movss         (%rax,%r10,4),%xmm2
  .byte  0xf3,0x42,0x0f,0x10,0x1c,0x80                   # movss         (%rax,%r8,4),%xmm3
  .byte  0x0f,0x14,0xd3                                  # unpcklps      %xmm3,%xmm2
  .byte  0x41,0x0f,0x14,0xd1                             # unpcklps      %xmm9,%xmm2
  .byte  0x66,0x41,0x0f,0x72,0xd0,0x18                   # psrld         $0x18,%xmm8
  .byte  0x45,0x0f,0x5b,0xc0                             # cvtdq2ps      %xmm8,%xmm8
  .byte  0xf3,0x0f,0x10,0x5a,0x0c                        # movss         0xc(%rdx),%xmm3
  .byte  0x0f,0xc6,0xdb,0x00                             # shufps        $0x0,%xmm3,%xmm3
  .byte  0x41,0x0f,0x59,0xd8                             # mulps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_load_8888_sse2
_sk_load_8888_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xf3,0x0f,0x6f,0x1c,0xb8                        # movdqu        (%rax,%rdi,4),%xmm3
  .byte  0x66,0x0f,0x6e,0x42,0x10                        # movd          0x10(%rdx),%xmm0
  .byte  0x66,0x0f,0x70,0xc0,0x00                        # pshufd        $0x0,%xmm0,%xmm0
  .byte  0x66,0x0f,0x6f,0xcb                             # movdqa        %xmm3,%xmm1
  .byte  0x66,0x0f,0x72,0xd1,0x08                        # psrld         $0x8,%xmm1
  .byte  0x66,0x0f,0xdb,0xc8                             # pand          %xmm0,%xmm1
  .byte  0x66,0x0f,0x6f,0xd3                             # movdqa        %xmm3,%xmm2
  .byte  0x66,0x0f,0x72,0xd2,0x10                        # psrld         $0x10,%xmm2
  .byte  0x66,0x0f,0xdb,0xd0                             # pand          %xmm0,%xmm2
  .byte  0x66,0x0f,0xdb,0xc3                             # pand          %xmm3,%xmm0
  .byte  0x0f,0x5b,0xc0                                  # cvtdq2ps      %xmm0,%xmm0
  .byte  0xf3,0x44,0x0f,0x10,0x42,0x0c                   # movss         0xc(%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x0f,0x5b,0xc9                                  # cvtdq2ps      %xmm1,%xmm1
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x0f,0x5b,0xd2                                  # cvtdq2ps      %xmm2,%xmm2
  .byte  0x41,0x0f,0x59,0xd0                             # mulps         %xmm8,%xmm2
  .byte  0x66,0x0f,0x72,0xd3,0x18                        # psrld         $0x18,%xmm3
  .byte  0x0f,0x5b,0xdb                                  # cvtdq2ps      %xmm3,%xmm3
  .byte  0x41,0x0f,0x59,0xd8                             # mulps         %xmm8,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_store_8888_sse2
_sk_store_8888_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x42,0x08                   # movss         0x8(%rdx),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xc8                             # mulps         %xmm0,%xmm9
  .byte  0x66,0x45,0x0f,0x5b,0xc9                        # cvtps2dq      %xmm9,%xmm9
  .byte  0x45,0x0f,0x28,0xd0                             # movaps        %xmm8,%xmm10
  .byte  0x44,0x0f,0x59,0xd1                             # mulps         %xmm1,%xmm10
  .byte  0x66,0x45,0x0f,0x5b,0xd2                        # cvtps2dq      %xmm10,%xmm10
  .byte  0x66,0x41,0x0f,0x72,0xf2,0x08                   # pslld         $0x8,%xmm10
  .byte  0x66,0x45,0x0f,0xeb,0xd1                        # por           %xmm9,%xmm10
  .byte  0x45,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xca                             # mulps         %xmm2,%xmm9
  .byte  0x66,0x45,0x0f,0x5b,0xc9                        # cvtps2dq      %xmm9,%xmm9
  .byte  0x66,0x41,0x0f,0x72,0xf1,0x10                   # pslld         $0x10,%xmm9
  .byte  0x44,0x0f,0x59,0xc3                             # mulps         %xmm3,%xmm8
  .byte  0x66,0x45,0x0f,0x5b,0xc0                        # cvtps2dq      %xmm8,%xmm8
  .byte  0x66,0x41,0x0f,0x72,0xf0,0x18                   # pslld         $0x18,%xmm8
  .byte  0x66,0x45,0x0f,0xeb,0xc1                        # por           %xmm9,%xmm8
  .byte  0x66,0x45,0x0f,0xeb,0xc2                        # por           %xmm10,%xmm8
  .byte  0xf3,0x44,0x0f,0x7f,0x04,0xb8                   # movdqu        %xmm8,(%rax,%rdi,4)
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_load_f16_sse2
_sk_load_f16_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0xf3,0x0f,0x6f,0x04,0xf8                        # movdqu        (%rax,%rdi,8),%xmm0
  .byte  0xf3,0x0f,0x6f,0x4c,0xf8,0x10                   # movdqu        0x10(%rax,%rdi,8),%xmm1
  .byte  0x66,0x0f,0x6f,0xd0                             # movdqa        %xmm0,%xmm2
  .byte  0x66,0x0f,0x61,0xd1                             # punpcklwd     %xmm1,%xmm2
  .byte  0x66,0x0f,0x69,0xc1                             # punpckhwd     %xmm1,%xmm0
  .byte  0x66,0x44,0x0f,0x6f,0xc2                        # movdqa        %xmm2,%xmm8
  .byte  0x66,0x44,0x0f,0x61,0xc0                        # punpcklwd     %xmm0,%xmm8
  .byte  0x66,0x0f,0x69,0xd0                             # punpckhwd     %xmm0,%xmm2
  .byte  0x66,0x0f,0x6e,0x42,0x64                        # movd          0x64(%rdx),%xmm0
  .byte  0x66,0x0f,0x70,0xd8,0x00                        # pshufd        $0x0,%xmm0,%xmm3
  .byte  0x66,0x0f,0x6f,0xcb                             # movdqa        %xmm3,%xmm1
  .byte  0x66,0x41,0x0f,0x65,0xc8                        # pcmpgtw       %xmm8,%xmm1
  .byte  0x66,0x41,0x0f,0xdf,0xc8                        # pandn         %xmm8,%xmm1
  .byte  0x66,0x0f,0x65,0xda                             # pcmpgtw       %xmm2,%xmm3
  .byte  0x66,0x0f,0xdf,0xda                             # pandn         %xmm2,%xmm3
  .byte  0x66,0x45,0x0f,0xef,0xc0                        # pxor          %xmm8,%xmm8
  .byte  0x66,0x0f,0x6f,0xc1                             # movdqa        %xmm1,%xmm0
  .byte  0x66,0x41,0x0f,0x61,0xc0                        # punpcklwd     %xmm8,%xmm0
  .byte  0x66,0x0f,0x72,0xf0,0x0d                        # pslld         $0xd,%xmm0
  .byte  0x66,0x0f,0x6e,0x52,0x5c                        # movd          0x5c(%rdx),%xmm2
  .byte  0x66,0x44,0x0f,0x70,0xca,0x00                   # pshufd        $0x0,%xmm2,%xmm9
  .byte  0x41,0x0f,0x59,0xc1                             # mulps         %xmm9,%xmm0
  .byte  0x66,0x41,0x0f,0x69,0xc8                        # punpckhwd     %xmm8,%xmm1
  .byte  0x66,0x0f,0x72,0xf1,0x0d                        # pslld         $0xd,%xmm1
  .byte  0x41,0x0f,0x59,0xc9                             # mulps         %xmm9,%xmm1
  .byte  0x66,0x0f,0x6f,0xd3                             # movdqa        %xmm3,%xmm2
  .byte  0x66,0x41,0x0f,0x61,0xd0                        # punpcklwd     %xmm8,%xmm2
  .byte  0x66,0x0f,0x72,0xf2,0x0d                        # pslld         $0xd,%xmm2
  .byte  0x41,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm2
  .byte  0x66,0x41,0x0f,0x69,0xd8                        # punpckhwd     %xmm8,%xmm3
  .byte  0x66,0x0f,0x72,0xf3,0x0d                        # pslld         $0xd,%xmm3
  .byte  0x41,0x0f,0x59,0xd9                             # mulps         %xmm9,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_store_f16_sse2
_sk_store_f16_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x48,0x8b,0x00                                  # mov           (%rax),%rax
  .byte  0x66,0x44,0x0f,0x6e,0x42,0x60                   # movd          0x60(%rdx),%xmm8
  .byte  0x66,0x45,0x0f,0x70,0xc0,0x00                   # pshufd        $0x0,%xmm8,%xmm8
  .byte  0x66,0x45,0x0f,0x6f,0xc8                        # movdqa        %xmm8,%xmm9
  .byte  0x44,0x0f,0x59,0xc8                             # mulps         %xmm0,%xmm9
  .byte  0x66,0x41,0x0f,0x72,0xd1,0x0d                   # psrld         $0xd,%xmm9
  .byte  0x66,0x45,0x0f,0x6f,0xd0                        # movdqa        %xmm8,%xmm10
  .byte  0x44,0x0f,0x59,0xd1                             # mulps         %xmm1,%xmm10
  .byte  0x66,0x41,0x0f,0x72,0xd2,0x0d                   # psrld         $0xd,%xmm10
  .byte  0x66,0x45,0x0f,0x6f,0xd8                        # movdqa        %xmm8,%xmm11
  .byte  0x44,0x0f,0x59,0xda                             # mulps         %xmm2,%xmm11
  .byte  0x66,0x41,0x0f,0x72,0xd3,0x0d                   # psrld         $0xd,%xmm11
  .byte  0x44,0x0f,0x59,0xc3                             # mulps         %xmm3,%xmm8
  .byte  0x66,0x41,0x0f,0x72,0xd0,0x0d                   # psrld         $0xd,%xmm8
  .byte  0x66,0x41,0x0f,0x73,0xfa,0x02                   # pslldq        $0x2,%xmm10
  .byte  0x66,0x45,0x0f,0xeb,0xd1                        # por           %xmm9,%xmm10
  .byte  0x66,0x41,0x0f,0x73,0xf8,0x02                   # pslldq        $0x2,%xmm8
  .byte  0x66,0x45,0x0f,0xeb,0xc3                        # por           %xmm11,%xmm8
  .byte  0x66,0x45,0x0f,0x6f,0xca                        # movdqa        %xmm10,%xmm9
  .byte  0x66,0x45,0x0f,0x62,0xc8                        # punpckldq     %xmm8,%xmm9
  .byte  0xf3,0x44,0x0f,0x7f,0x0c,0xf8                   # movdqu        %xmm9,(%rax,%rdi,8)
  .byte  0x66,0x45,0x0f,0x6a,0xd0                        # punpckhdq     %xmm8,%xmm10
  .byte  0xf3,0x44,0x0f,0x7f,0x54,0xf8,0x10              # movdqu        %xmm10,0x10(%rax,%rdi,8)
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_x_sse2
_sk_clamp_x_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x00                        # movss         (%rax),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x66,0x45,0x0f,0x76,0xc9                        # pcmpeqd       %xmm9,%xmm9
  .byte  0x66,0x45,0x0f,0xfe,0xc8                        # paddd         %xmm8,%xmm9
  .byte  0x41,0x0f,0x5d,0xc1                             # minps         %xmm9,%xmm0
  .byte  0x45,0x0f,0x57,0xc0                             # xorps         %xmm8,%xmm8
  .byte  0x44,0x0f,0x5f,0xc0                             # maxps         %xmm0,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xc0                             # movaps        %xmm8,%xmm0
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_clamp_y_sse2
_sk_clamp_y_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x44,0x0f,0x10,0x00                        # movss         (%rax),%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x66,0x45,0x0f,0x76,0xc9                        # pcmpeqd       %xmm9,%xmm9
  .byte  0x66,0x45,0x0f,0xfe,0xc8                        # paddd         %xmm8,%xmm9
  .byte  0x41,0x0f,0x5d,0xc9                             # minps         %xmm9,%xmm1
  .byte  0x45,0x0f,0x57,0xc0                             # xorps         %xmm8,%xmm8
  .byte  0x44,0x0f,0x5f,0xc1                             # maxps         %xmm1,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xc8                             # movaps        %xmm8,%xmm1
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_matrix_2x3_sse2
_sk_matrix_2x3_sse2:
  .byte  0x44,0x0f,0x28,0xc9                             # movaps        %xmm1,%xmm9
  .byte  0x44,0x0f,0x28,0xc0                             # movaps        %xmm0,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x0f,0x10,0x00                             # movss         (%rax),%xmm0
  .byte  0xf3,0x0f,0x10,0x48,0x04                        # movss         0x4(%rax),%xmm1
  .byte  0x0f,0xc6,0xc0,0x00                             # shufps        $0x0,%xmm0,%xmm0
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x08                   # movss         0x8(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x10                   # movss         0x10(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0x45,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x41,0x0f,0x58,0xc2                             # addps         %xmm10,%xmm0
  .byte  0x0f,0xc6,0xc9,0x00                             # shufps        $0x0,%xmm1,%xmm1
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x0c                   # movss         0xc(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x14                   # movss         0x14(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0x45,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x41,0x0f,0x58,0xca                             # addps         %xmm10,%xmm1
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_matrix_3x4_sse2
_sk_matrix_3x4_sse2:
  .byte  0x44,0x0f,0x28,0xc9                             # movaps        %xmm1,%xmm9
  .byte  0x44,0x0f,0x28,0xc0                             # movaps        %xmm0,%xmm8
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0xf3,0x0f,0x10,0x00                             # movss         (%rax),%xmm0
  .byte  0xf3,0x0f,0x10,0x48,0x04                        # movss         0x4(%rax),%xmm1
  .byte  0x0f,0xc6,0xc0,0x00                             # shufps        $0x0,%xmm0,%xmm0
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x0c                   # movss         0xc(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x18                   # movss         0x18(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0xf3,0x44,0x0f,0x10,0x60,0x24                   # movss         0x24(%rax),%xmm12
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0x44,0x0f,0x59,0xda                             # mulps         %xmm2,%xmm11
  .byte  0x45,0x0f,0x58,0xdc                             # addps         %xmm12,%xmm11
  .byte  0x45,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x41,0x0f,0x59,0xc0                             # mulps         %xmm8,%xmm0
  .byte  0x41,0x0f,0x58,0xc2                             # addps         %xmm10,%xmm0
  .byte  0x0f,0xc6,0xc9,0x00                             # shufps        $0x0,%xmm1,%xmm1
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x10                   # movss         0x10(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x1c                   # movss         0x1c(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0xf3,0x44,0x0f,0x10,0x60,0x28                   # movss         0x28(%rax),%xmm12
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0x44,0x0f,0x59,0xda                             # mulps         %xmm2,%xmm11
  .byte  0x45,0x0f,0x58,0xdc                             # addps         %xmm12,%xmm11
  .byte  0x45,0x0f,0x59,0xd1                             # mulps         %xmm9,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x41,0x0f,0x59,0xc8                             # mulps         %xmm8,%xmm1
  .byte  0x41,0x0f,0x58,0xca                             # addps         %xmm10,%xmm1
  .byte  0xf3,0x44,0x0f,0x10,0x50,0x08                   # movss         0x8(%rax),%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0x00                        # shufps        $0x0,%xmm10,%xmm10
  .byte  0xf3,0x44,0x0f,0x10,0x58,0x14                   # movss         0x14(%rax),%xmm11
  .byte  0x45,0x0f,0xc6,0xdb,0x00                        # shufps        $0x0,%xmm11,%xmm11
  .byte  0xf3,0x44,0x0f,0x10,0x60,0x20                   # movss         0x20(%rax),%xmm12
  .byte  0x45,0x0f,0xc6,0xe4,0x00                        # shufps        $0x0,%xmm12,%xmm12
  .byte  0xf3,0x44,0x0f,0x10,0x68,0x2c                   # movss         0x2c(%rax),%xmm13
  .byte  0x45,0x0f,0xc6,0xed,0x00                        # shufps        $0x0,%xmm13,%xmm13
  .byte  0x44,0x0f,0x59,0xe2                             # mulps         %xmm2,%xmm12
  .byte  0x45,0x0f,0x58,0xe5                             # addps         %xmm13,%xmm12
  .byte  0x45,0x0f,0x59,0xd9                             # mulps         %xmm9,%xmm11
  .byte  0x45,0x0f,0x58,0xdc                             # addps         %xmm12,%xmm11
  .byte  0x45,0x0f,0x59,0xd0                             # mulps         %xmm8,%xmm10
  .byte  0x45,0x0f,0x58,0xd3                             # addps         %xmm11,%xmm10
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xd2                             # movaps        %xmm10,%xmm2
  .byte  0xff,0xe0                                       # jmpq          *%rax

.globl _sk_linear_gradient_2stops_sse2
_sk_linear_gradient_2stops_sse2:
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x44,0x0f,0x10,0x08                             # movups        (%rax),%xmm9
  .byte  0x0f,0x10,0x58,0x10                             # movups        0x10(%rax),%xmm3
  .byte  0x44,0x0f,0x28,0xc3                             # movaps        %xmm3,%xmm8
  .byte  0x45,0x0f,0xc6,0xc0,0x00                        # shufps        $0x0,%xmm8,%xmm8
  .byte  0x41,0x0f,0x28,0xc9                             # movaps        %xmm9,%xmm1
  .byte  0x0f,0xc6,0xc9,0x00                             # shufps        $0x0,%xmm1,%xmm1
  .byte  0x44,0x0f,0x59,0xc0                             # mulps         %xmm0,%xmm8
  .byte  0x44,0x0f,0x58,0xc1                             # addps         %xmm1,%xmm8
  .byte  0x0f,0x28,0xcb                                  # movaps        %xmm3,%xmm1
  .byte  0x0f,0xc6,0xc9,0x55                             # shufps        $0x55,%xmm1,%xmm1
  .byte  0x41,0x0f,0x28,0xd1                             # movaps        %xmm9,%xmm2
  .byte  0x0f,0xc6,0xd2,0x55                             # shufps        $0x55,%xmm2,%xmm2
  .byte  0x0f,0x59,0xc8                                  # mulps         %xmm0,%xmm1
  .byte  0x0f,0x58,0xca                                  # addps         %xmm2,%xmm1
  .byte  0x0f,0x28,0xd3                                  # movaps        %xmm3,%xmm2
  .byte  0x0f,0xc6,0xd2,0xaa                             # shufps        $0xaa,%xmm2,%xmm2
  .byte  0x45,0x0f,0x28,0xd1                             # movaps        %xmm9,%xmm10
  .byte  0x45,0x0f,0xc6,0xd2,0xaa                        # shufps        $0xaa,%xmm10,%xmm10
  .byte  0x0f,0x59,0xd0                                  # mulps         %xmm0,%xmm2
  .byte  0x41,0x0f,0x58,0xd2                             # addps         %xmm10,%xmm2
  .byte  0x0f,0xc6,0xdb,0xff                             # shufps        $0xff,%xmm3,%xmm3
  .byte  0x45,0x0f,0xc6,0xc9,0xff                        # shufps        $0xff,%xmm9,%xmm9
  .byte  0x0f,0x59,0xd8                                  # mulps         %xmm0,%xmm3
  .byte  0x41,0x0f,0x58,0xd9                             # addps         %xmm9,%xmm3
  .byte  0x48,0xad                                       # lods          %ds:(%rsi),%rax
  .byte  0x41,0x0f,0x28,0xc0                             # movaps        %xmm8,%xmm0
  .byte  0xff,0xe0                                       # jmpq          *%rax
