; Copyright 2017 Google Inc.
;
; Use of this source code is governed by a BSD-style license that can be
; found in the LICENSE file.

; This file is generated semi-automatically with this command:
;   $ src/jumper/build_stages.py

_text SEGMENT
ALIGN 16

PUBLIC _sk_start_pipeline_hsw
_sk_start_pipeline_hsw LABEL PROC
  DB  65,87                               ; push          %r15
  DB  65,86                               ; push          %r14
  DB  65,85                               ; push          %r13
  DB  65,84                               ; push          %r12
  DB  86                                  ; push          %rsi
  DB  87                                  ; push          %rdi
  DB  83                                  ; push          %rbx
  DB  72,129,236,160,0,0,0                ; sub           $0xa0,%rsp
  DB  197,120,41,188,36,144,0,0,0         ; vmovaps       %xmm15,0x90(%rsp)
  DB  197,120,41,180,36,128,0,0,0         ; vmovaps       %xmm14,0x80(%rsp)
  DB  197,120,41,108,36,112               ; vmovaps       %xmm13,0x70(%rsp)
  DB  197,120,41,100,36,96                ; vmovaps       %xmm12,0x60(%rsp)
  DB  197,120,41,92,36,80                 ; vmovaps       %xmm11,0x50(%rsp)
  DB  197,120,41,84,36,64                 ; vmovaps       %xmm10,0x40(%rsp)
  DB  197,120,41,76,36,48                 ; vmovaps       %xmm9,0x30(%rsp)
  DB  197,120,41,68,36,32                 ; vmovaps       %xmm8,0x20(%rsp)
  DB  197,248,41,124,36,16                ; vmovaps       %xmm7,0x10(%rsp)
  DB  197,248,41,52,36                    ; vmovaps       %xmm6,(%rsp)
  DB  77,137,207                          ; mov           %r9,%r15
  DB  77,137,198                          ; mov           %r8,%r14
  DB  72,137,203                          ; mov           %rcx,%rbx
  DB  72,137,214                          ; mov           %rdx,%rsi
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  73,137,196                          ; mov           %rax,%r12
  DB  73,137,245                          ; mov           %rsi,%r13
  DB  72,141,67,8                         ; lea           0x8(%rbx),%rax
  DB  76,57,248                           ; cmp           %r15,%rax
  DB  118,5                               ; jbe           75 <_sk_start_pipeline_hsw+0x75>
  DB  72,137,216                          ; mov           %rbx,%rax
  DB  235,60                              ; jmp           b1 <_sk_start_pipeline_hsw+0xb1>
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  197,220,87,228                      ; vxorps        %ymm4,%ymm4,%ymm4
  DB  197,212,87,237                      ; vxorps        %ymm5,%ymm5,%ymm5
  DB  197,204,87,246                      ; vxorps        %ymm6,%ymm6,%ymm6
  DB  197,196,87,255                      ; vxorps        %ymm7,%ymm7,%ymm7
  DB  72,137,223                          ; mov           %rbx,%rdi
  DB  76,137,238                          ; mov           %r13,%rsi
  DB  76,137,242                          ; mov           %r14,%rdx
  DB  65,255,212                          ; callq         *%r12
  DB  72,141,67,8                         ; lea           0x8(%rbx),%rax
  DB  72,131,195,16                       ; add           $0x10,%rbx
  DB  76,57,251                           ; cmp           %r15,%rbx
  DB  72,137,195                          ; mov           %rax,%rbx
  DB  118,196                             ; jbe           75 <_sk_start_pipeline_hsw+0x75>
  DB  197,248,40,52,36                    ; vmovaps       (%rsp),%xmm6
  DB  197,248,40,124,36,16                ; vmovaps       0x10(%rsp),%xmm7
  DB  197,120,40,68,36,32                 ; vmovaps       0x20(%rsp),%xmm8
  DB  197,120,40,76,36,48                 ; vmovaps       0x30(%rsp),%xmm9
  DB  197,120,40,84,36,64                 ; vmovaps       0x40(%rsp),%xmm10
  DB  197,120,40,92,36,80                 ; vmovaps       0x50(%rsp),%xmm11
  DB  197,120,40,100,36,96                ; vmovaps       0x60(%rsp),%xmm12
  DB  197,120,40,108,36,112               ; vmovaps       0x70(%rsp),%xmm13
  DB  197,120,40,180,36,128,0,0,0         ; vmovaps       0x80(%rsp),%xmm14
  DB  197,120,40,188,36,144,0,0,0         ; vmovaps       0x90(%rsp),%xmm15
  DB  72,129,196,160,0,0,0                ; add           $0xa0,%rsp
  DB  91                                  ; pop           %rbx
  DB  95                                  ; pop           %rdi
  DB  94                                  ; pop           %rsi
  DB  65,92                               ; pop           %r12
  DB  65,93                               ; pop           %r13
  DB  65,94                               ; pop           %r14
  DB  65,95                               ; pop           %r15
  DB  197,248,119                         ; vzeroupper
  DB  195                                 ; retq

PUBLIC _sk_just_return_hsw
_sk_just_return_hsw LABEL PROC
  DB  195                                 ; retq

PUBLIC _sk_seed_shader_hsw
_sk_seed_shader_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,249,110,199                     ; vmovd         %edi,%xmm0
  DB  196,226,125,24,192                  ; vbroadcastss  %xmm0,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,13,163,10,0,0        ; vbroadcastss  0xaa3(%rip),%ymm1        # bc4 <_sk_linear_gradient_2stops_hsw+0x4f>
  DB  197,252,88,193                      ; vaddps        %ymm1,%ymm0,%ymm0
  DB  197,252,88,5,83,11,0,0              ; vaddps        0xb53(%rip),%ymm0,%ymm0        # c80 <_sk_linear_gradient_2stops_hsw+0x10b>
  DB  196,226,125,24,16                   ; vbroadcastss  (%rax),%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  197,236,88,201                      ; vaddps        %ymm1,%ymm2,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,21,131,10,0,0        ; vbroadcastss  0xa83(%rip),%ymm2        # bc8 <_sk_linear_gradient_2stops_hsw+0x53>
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  197,220,87,228                      ; vxorps        %ymm4,%ymm4,%ymm4
  DB  197,212,87,237                      ; vxorps        %ymm5,%ymm5,%ymm5
  DB  197,204,87,246                      ; vxorps        %ymm6,%ymm6,%ymm6
  DB  197,196,87,255                      ; vxorps        %ymm7,%ymm7,%ymm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_constant_color_hsw
_sk_constant_color_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,0                    ; vbroadcastss  (%rax),%ymm0
  DB  196,226,125,24,72,4                 ; vbroadcastss  0x4(%rax),%ymm1
  DB  196,226,125,24,80,8                 ; vbroadcastss  0x8(%rax),%ymm2
  DB  196,226,125,24,88,12                ; vbroadcastss  0xc(%rax),%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clear_hsw
_sk_clear_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_plus__hsw
_sk_plus__hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,88,196                      ; vaddps        %ymm4,%ymm0,%ymm0
  DB  197,244,88,205                      ; vaddps        %ymm5,%ymm1,%ymm1
  DB  197,236,88,214                      ; vaddps        %ymm6,%ymm2,%ymm2
  DB  197,228,88,223                      ; vaddps        %ymm7,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_srcover_hsw
_sk_srcover_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,29,10,0,0           ; vbroadcastss  0xa1d(%rip),%ymm8        # bcc <_sk_linear_gradient_2stops_hsw+0x57>
  DB  197,60,92,195                       ; vsubps        %ymm3,%ymm8,%ymm8
  DB  196,194,93,184,192                  ; vfmadd231ps   %ymm8,%ymm4,%ymm0
  DB  196,194,85,184,200                  ; vfmadd231ps   %ymm8,%ymm5,%ymm1
  DB  196,194,77,184,208                  ; vfmadd231ps   %ymm8,%ymm6,%ymm2
  DB  196,194,69,184,216                  ; vfmadd231ps   %ymm8,%ymm7,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_dstover_hsw
_sk_dstover_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,250,9,0,0           ; vbroadcastss  0x9fa(%rip),%ymm8        # bd0 <_sk_linear_gradient_2stops_hsw+0x5b>
  DB  197,60,92,199                       ; vsubps        %ymm7,%ymm8,%ymm8
  DB  196,226,61,168,196                  ; vfmadd213ps   %ymm4,%ymm8,%ymm0
  DB  196,226,61,168,205                  ; vfmadd213ps   %ymm5,%ymm8,%ymm1
  DB  196,226,61,168,214                  ; vfmadd213ps   %ymm6,%ymm8,%ymm2
  DB  196,226,61,168,223                  ; vfmadd213ps   %ymm7,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_0_hsw
_sk_clamp_0_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  196,193,124,95,192                  ; vmaxps        %ymm8,%ymm0,%ymm0
  DB  196,193,116,95,200                  ; vmaxps        %ymm8,%ymm1,%ymm1
  DB  196,193,108,95,208                  ; vmaxps        %ymm8,%ymm2,%ymm2
  DB  196,193,100,95,216                  ; vmaxps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_1_hsw
_sk_clamp_1_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,184,9,0,0           ; vbroadcastss  0x9b8(%rip),%ymm8        # bd4 <_sk_linear_gradient_2stops_hsw+0x5f>
  DB  196,193,124,93,192                  ; vminps        %ymm8,%ymm0,%ymm0
  DB  196,193,116,93,200                  ; vminps        %ymm8,%ymm1,%ymm1
  DB  196,193,108,93,208                  ; vminps        %ymm8,%ymm2,%ymm2
  DB  196,193,100,93,216                  ; vminps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_a_hsw
_sk_clamp_a_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,153,9,0,0           ; vbroadcastss  0x999(%rip),%ymm8        # bd8 <_sk_linear_gradient_2stops_hsw+0x63>
  DB  196,193,100,93,216                  ; vminps        %ymm8,%ymm3,%ymm3
  DB  197,252,93,195                      ; vminps        %ymm3,%ymm0,%ymm0
  DB  197,244,93,203                      ; vminps        %ymm3,%ymm1,%ymm1
  DB  197,236,93,211                      ; vminps        %ymm3,%ymm2,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_set_rgb_hsw
_sk_set_rgb_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,0                    ; vbroadcastss  (%rax),%ymm0
  DB  196,226,125,24,72,4                 ; vbroadcastss  0x4(%rax),%ymm1
  DB  196,226,125,24,80,8                 ; vbroadcastss  0x8(%rax),%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_swap_rb_hsw
_sk_swap_rb_hsw LABEL PROC
  DB  197,124,40,192                      ; vmovaps       %ymm0,%ymm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,40,194                      ; vmovaps       %ymm2,%ymm0
  DB  197,124,41,194                      ; vmovaps       %ymm8,%ymm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_swap_hsw
_sk_swap_hsw LABEL PROC
  DB  197,124,40,195                      ; vmovaps       %ymm3,%ymm8
  DB  197,124,40,202                      ; vmovaps       %ymm2,%ymm9
  DB  197,124,40,209                      ; vmovaps       %ymm1,%ymm10
  DB  197,124,40,216                      ; vmovaps       %ymm0,%ymm11
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,40,196                      ; vmovaps       %ymm4,%ymm0
  DB  197,252,40,205                      ; vmovaps       %ymm5,%ymm1
  DB  197,252,40,214                      ; vmovaps       %ymm6,%ymm2
  DB  197,252,40,223                      ; vmovaps       %ymm7,%ymm3
  DB  197,124,41,220                      ; vmovaps       %ymm11,%ymm4
  DB  197,124,41,213                      ; vmovaps       %ymm10,%ymm5
  DB  197,124,41,206                      ; vmovaps       %ymm9,%ymm6
  DB  197,124,41,199                      ; vmovaps       %ymm8,%ymm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_move_src_dst_hsw
_sk_move_src_dst_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,40,224                      ; vmovaps       %ymm0,%ymm4
  DB  197,252,40,233                      ; vmovaps       %ymm1,%ymm5
  DB  197,252,40,242                      ; vmovaps       %ymm2,%ymm6
  DB  197,252,40,251                      ; vmovaps       %ymm3,%ymm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_move_dst_src_hsw
_sk_move_dst_src_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,40,196                      ; vmovaps       %ymm4,%ymm0
  DB  197,252,40,205                      ; vmovaps       %ymm5,%ymm1
  DB  197,252,40,214                      ; vmovaps       %ymm6,%ymm2
  DB  197,252,40,223                      ; vmovaps       %ymm7,%ymm3
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_premul_hsw
_sk_premul_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,89,195                      ; vmulps        %ymm3,%ymm0,%ymm0
  DB  197,244,89,203                      ; vmulps        %ymm3,%ymm1,%ymm1
  DB  197,236,89,211                      ; vmulps        %ymm3,%ymm2,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_unpremul_hsw
_sk_unpremul_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  196,65,100,194,200,0                ; vcmpeqps      %ymm8,%ymm3,%ymm9
  DB  196,98,125,24,21,213,8,0,0          ; vbroadcastss  0x8d5(%rip),%ymm10        # bdc <_sk_linear_gradient_2stops_hsw+0x67>
  DB  197,44,94,211                       ; vdivps        %ymm3,%ymm10,%ymm10
  DB  196,67,45,74,192,144                ; vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_from_srgb_hsw
_sk_from_srgb_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,180,8,0,0           ; vbroadcastss  0x8b4(%rip),%ymm8        # be0 <_sk_linear_gradient_2stops_hsw+0x6b>
  DB  196,65,124,89,200                   ; vmulps        %ymm8,%ymm0,%ymm9
  DB  197,124,89,208                      ; vmulps        %ymm0,%ymm0,%ymm10
  DB  196,98,125,24,29,166,8,0,0          ; vbroadcastss  0x8a6(%rip),%ymm11        # be4 <_sk_linear_gradient_2stops_hsw+0x6f>
  DB  196,98,125,24,37,161,8,0,0          ; vbroadcastss  0x8a1(%rip),%ymm12        # be8 <_sk_linear_gradient_2stops_hsw+0x73>
  DB  196,65,124,40,236                   ; vmovaps       %ymm12,%ymm13
  DB  196,66,125,168,235                  ; vfmadd213ps   %ymm11,%ymm0,%ymm13
  DB  196,98,125,24,53,146,8,0,0          ; vbroadcastss  0x892(%rip),%ymm14        # bec <_sk_linear_gradient_2stops_hsw+0x77>
  DB  196,66,45,168,238                   ; vfmadd213ps   %ymm14,%ymm10,%ymm13
  DB  196,98,125,24,21,136,8,0,0          ; vbroadcastss  0x888(%rip),%ymm10        # bf0 <_sk_linear_gradient_2stops_hsw+0x7b>
  DB  196,193,124,194,194,1               ; vcmpltps      %ymm10,%ymm0,%ymm0
  DB  196,195,21,74,193,0                 ; vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
  DB  196,65,116,89,200                   ; vmulps        %ymm8,%ymm1,%ymm9
  DB  197,116,89,233                      ; vmulps        %ymm1,%ymm1,%ymm13
  DB  196,65,124,40,252                   ; vmovaps       %ymm12,%ymm15
  DB  196,66,117,168,251                  ; vfmadd213ps   %ymm11,%ymm1,%ymm15
  DB  196,66,21,168,254                   ; vfmadd213ps   %ymm14,%ymm13,%ymm15
  DB  196,193,116,194,202,1               ; vcmpltps      %ymm10,%ymm1,%ymm1
  DB  196,195,5,74,201,16                 ; vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
  DB  196,65,108,89,192                   ; vmulps        %ymm8,%ymm2,%ymm8
  DB  197,108,89,202                      ; vmulps        %ymm2,%ymm2,%ymm9
  DB  196,66,109,168,227                  ; vfmadd213ps   %ymm11,%ymm2,%ymm12
  DB  196,66,53,168,230                   ; vfmadd213ps   %ymm14,%ymm9,%ymm12
  DB  196,193,108,194,210,1               ; vcmpltps      %ymm10,%ymm2,%ymm2
  DB  196,195,29,74,208,32                ; vblendvps     %ymm2,%ymm8,%ymm12,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_to_srgb_hsw
_sk_to_srgb_hsw LABEL PROC
  DB  197,124,82,192                      ; vrsqrtps      %ymm0,%ymm8
  DB  196,65,124,83,200                   ; vrcpps        %ymm8,%ymm9
  DB  196,65,124,82,208                   ; vrsqrtps      %ymm8,%ymm10
  DB  196,98,125,24,5,34,8,0,0            ; vbroadcastss  0x822(%rip),%ymm8        # bf4 <_sk_linear_gradient_2stops_hsw+0x7f>
  DB  196,65,124,89,216                   ; vmulps        %ymm8,%ymm0,%ymm11
  DB  196,98,125,24,37,24,8,0,0           ; vbroadcastss  0x818(%rip),%ymm12        # bf8 <_sk_linear_gradient_2stops_hsw+0x83>
  DB  196,98,125,24,45,19,8,0,0           ; vbroadcastss  0x813(%rip),%ymm13        # bfc <_sk_linear_gradient_2stops_hsw+0x87>
  DB  196,66,21,168,204                   ; vfmadd213ps   %ymm12,%ymm13,%ymm9
  DB  196,98,125,24,53,9,8,0,0            ; vbroadcastss  0x809(%rip),%ymm14        # c00 <_sk_linear_gradient_2stops_hsw+0x8b>
  DB  196,66,13,184,202                   ; vfmadd231ps   %ymm10,%ymm14,%ymm9
  DB  196,98,125,24,21,255,7,0,0          ; vbroadcastss  0x7ff(%rip),%ymm10        # c04 <_sk_linear_gradient_2stops_hsw+0x8f>
  DB  196,65,44,93,201                    ; vminps        %ymm9,%ymm10,%ymm9
  DB  196,98,125,24,61,245,7,0,0          ; vbroadcastss  0x7f5(%rip),%ymm15        # c08 <_sk_linear_gradient_2stops_hsw+0x93>
  DB  196,193,124,194,199,1               ; vcmpltps      %ymm15,%ymm0,%ymm0
  DB  196,195,53,74,195,0                 ; vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  DB  197,124,82,201                      ; vrsqrtps      %ymm1,%ymm9
  DB  196,65,124,83,217                   ; vrcpps        %ymm9,%ymm11
  DB  196,65,124,82,201                   ; vrsqrtps      %ymm9,%ymm9
  DB  196,66,21,168,220                   ; vfmadd213ps   %ymm12,%ymm13,%ymm11
  DB  196,66,13,184,217                   ; vfmadd231ps   %ymm9,%ymm14,%ymm11
  DB  196,65,116,89,200                   ; vmulps        %ymm8,%ymm1,%ymm9
  DB  196,65,44,93,219                    ; vminps        %ymm11,%ymm10,%ymm11
  DB  196,193,116,194,207,1               ; vcmpltps      %ymm15,%ymm1,%ymm1
  DB  196,195,37,74,201,16                ; vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
  DB  197,124,82,202                      ; vrsqrtps      %ymm2,%ymm9
  DB  196,65,124,83,217                   ; vrcpps        %ymm9,%ymm11
  DB  196,66,21,168,220                   ; vfmadd213ps   %ymm12,%ymm13,%ymm11
  DB  196,65,124,82,201                   ; vrsqrtps      %ymm9,%ymm9
  DB  196,66,13,184,217                   ; vfmadd231ps   %ymm9,%ymm14,%ymm11
  DB  196,65,44,93,203                    ; vminps        %ymm11,%ymm10,%ymm9
  DB  196,65,108,89,192                   ; vmulps        %ymm8,%ymm2,%ymm8
  DB  196,193,108,194,215,1               ; vcmpltps      %ymm15,%ymm2,%ymm2
  DB  196,195,53,74,208,32                ; vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_scale_1_float_hsw
_sk_scale_1_float_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  197,188,89,219                      ; vmulps        %ymm3,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_scale_u8_hsw
_sk_scale_u8_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,49,4,56                  ; vpmovzxbd     (%rax,%rdi,1),%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,13,87,7,0,0           ; vbroadcastss  0x757(%rip),%ymm9        # c0c <_sk_linear_gradient_2stops_hsw+0x97>
  DB  196,65,60,89,193                    ; vmulps        %ymm9,%ymm8,%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  197,188,89,219                      ; vmulps        %ymm3,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_1_float_hsw
_sk_lerp_1_float_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  197,252,92,196                      ; vsubps        %ymm4,%ymm0,%ymm0
  DB  196,226,61,168,196                  ; vfmadd213ps   %ymm4,%ymm8,%ymm0
  DB  197,244,92,205                      ; vsubps        %ymm5,%ymm1,%ymm1
  DB  196,226,61,168,205                  ; vfmadd213ps   %ymm5,%ymm8,%ymm1
  DB  197,236,92,214                      ; vsubps        %ymm6,%ymm2,%ymm2
  DB  196,226,61,168,214                  ; vfmadd213ps   %ymm6,%ymm8,%ymm2
  DB  197,228,92,223                      ; vsubps        %ymm7,%ymm3,%ymm3
  DB  196,226,61,168,223                  ; vfmadd213ps   %ymm7,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_u8_hsw
_sk_lerp_u8_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,49,4,56                  ; vpmovzxbd     (%rax,%rdi,1),%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,13,250,6,0,0          ; vbroadcastss  0x6fa(%rip),%ymm9        # c10 <_sk_linear_gradient_2stops_hsw+0x9b>
  DB  196,65,60,89,193                    ; vmulps        %ymm9,%ymm8,%ymm8
  DB  197,252,92,196                      ; vsubps        %ymm4,%ymm0,%ymm0
  DB  196,226,61,168,196                  ; vfmadd213ps   %ymm4,%ymm8,%ymm0
  DB  197,244,92,205                      ; vsubps        %ymm5,%ymm1,%ymm1
  DB  196,226,61,168,205                  ; vfmadd213ps   %ymm5,%ymm8,%ymm1
  DB  197,236,92,214                      ; vsubps        %ymm6,%ymm2,%ymm2
  DB  196,226,61,168,214                  ; vfmadd213ps   %ymm6,%ymm8,%ymm2
  DB  197,228,92,223                      ; vsubps        %ymm7,%ymm3,%ymm3
  DB  196,226,61,168,223                  ; vfmadd213ps   %ymm7,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_565_hsw
_sk_lerp_565_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,226,125,51,28,120               ; vpmovzxwd     (%rax,%rdi,2),%ymm3
  DB  196,98,125,88,5,189,6,0,0           ; vpbroadcastd  0x6bd(%rip),%ymm8        # c14 <_sk_linear_gradient_2stops_hsw+0x9f>
  DB  196,65,101,219,192                  ; vpand         %ymm8,%ymm3,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,13,174,6,0,0          ; vbroadcastss  0x6ae(%rip),%ymm9        # c18 <_sk_linear_gradient_2stops_hsw+0xa3>
  DB  196,65,60,89,193                    ; vmulps        %ymm9,%ymm8,%ymm8
  DB  196,98,125,88,13,164,6,0,0          ; vpbroadcastd  0x6a4(%rip),%ymm9        # c1c <_sk_linear_gradient_2stops_hsw+0xa7>
  DB  196,65,101,219,201                  ; vpand         %ymm9,%ymm3,%ymm9
  DB  196,65,124,91,201                   ; vcvtdq2ps     %ymm9,%ymm9
  DB  196,98,125,24,21,149,6,0,0          ; vbroadcastss  0x695(%rip),%ymm10        # c20 <_sk_linear_gradient_2stops_hsw+0xab>
  DB  196,65,52,89,202                    ; vmulps        %ymm10,%ymm9,%ymm9
  DB  196,98,125,88,21,139,6,0,0          ; vpbroadcastd  0x68b(%rip),%ymm10        # c24 <_sk_linear_gradient_2stops_hsw+0xaf>
  DB  196,193,101,219,218                 ; vpand         %ymm10,%ymm3,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,98,125,24,21,125,6,0,0          ; vbroadcastss  0x67d(%rip),%ymm10        # c28 <_sk_linear_gradient_2stops_hsw+0xb3>
  DB  196,193,100,89,218                  ; vmulps        %ymm10,%ymm3,%ymm3
  DB  197,252,92,196                      ; vsubps        %ymm4,%ymm0,%ymm0
  DB  196,226,61,168,196                  ; vfmadd213ps   %ymm4,%ymm8,%ymm0
  DB  197,244,92,205                      ; vsubps        %ymm5,%ymm1,%ymm1
  DB  196,226,53,168,205                  ; vfmadd213ps   %ymm5,%ymm9,%ymm1
  DB  197,236,92,214                      ; vsubps        %ymm6,%ymm2,%ymm2
  DB  196,226,101,168,214                 ; vfmadd213ps   %ymm6,%ymm3,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,29,86,6,0,0          ; vbroadcastss  0x656(%rip),%ymm3        # c2c <_sk_linear_gradient_2stops_hsw+0xb7>
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_tables_hsw
_sk_load_tables_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,8                            ; mov           (%rax),%rcx
  DB  76,139,64,8                         ; mov           0x8(%rax),%r8
  DB  197,252,16,28,185                   ; vmovups       (%rcx,%rdi,4),%ymm3
  DB  197,252,40,21,178,6,0,0             ; vmovaps       0x6b2(%rip),%ymm2        # ca0 <_sk_linear_gradient_2stops_hsw+0x12b>
  DB  197,228,84,202                      ; vandps        %ymm2,%ymm3,%ymm1
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,124,194,192,0                   ; vcmpeqps      %ymm0,%ymm0,%ymm8
  DB  196,65,124,40,200                   ; vmovaps       %ymm8,%ymm9
  DB  196,194,53,146,4,136                ; vgatherdps    %ymm9,(%r8,%ymm1,4),%ymm0
  DB  72,139,72,16                        ; mov           0x10(%rax),%rcx
  DB  197,245,114,211,8                   ; vpsrld        $0x8,%ymm3,%ymm1
  DB  197,117,219,202                     ; vpand         %ymm2,%ymm1,%ymm9
  DB  196,65,124,40,208                   ; vmovaps       %ymm8,%ymm10
  DB  196,162,45,146,12,137               ; vgatherdps    %ymm10,(%rcx,%ymm9,4),%ymm1
  DB  72,139,64,24                        ; mov           0x18(%rax),%rax
  DB  197,181,114,211,16                  ; vpsrld        $0x10,%ymm3,%ymm9
  DB  197,53,219,202                      ; vpand         %ymm2,%ymm9,%ymm9
  DB  196,162,61,146,20,136               ; vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
  DB  197,229,114,211,24                  ; vpsrld        $0x18,%ymm3,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,98,125,24,5,237,5,0,0           ; vbroadcastss  0x5ed(%rip),%ymm8        # c30 <_sk_linear_gradient_2stops_hsw+0xbb>
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_a8_hsw
_sk_load_a8_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,226,125,49,4,56                 ; vpmovzxbd     (%rax,%rdi,1),%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,13,208,5,0,0         ; vbroadcastss  0x5d0(%rip),%ymm1        # c34 <_sk_linear_gradient_2stops_hsw+0xbf>
  DB  197,252,89,217                      ; vmulps        %ymm1,%ymm0,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_a8_hsw
_sk_store_a8_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,24,5,178,5,0,0           ; vbroadcastss  0x5b2(%rip),%ymm8        # c38 <_sk_linear_gradient_2stops_hsw+0xc3>
  DB  196,65,100,89,192                   ; vmulps        %ymm8,%ymm3,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,67,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm9
  DB  196,66,57,43,193                    ; vpackusdw     %xmm9,%xmm8,%xmm8
  DB  196,65,57,103,192                   ; vpackuswb     %xmm8,%xmm8,%xmm8
  DB  197,121,214,4,56                    ; vmovq         %xmm8,(%rax,%rdi,1)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_565_hsw
_sk_load_565_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,226,125,51,20,120               ; vpmovzxwd     (%rax,%rdi,2),%ymm2
  DB  196,226,125,88,5,127,5,0,0          ; vpbroadcastd  0x57f(%rip),%ymm0        # c3c <_sk_linear_gradient_2stops_hsw+0xc7>
  DB  197,237,219,192                     ; vpand         %ymm0,%ymm2,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,13,114,5,0,0         ; vbroadcastss  0x572(%rip),%ymm1        # c40 <_sk_linear_gradient_2stops_hsw+0xcb>
  DB  197,252,89,193                      ; vmulps        %ymm1,%ymm0,%ymm0
  DB  196,226,125,88,13,105,5,0,0         ; vpbroadcastd  0x569(%rip),%ymm1        # c44 <_sk_linear_gradient_2stops_hsw+0xcf>
  DB  197,237,219,201                     ; vpand         %ymm1,%ymm2,%ymm1
  DB  197,252,91,201                      ; vcvtdq2ps     %ymm1,%ymm1
  DB  196,226,125,24,29,92,5,0,0          ; vbroadcastss  0x55c(%rip),%ymm3        # c48 <_sk_linear_gradient_2stops_hsw+0xd3>
  DB  197,244,89,203                      ; vmulps        %ymm3,%ymm1,%ymm1
  DB  196,226,125,88,29,83,5,0,0          ; vpbroadcastd  0x553(%rip),%ymm3        # c4c <_sk_linear_gradient_2stops_hsw+0xd7>
  DB  197,237,219,211                     ; vpand         %ymm3,%ymm2,%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  196,226,125,24,29,70,5,0,0          ; vbroadcastss  0x546(%rip),%ymm3        # c50 <_sk_linear_gradient_2stops_hsw+0xdb>
  DB  197,236,89,211                      ; vmulps        %ymm3,%ymm2,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,29,59,5,0,0          ; vbroadcastss  0x53b(%rip),%ymm3        # c54 <_sk_linear_gradient_2stops_hsw+0xdf>
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_565_hsw
_sk_store_565_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,24,5,47,5,0,0            ; vbroadcastss  0x52f(%rip),%ymm8        # c58 <_sk_linear_gradient_2stops_hsw+0xe3>
  DB  196,65,124,89,200                   ; vmulps        %ymm8,%ymm0,%ymm9
  DB  196,65,125,91,201                   ; vcvtps2dq     %ymm9,%ymm9
  DB  196,193,53,114,241,11               ; vpslld        $0xb,%ymm9,%ymm9
  DB  196,98,125,24,21,26,5,0,0           ; vbroadcastss  0x51a(%rip),%ymm10        # c5c <_sk_linear_gradient_2stops_hsw+0xe7>
  DB  196,65,116,89,210                   ; vmulps        %ymm10,%ymm1,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,45,114,242,5                ; vpslld        $0x5,%ymm10,%ymm10
  DB  196,65,45,235,201                   ; vpor          %ymm9,%ymm10,%ymm9
  DB  196,65,108,89,192                   ; vmulps        %ymm8,%ymm2,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,65,53,235,192                   ; vpor          %ymm8,%ymm9,%ymm8
  DB  196,67,125,57,193,1                 ; vextracti128  $0x1,%ymm8,%xmm9
  DB  196,66,57,43,193                    ; vpackusdw     %xmm9,%xmm8,%xmm8
  DB  197,122,127,4,120                   ; vmovdqu       %xmm8,(%rax,%rdi,2)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_8888_hsw
_sk_load_8888_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  197,252,16,28,184                   ; vmovups       (%rax,%rdi,4),%ymm3
  DB  197,252,40,21,52,5,0,0              ; vmovaps       0x534(%rip),%ymm2        # cc0 <_sk_linear_gradient_2stops_hsw+0x14b>
  DB  197,228,84,194                      ; vandps        %ymm2,%ymm3,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,98,125,24,5,195,4,0,0           ; vbroadcastss  0x4c3(%rip),%ymm8        # c60 <_sk_linear_gradient_2stops_hsw+0xeb>
  DB  196,193,124,89,192                  ; vmulps        %ymm8,%ymm0,%ymm0
  DB  197,245,114,211,8                   ; vpsrld        $0x8,%ymm3,%ymm1
  DB  197,245,219,202                     ; vpand         %ymm2,%ymm1,%ymm1
  DB  197,252,91,201                      ; vcvtdq2ps     %ymm1,%ymm1
  DB  196,193,116,89,200                  ; vmulps        %ymm8,%ymm1,%ymm1
  DB  197,181,114,211,16                  ; vpsrld        $0x10,%ymm3,%ymm9
  DB  197,181,219,210                     ; vpand         %ymm2,%ymm9,%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  196,193,108,89,208                  ; vmulps        %ymm8,%ymm2,%ymm2
  DB  197,229,114,211,24                  ; vpsrld        $0x18,%ymm3,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_8888_hsw
_sk_store_8888_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,24,5,126,4,0,0           ; vbroadcastss  0x47e(%rip),%ymm8        # c64 <_sk_linear_gradient_2stops_hsw+0xef>
  DB  196,65,124,89,200                   ; vmulps        %ymm8,%ymm0,%ymm9
  DB  196,65,125,91,201                   ; vcvtps2dq     %ymm9,%ymm9
  DB  196,65,116,89,208                   ; vmulps        %ymm8,%ymm1,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,45,114,242,8                ; vpslld        $0x8,%ymm10,%ymm10
  DB  196,65,45,235,201                   ; vpor          %ymm9,%ymm10,%ymm9
  DB  196,65,108,89,208                   ; vmulps        %ymm8,%ymm2,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,45,114,242,16               ; vpslld        $0x10,%ymm10,%ymm10
  DB  196,65,100,89,192                   ; vmulps        %ymm8,%ymm3,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,193,61,114,240,24               ; vpslld        $0x18,%ymm8,%ymm8
  DB  196,65,45,235,192                   ; vpor          %ymm8,%ymm10,%ymm8
  DB  196,65,53,235,192                   ; vpor          %ymm8,%ymm9,%ymm8
  DB  197,126,127,4,184                   ; vmovdqu       %ymm8,(%rax,%rdi,4)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_f16_hsw
_sk_load_f16_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  197,250,111,4,248                   ; vmovdqu       (%rax,%rdi,8),%xmm0
  DB  197,250,111,76,248,16               ; vmovdqu       0x10(%rax,%rdi,8),%xmm1
  DB  197,250,111,84,248,32               ; vmovdqu       0x20(%rax,%rdi,8),%xmm2
  DB  197,250,111,92,248,48               ; vmovdqu       0x30(%rax,%rdi,8),%xmm3
  DB  197,121,97,193                      ; vpunpcklwd    %xmm1,%xmm0,%xmm8
  DB  197,249,105,193                     ; vpunpckhwd    %xmm1,%xmm0,%xmm0
  DB  197,233,97,203                      ; vpunpcklwd    %xmm3,%xmm2,%xmm1
  DB  197,233,105,211                     ; vpunpckhwd    %xmm3,%xmm2,%xmm2
  DB  197,57,97,200                       ; vpunpcklwd    %xmm0,%xmm8,%xmm9
  DB  197,57,105,192                      ; vpunpckhwd    %xmm0,%xmm8,%xmm8
  DB  197,241,97,218                      ; vpunpcklwd    %xmm2,%xmm1,%xmm3
  DB  197,113,105,210                     ; vpunpckhwd    %xmm2,%xmm1,%xmm10
  DB  197,177,108,195                     ; vpunpcklqdq   %xmm3,%xmm9,%xmm0
  DB  196,226,125,19,192                  ; vcvtph2ps     %xmm0,%ymm0
  DB  197,177,109,203                     ; vpunpckhqdq   %xmm3,%xmm9,%xmm1
  DB  196,226,125,19,201                  ; vcvtph2ps     %xmm1,%ymm1
  DB  196,193,57,108,210                  ; vpunpcklqdq   %xmm10,%xmm8,%xmm2
  DB  196,226,125,19,210                  ; vcvtph2ps     %xmm2,%ymm2
  DB  196,193,57,109,218                  ; vpunpckhqdq   %xmm10,%xmm8,%xmm3
  DB  196,226,125,19,219                  ; vcvtph2ps     %xmm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_f16_hsw
_sk_store_f16_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,195,125,29,192,4                ; vcvtps2ph     $0x4,%ymm0,%xmm8
  DB  196,195,125,29,201,4                ; vcvtps2ph     $0x4,%ymm1,%xmm9
  DB  196,195,125,29,210,4                ; vcvtps2ph     $0x4,%ymm2,%xmm10
  DB  196,195,125,29,219,4                ; vcvtps2ph     $0x4,%ymm3,%xmm11
  DB  196,65,57,97,225                    ; vpunpcklwd    %xmm9,%xmm8,%xmm12
  DB  196,65,57,105,193                   ; vpunpckhwd    %xmm9,%xmm8,%xmm8
  DB  196,65,41,97,203                    ; vpunpcklwd    %xmm11,%xmm10,%xmm9
  DB  196,65,41,105,211                   ; vpunpckhwd    %xmm11,%xmm10,%xmm10
  DB  196,65,25,98,217                    ; vpunpckldq    %xmm9,%xmm12,%xmm11
  DB  197,122,127,28,248                  ; vmovdqu       %xmm11,(%rax,%rdi,8)
  DB  196,65,25,106,201                   ; vpunpckhdq    %xmm9,%xmm12,%xmm9
  DB  197,122,127,76,248,16               ; vmovdqu       %xmm9,0x10(%rax,%rdi,8)
  DB  196,65,57,98,202                    ; vpunpckldq    %xmm10,%xmm8,%xmm9
  DB  197,122,127,76,248,32               ; vmovdqu       %xmm9,0x20(%rax,%rdi,8)
  DB  196,65,57,106,194                   ; vpunpckhdq    %xmm10,%xmm8,%xmm8
  DB  197,122,127,68,248,48               ; vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_x_hsw
_sk_clamp_x_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  197,188,95,192                      ; vmaxps        %ymm0,%ymm8,%ymm0
  DB  196,98,125,88,0                     ; vpbroadcastd  (%rax),%ymm8
  DB  196,65,53,118,201                   ; vpcmpeqd      %ymm9,%ymm9,%ymm9
  DB  196,65,61,254,193                   ; vpaddd        %ymm9,%ymm8,%ymm8
  DB  196,193,124,93,192                  ; vminps        %ymm8,%ymm0,%ymm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_y_hsw
_sk_clamp_y_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  197,188,95,201                      ; vmaxps        %ymm1,%ymm8,%ymm1
  DB  196,98,125,88,0                     ; vpbroadcastd  (%rax),%ymm8
  DB  196,65,53,118,201                   ; vpcmpeqd      %ymm9,%ymm9,%ymm9
  DB  196,65,61,254,193                   ; vpaddd        %ymm9,%ymm8,%ymm8
  DB  196,193,116,93,200                  ; vminps        %ymm8,%ymm1,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_repeat_x_hsw
_sk_repeat_x_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,65,124,94,200                   ; vdivps        %ymm8,%ymm0,%ymm9
  DB  196,67,125,8,201,1                  ; vroundps      $0x1,%ymm9,%ymm9
  DB  196,65,52,89,200                    ; vmulps        %ymm8,%ymm9,%ymm9
  DB  196,193,124,92,193                  ; vsubps        %ymm9,%ymm0,%ymm0
  DB  196,65,53,118,201                   ; vpcmpeqd      %ymm9,%ymm9,%ymm9
  DB  196,65,61,254,193                   ; vpaddd        %ymm9,%ymm8,%ymm8
  DB  196,193,124,93,192                  ; vminps        %ymm8,%ymm0,%ymm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_repeat_y_hsw
_sk_repeat_y_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,65,116,94,200                   ; vdivps        %ymm8,%ymm1,%ymm9
  DB  196,67,125,8,201,1                  ; vroundps      $0x1,%ymm9,%ymm9
  DB  196,65,52,89,200                    ; vmulps        %ymm8,%ymm9,%ymm9
  DB  196,193,116,92,201                  ; vsubps        %ymm9,%ymm1,%ymm1
  DB  196,65,53,118,201                   ; vpcmpeqd      %ymm9,%ymm9,%ymm9
  DB  196,65,61,254,193                   ; vpaddd        %ymm9,%ymm8,%ymm8
  DB  196,193,116,93,200                  ; vminps        %ymm8,%ymm1,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_mirror_x_hsw
_sk_mirror_x_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,122,16,0                        ; vmovss        (%rax),%xmm8
  DB  196,66,125,24,200                   ; vbroadcastss  %xmm8,%ymm9
  DB  196,65,124,92,209                   ; vsubps        %ymm9,%ymm0,%ymm10
  DB  196,193,58,88,192                   ; vaddss        %xmm8,%xmm8,%xmm0
  DB  196,226,125,24,192                  ; vbroadcastss  %xmm0,%ymm0
  DB  197,44,94,192                       ; vdivps        %ymm0,%ymm10,%ymm8
  DB  196,67,125,8,192,1                  ; vroundps      $0x1,%ymm8,%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,172,92,192                      ; vsubps        %ymm0,%ymm10,%ymm0
  DB  196,193,124,92,193                  ; vsubps        %ymm9,%ymm0,%ymm0
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  197,60,92,192                       ; vsubps        %ymm0,%ymm8,%ymm8
  DB  197,188,84,192                      ; vandps        %ymm0,%ymm8,%ymm0
  DB  196,65,61,118,192                   ; vpcmpeqd      %ymm8,%ymm8,%ymm8
  DB  196,65,53,254,192                   ; vpaddd        %ymm8,%ymm9,%ymm8
  DB  196,193,124,93,192                  ; vminps        %ymm8,%ymm0,%ymm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_mirror_y_hsw
_sk_mirror_y_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,122,16,0                        ; vmovss        (%rax),%xmm8
  DB  196,66,125,24,200                   ; vbroadcastss  %xmm8,%ymm9
  DB  196,65,116,92,209                   ; vsubps        %ymm9,%ymm1,%ymm10
  DB  196,193,58,88,200                   ; vaddss        %xmm8,%xmm8,%xmm1
  DB  196,226,125,24,201                  ; vbroadcastss  %xmm1,%ymm1
  DB  197,44,94,193                       ; vdivps        %ymm1,%ymm10,%ymm8
  DB  196,67,125,8,192,1                  ; vroundps      $0x1,%ymm8,%ymm8
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,172,92,201                      ; vsubps        %ymm1,%ymm10,%ymm1
  DB  196,193,116,92,201                  ; vsubps        %ymm9,%ymm1,%ymm1
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  197,60,92,193                       ; vsubps        %ymm1,%ymm8,%ymm8
  DB  197,188,84,201                      ; vandps        %ymm1,%ymm8,%ymm1
  DB  196,65,61,118,192                   ; vpcmpeqd      %ymm8,%ymm8,%ymm8
  DB  196,65,53,254,192                   ; vpaddd        %ymm8,%ymm9,%ymm8
  DB  196,193,116,93,200                  ; vminps        %ymm8,%ymm1,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_2x3_hsw
_sk_matrix_2x3_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,8                     ; vbroadcastss  (%rax),%ymm9
  DB  196,98,125,24,80,8                  ; vbroadcastss  0x8(%rax),%ymm10
  DB  196,98,125,24,64,16                 ; vbroadcastss  0x10(%rax),%ymm8
  DB  196,66,117,184,194                  ; vfmadd231ps   %ymm10,%ymm1,%ymm8
  DB  196,66,125,184,193                  ; vfmadd231ps   %ymm9,%ymm0,%ymm8
  DB  196,98,125,24,80,4                  ; vbroadcastss  0x4(%rax),%ymm10
  DB  196,98,125,24,88,12                 ; vbroadcastss  0xc(%rax),%ymm11
  DB  196,98,125,24,72,20                 ; vbroadcastss  0x14(%rax),%ymm9
  DB  196,66,117,184,203                  ; vfmadd231ps   %ymm11,%ymm1,%ymm9
  DB  196,66,125,184,202                  ; vfmadd231ps   %ymm10,%ymm0,%ymm9
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,124,41,192                      ; vmovaps       %ymm8,%ymm0
  DB  197,124,41,201                      ; vmovaps       %ymm9,%ymm1
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_3x4_hsw
_sk_matrix_3x4_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,8                     ; vbroadcastss  (%rax),%ymm9
  DB  196,98,125,24,80,12                 ; vbroadcastss  0xc(%rax),%ymm10
  DB  196,98,125,24,88,24                 ; vbroadcastss  0x18(%rax),%ymm11
  DB  196,98,125,24,64,36                 ; vbroadcastss  0x24(%rax),%ymm8
  DB  196,66,109,184,195                  ; vfmadd231ps   %ymm11,%ymm2,%ymm8
  DB  196,66,117,184,194                  ; vfmadd231ps   %ymm10,%ymm1,%ymm8
  DB  196,66,125,184,193                  ; vfmadd231ps   %ymm9,%ymm0,%ymm8
  DB  196,98,125,24,80,4                  ; vbroadcastss  0x4(%rax),%ymm10
  DB  196,98,125,24,88,16                 ; vbroadcastss  0x10(%rax),%ymm11
  DB  196,98,125,24,96,28                 ; vbroadcastss  0x1c(%rax),%ymm12
  DB  196,98,125,24,72,40                 ; vbroadcastss  0x28(%rax),%ymm9
  DB  196,66,109,184,204                  ; vfmadd231ps   %ymm12,%ymm2,%ymm9
  DB  196,66,117,184,203                  ; vfmadd231ps   %ymm11,%ymm1,%ymm9
  DB  196,66,125,184,202                  ; vfmadd231ps   %ymm10,%ymm0,%ymm9
  DB  196,98,125,24,88,8                  ; vbroadcastss  0x8(%rax),%ymm11
  DB  196,98,125,24,96,20                 ; vbroadcastss  0x14(%rax),%ymm12
  DB  196,98,125,24,104,32                ; vbroadcastss  0x20(%rax),%ymm13
  DB  196,98,125,24,80,44                 ; vbroadcastss  0x2c(%rax),%ymm10
  DB  196,66,109,184,213                  ; vfmadd231ps   %ymm13,%ymm2,%ymm10
  DB  196,66,117,184,212                  ; vfmadd231ps   %ymm12,%ymm1,%ymm10
  DB  196,66,125,184,211                  ; vfmadd231ps   %ymm11,%ymm0,%ymm10
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,124,41,192                      ; vmovaps       %ymm8,%ymm0
  DB  197,124,41,201                      ; vmovaps       %ymm9,%ymm1
  DB  197,124,41,210                      ; vmovaps       %ymm10,%ymm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_perspective_hsw
_sk_matrix_perspective_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,98,125,24,72,4                  ; vbroadcastss  0x4(%rax),%ymm9
  DB  196,98,125,24,80,8                  ; vbroadcastss  0x8(%rax),%ymm10
  DB  196,66,117,184,209                  ; vfmadd231ps   %ymm9,%ymm1,%ymm10
  DB  196,66,125,184,208                  ; vfmadd231ps   %ymm8,%ymm0,%ymm10
  DB  196,98,125,24,64,12                 ; vbroadcastss  0xc(%rax),%ymm8
  DB  196,98,125,24,72,16                 ; vbroadcastss  0x10(%rax),%ymm9
  DB  196,98,125,24,88,20                 ; vbroadcastss  0x14(%rax),%ymm11
  DB  196,66,117,184,217                  ; vfmadd231ps   %ymm9,%ymm1,%ymm11
  DB  196,66,125,184,216                  ; vfmadd231ps   %ymm8,%ymm0,%ymm11
  DB  196,98,125,24,64,24                 ; vbroadcastss  0x18(%rax),%ymm8
  DB  196,98,125,24,72,28                 ; vbroadcastss  0x1c(%rax),%ymm9
  DB  196,98,125,24,96,32                 ; vbroadcastss  0x20(%rax),%ymm12
  DB  196,66,117,184,225                  ; vfmadd231ps   %ymm9,%ymm1,%ymm12
  DB  196,66,125,184,224                  ; vfmadd231ps   %ymm8,%ymm0,%ymm12
  DB  196,193,124,83,204                  ; vrcpps        %ymm12,%ymm1
  DB  197,172,89,193                      ; vmulps        %ymm1,%ymm10,%ymm0
  DB  197,164,89,201                      ; vmulps        %ymm1,%ymm11,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_linear_gradient_2stops_hsw
_sk_linear_gradient_2stops_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,72,16                ; vbroadcastss  0x10(%rax),%ymm1
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,98,125,184,193                  ; vfmadd231ps   %ymm1,%ymm0,%ymm8
  DB  196,226,125,24,80,20                ; vbroadcastss  0x14(%rax),%ymm2
  DB  196,226,125,24,72,4                 ; vbroadcastss  0x4(%rax),%ymm1
  DB  196,226,125,184,202                 ; vfmadd231ps   %ymm2,%ymm0,%ymm1
  DB  196,226,125,24,88,24                ; vbroadcastss  0x18(%rax),%ymm3
  DB  196,226,125,24,80,8                 ; vbroadcastss  0x8(%rax),%ymm2
  DB  196,226,125,184,211                 ; vfmadd231ps   %ymm3,%ymm0,%ymm2
  DB  196,98,125,24,72,28                 ; vbroadcastss  0x1c(%rax),%ymm9
  DB  196,226,125,24,88,12                ; vbroadcastss  0xc(%rax),%ymm3
  DB  196,194,125,184,217                 ; vfmadd231ps   %ymm9,%ymm0,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,124,41,192                      ; vmovaps       %ymm8,%ymm0
  DB  255,224                             ; jmpq          *%rax

ALIGN 4
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  145                                 ; xchg          %eax,%ecx
  DB  131,158,61,92,143,50,63             ; sbbl          $0x3f,0x328f5c3d(%rsi)
  DB  154                                 ; (bad)
  DB  153                                 ; cltd
  DB  153                                 ; cltd
  DB  62,10,215                           ; ds            or  %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  174                                 ; scas          %es:(%rdi),%al
  DB  71,97                               ; rex.RXB       (bad)
  DB  61,41,92,71,65                      ; cmp           $0x41475c29,%eax
  DB  168,87                              ; test          $0x57,%al
  DB  202,189,206                         ; lret          $0xcebd
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  194,135,210                         ; retq          $0xd287
  DB  62,0,0                              ; add           %al,%ds:(%rax)
  DB  128,63,4                            ; cmpb          $0x4,(%rdi)
  DB  231,140                             ; out           %eax,$0x8c
  DB  59,129,128,128,59,129               ; cmp           -0x7ec47f80(%rcx),%eax
  DB  128,128,59,0,248,0,0                ; addb          $0x0,0xf8003b(%rax)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  224,7                               ; loopne        c25 <.literal4+0x61>
  DB  0,0                                 ; add           %al,(%rax)
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  31                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,8                                 ; add           %cl,(%rax)
  DB  33,4,61,0,0,128,63                  ; and           %eax,0x3f800000(,%rdi,1)
  DB  129,128,128,59,129,128,128,59,0,0   ; addl          $0x3b80,-0x7f7ec480(%rax)
  DB  127,67                              ; jg            c7f <_sk_linear_gradient_2stops_hsw+0x10a>
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  224,7                               ; loopne        c4d <.literal4+0x89>
  DB  0,0                                 ; add           %al,(%rax)
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  31                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,8                                 ; add           %cl,(%rax)
  DB  33,4,61,0,0,128,63                  ; and           %eax,0x3f800000(,%rdi,1)
  DB  0,0                                 ; add           %al,(%rax)
  DB  248                                 ; clc
  DB  65,0,0                              ; add           %al,(%r8)
  DB  124,66                              ; jl            ca2 <_sk_linear_gradient_2stops_hsw+0x12d>
  DB  129                                 ; .byte         0x81
  DB  128,128,59,0,0,127,67               ; addb          $0x43,0x7f00003b(%rax)

ALIGN 16
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  64,0,0                              ; add           %al,(%rax)
  DB  64                                  ; rex
  DB  64,0,0                              ; add           %al,(%rax)
  DB  128,64,0,0                          ; addb          $0x0,0x0(%rax)
  DB  160,64,0,0,192,64,0,0,224           ; movabs        0xe0000040c0000040,%al
  DB  64,255,0                            ; rex           incl (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
ALIGN 16

PUBLIC _sk_start_pipeline_avx
_sk_start_pipeline_avx LABEL PROC
  DB  65,87                               ; push          %r15
  DB  65,86                               ; push          %r14
  DB  65,85                               ; push          %r13
  DB  65,84                               ; push          %r12
  DB  86                                  ; push          %rsi
  DB  87                                  ; push          %rdi
  DB  83                                  ; push          %rbx
  DB  72,129,236,160,0,0,0                ; sub           $0xa0,%rsp
  DB  197,120,41,188,36,144,0,0,0         ; vmovaps       %xmm15,0x90(%rsp)
  DB  197,120,41,180,36,128,0,0,0         ; vmovaps       %xmm14,0x80(%rsp)
  DB  197,120,41,108,36,112               ; vmovaps       %xmm13,0x70(%rsp)
  DB  197,120,41,100,36,96                ; vmovaps       %xmm12,0x60(%rsp)
  DB  197,120,41,92,36,80                 ; vmovaps       %xmm11,0x50(%rsp)
  DB  197,120,41,84,36,64                 ; vmovaps       %xmm10,0x40(%rsp)
  DB  197,120,41,76,36,48                 ; vmovaps       %xmm9,0x30(%rsp)
  DB  197,120,41,68,36,32                 ; vmovaps       %xmm8,0x20(%rsp)
  DB  197,248,41,124,36,16                ; vmovaps       %xmm7,0x10(%rsp)
  DB  197,248,41,52,36                    ; vmovaps       %xmm6,(%rsp)
  DB  77,137,207                          ; mov           %r9,%r15
  DB  77,137,198                          ; mov           %r8,%r14
  DB  72,137,203                          ; mov           %rcx,%rbx
  DB  72,137,214                          ; mov           %rdx,%rsi
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  73,137,196                          ; mov           %rax,%r12
  DB  73,137,245                          ; mov           %rsi,%r13
  DB  72,141,67,8                         ; lea           0x8(%rbx),%rax
  DB  76,57,248                           ; cmp           %r15,%rax
  DB  118,5                               ; jbe           75 <_sk_start_pipeline_avx+0x75>
  DB  72,137,216                          ; mov           %rbx,%rax
  DB  235,60                              ; jmp           b1 <_sk_start_pipeline_avx+0xb1>
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  197,220,87,228                      ; vxorps        %ymm4,%ymm4,%ymm4
  DB  197,212,87,237                      ; vxorps        %ymm5,%ymm5,%ymm5
  DB  197,204,87,246                      ; vxorps        %ymm6,%ymm6,%ymm6
  DB  197,196,87,255                      ; vxorps        %ymm7,%ymm7,%ymm7
  DB  72,137,223                          ; mov           %rbx,%rdi
  DB  76,137,238                          ; mov           %r13,%rsi
  DB  76,137,242                          ; mov           %r14,%rdx
  DB  65,255,212                          ; callq         *%r12
  DB  72,141,67,8                         ; lea           0x8(%rbx),%rax
  DB  72,131,195,16                       ; add           $0x10,%rbx
  DB  76,57,251                           ; cmp           %r15,%rbx
  DB  72,137,195                          ; mov           %rax,%rbx
  DB  118,196                             ; jbe           75 <_sk_start_pipeline_avx+0x75>
  DB  197,248,40,52,36                    ; vmovaps       (%rsp),%xmm6
  DB  197,248,40,124,36,16                ; vmovaps       0x10(%rsp),%xmm7
  DB  197,120,40,68,36,32                 ; vmovaps       0x20(%rsp),%xmm8
  DB  197,120,40,76,36,48                 ; vmovaps       0x30(%rsp),%xmm9
  DB  197,120,40,84,36,64                 ; vmovaps       0x40(%rsp),%xmm10
  DB  197,120,40,92,36,80                 ; vmovaps       0x50(%rsp),%xmm11
  DB  197,120,40,100,36,96                ; vmovaps       0x60(%rsp),%xmm12
  DB  197,120,40,108,36,112               ; vmovaps       0x70(%rsp),%xmm13
  DB  197,120,40,180,36,128,0,0,0         ; vmovaps       0x80(%rsp),%xmm14
  DB  197,120,40,188,36,144,0,0,0         ; vmovaps       0x90(%rsp),%xmm15
  DB  72,129,196,160,0,0,0                ; add           $0xa0,%rsp
  DB  91                                  ; pop           %rbx
  DB  95                                  ; pop           %rdi
  DB  94                                  ; pop           %rsi
  DB  65,92                               ; pop           %r12
  DB  65,93                               ; pop           %r13
  DB  65,94                               ; pop           %r14
  DB  65,95                               ; pop           %r15
  DB  197,248,119                         ; vzeroupper
  DB  195                                 ; retq

PUBLIC _sk_just_return_avx
_sk_just_return_avx LABEL PROC
  DB  195                                 ; retq

PUBLIC _sk_seed_shader_avx
_sk_seed_shader_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,249,110,199                     ; vmovd         %edi,%xmm0
  DB  196,227,121,4,192,0                 ; vpermilps     $0x0,%xmm0,%xmm0
  DB  196,227,125,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,13,72,15,0,0         ; vbroadcastss  0xf48(%rip),%ymm1        # 1070 <_sk_linear_gradient_2stops_avx+0x5b>
  DB  197,252,88,193                      ; vaddps        %ymm1,%ymm0,%ymm0
  DB  197,252,88,5,236,15,0,0             ; vaddps        0xfec(%rip),%ymm0,%ymm0        # 1120 <_sk_linear_gradient_2stops_avx+0x10b>
  DB  197,249,110,16                      ; vmovd         (%rax),%xmm2
  DB  196,227,121,4,210,0                 ; vpermilps     $0x0,%xmm2,%xmm2
  DB  196,227,109,24,210,1                ; vinsertf128   $0x1,%xmm2,%ymm2,%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  197,236,88,201                      ; vaddps        %ymm1,%ymm2,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,21,29,15,0,0         ; vbroadcastss  0xf1d(%rip),%ymm2        # 1074 <_sk_linear_gradient_2stops_avx+0x5f>
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  197,220,87,228                      ; vxorps        %ymm4,%ymm4,%ymm4
  DB  197,212,87,237                      ; vxorps        %ymm5,%ymm5,%ymm5
  DB  197,204,87,246                      ; vxorps        %ymm6,%ymm6,%ymm6
  DB  197,196,87,255                      ; vxorps        %ymm7,%ymm7,%ymm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_constant_color_avx
_sk_constant_color_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,0                    ; vbroadcastss  (%rax),%ymm0
  DB  196,226,125,24,72,4                 ; vbroadcastss  0x4(%rax),%ymm1
  DB  196,226,125,24,80,8                 ; vbroadcastss  0x8(%rax),%ymm2
  DB  196,226,125,24,88,12                ; vbroadcastss  0xc(%rax),%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clear_avx
_sk_clear_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_plus__avx
_sk_plus__avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,88,196                      ; vaddps        %ymm4,%ymm0,%ymm0
  DB  197,244,88,205                      ; vaddps        %ymm5,%ymm1,%ymm1
  DB  197,236,88,214                      ; vaddps        %ymm6,%ymm2,%ymm2
  DB  197,228,88,223                      ; vaddps        %ymm7,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_srcover_avx
_sk_srcover_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,183,14,0,0          ; vbroadcastss  0xeb7(%rip),%ymm8        # 1078 <_sk_linear_gradient_2stops_avx+0x63>
  DB  197,60,92,195                       ; vsubps        %ymm3,%ymm8,%ymm8
  DB  197,60,89,204                       ; vmulps        %ymm4,%ymm8,%ymm9
  DB  197,180,88,192                      ; vaddps        %ymm0,%ymm9,%ymm0
  DB  197,60,89,205                       ; vmulps        %ymm5,%ymm8,%ymm9
  DB  197,180,88,201                      ; vaddps        %ymm1,%ymm9,%ymm1
  DB  197,60,89,206                       ; vmulps        %ymm6,%ymm8,%ymm9
  DB  197,180,88,210                      ; vaddps        %ymm2,%ymm9,%ymm2
  DB  197,60,89,199                       ; vmulps        %ymm7,%ymm8,%ymm8
  DB  197,188,88,219                      ; vaddps        %ymm3,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_dstover_avx
_sk_dstover_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,136,14,0,0          ; vbroadcastss  0xe88(%rip),%ymm8        # 107c <_sk_linear_gradient_2stops_avx+0x67>
  DB  197,60,92,199                       ; vsubps        %ymm7,%ymm8,%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,252,88,196                      ; vaddps        %ymm4,%ymm0,%ymm0
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,244,88,205                      ; vaddps        %ymm5,%ymm1,%ymm1
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  197,236,88,214                      ; vaddps        %ymm6,%ymm2,%ymm2
  DB  197,188,89,219                      ; vmulps        %ymm3,%ymm8,%ymm3
  DB  197,228,88,223                      ; vaddps        %ymm7,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_0_avx
_sk_clamp_0_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  196,193,124,95,192                  ; vmaxps        %ymm8,%ymm0,%ymm0
  DB  196,193,116,95,200                  ; vmaxps        %ymm8,%ymm1,%ymm1
  DB  196,193,108,95,208                  ; vmaxps        %ymm8,%ymm2,%ymm2
  DB  196,193,100,95,216                  ; vmaxps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_1_avx
_sk_clamp_1_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,58,14,0,0           ; vbroadcastss  0xe3a(%rip),%ymm8        # 1080 <_sk_linear_gradient_2stops_avx+0x6b>
  DB  196,193,124,93,192                  ; vminps        %ymm8,%ymm0,%ymm0
  DB  196,193,116,93,200                  ; vminps        %ymm8,%ymm1,%ymm1
  DB  196,193,108,93,208                  ; vminps        %ymm8,%ymm2,%ymm2
  DB  196,193,100,93,216                  ; vminps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_a_avx
_sk_clamp_a_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,27,14,0,0           ; vbroadcastss  0xe1b(%rip),%ymm8        # 1084 <_sk_linear_gradient_2stops_avx+0x6f>
  DB  196,193,100,93,216                  ; vminps        %ymm8,%ymm3,%ymm3
  DB  197,252,93,195                      ; vminps        %ymm3,%ymm0,%ymm0
  DB  197,244,93,203                      ; vminps        %ymm3,%ymm1,%ymm1
  DB  197,236,93,211                      ; vminps        %ymm3,%ymm2,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_set_rgb_avx
_sk_set_rgb_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,0                    ; vbroadcastss  (%rax),%ymm0
  DB  196,226,125,24,72,4                 ; vbroadcastss  0x4(%rax),%ymm1
  DB  196,226,125,24,80,8                 ; vbroadcastss  0x8(%rax),%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_swap_rb_avx
_sk_swap_rb_avx LABEL PROC
  DB  197,124,40,192                      ; vmovaps       %ymm0,%ymm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,40,194                      ; vmovaps       %ymm2,%ymm0
  DB  197,124,41,194                      ; vmovaps       %ymm8,%ymm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_swap_avx
_sk_swap_avx LABEL PROC
  DB  197,124,40,195                      ; vmovaps       %ymm3,%ymm8
  DB  197,124,40,202                      ; vmovaps       %ymm2,%ymm9
  DB  197,124,40,209                      ; vmovaps       %ymm1,%ymm10
  DB  197,124,40,216                      ; vmovaps       %ymm0,%ymm11
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,40,196                      ; vmovaps       %ymm4,%ymm0
  DB  197,252,40,205                      ; vmovaps       %ymm5,%ymm1
  DB  197,252,40,214                      ; vmovaps       %ymm6,%ymm2
  DB  197,252,40,223                      ; vmovaps       %ymm7,%ymm3
  DB  197,124,41,220                      ; vmovaps       %ymm11,%ymm4
  DB  197,124,41,213                      ; vmovaps       %ymm10,%ymm5
  DB  197,124,41,206                      ; vmovaps       %ymm9,%ymm6
  DB  197,124,41,199                      ; vmovaps       %ymm8,%ymm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_move_src_dst_avx
_sk_move_src_dst_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,40,224                      ; vmovaps       %ymm0,%ymm4
  DB  197,252,40,233                      ; vmovaps       %ymm1,%ymm5
  DB  197,252,40,242                      ; vmovaps       %ymm2,%ymm6
  DB  197,252,40,251                      ; vmovaps       %ymm3,%ymm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_move_dst_src_avx
_sk_move_dst_src_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,40,196                      ; vmovaps       %ymm4,%ymm0
  DB  197,252,40,205                      ; vmovaps       %ymm5,%ymm1
  DB  197,252,40,214                      ; vmovaps       %ymm6,%ymm2
  DB  197,252,40,223                      ; vmovaps       %ymm7,%ymm3
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_premul_avx
_sk_premul_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,89,195                      ; vmulps        %ymm3,%ymm0,%ymm0
  DB  197,244,89,203                      ; vmulps        %ymm3,%ymm1,%ymm1
  DB  197,236,89,211                      ; vmulps        %ymm3,%ymm2,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_unpremul_avx
_sk_unpremul_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  196,65,100,194,200,0                ; vcmpeqps      %ymm8,%ymm3,%ymm9
  DB  196,98,125,24,21,87,13,0,0          ; vbroadcastss  0xd57(%rip),%ymm10        # 1088 <_sk_linear_gradient_2stops_avx+0x73>
  DB  197,44,94,211                       ; vdivps        %ymm3,%ymm10,%ymm10
  DB  196,67,45,74,192,144                ; vblendvps     %ymm9,%ymm8,%ymm10,%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_from_srgb_avx
_sk_from_srgb_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,5,54,13,0,0           ; vbroadcastss  0xd36(%rip),%ymm8        # 108c <_sk_linear_gradient_2stops_avx+0x77>
  DB  196,65,124,89,200                   ; vmulps        %ymm8,%ymm0,%ymm9
  DB  197,124,89,208                      ; vmulps        %ymm0,%ymm0,%ymm10
  DB  196,98,125,24,29,40,13,0,0          ; vbroadcastss  0xd28(%rip),%ymm11        # 1090 <_sk_linear_gradient_2stops_avx+0x7b>
  DB  196,65,124,89,227                   ; vmulps        %ymm11,%ymm0,%ymm12
  DB  196,98,125,24,45,30,13,0,0          ; vbroadcastss  0xd1e(%rip),%ymm13        # 1094 <_sk_linear_gradient_2stops_avx+0x7f>
  DB  196,65,28,88,229                    ; vaddps        %ymm13,%ymm12,%ymm12
  DB  196,65,44,89,212                    ; vmulps        %ymm12,%ymm10,%ymm10
  DB  196,98,125,24,37,15,13,0,0          ; vbroadcastss  0xd0f(%rip),%ymm12        # 1098 <_sk_linear_gradient_2stops_avx+0x83>
  DB  196,65,44,88,212                    ; vaddps        %ymm12,%ymm10,%ymm10
  DB  196,98,125,24,53,5,13,0,0           ; vbroadcastss  0xd05(%rip),%ymm14        # 109c <_sk_linear_gradient_2stops_avx+0x87>
  DB  196,193,124,194,198,1               ; vcmpltps      %ymm14,%ymm0,%ymm0
  DB  196,195,45,74,193,0                 ; vblendvps     %ymm0,%ymm9,%ymm10,%ymm0
  DB  196,65,116,89,200                   ; vmulps        %ymm8,%ymm1,%ymm9
  DB  197,116,89,209                      ; vmulps        %ymm1,%ymm1,%ymm10
  DB  196,65,116,89,251                   ; vmulps        %ymm11,%ymm1,%ymm15
  DB  196,65,4,88,253                     ; vaddps        %ymm13,%ymm15,%ymm15
  DB  196,65,44,89,215                    ; vmulps        %ymm15,%ymm10,%ymm10
  DB  196,65,44,88,212                    ; vaddps        %ymm12,%ymm10,%ymm10
  DB  196,193,116,194,206,1               ; vcmpltps      %ymm14,%ymm1,%ymm1
  DB  196,195,45,74,201,16                ; vblendvps     %ymm1,%ymm9,%ymm10,%ymm1
  DB  196,65,108,89,192                   ; vmulps        %ymm8,%ymm2,%ymm8
  DB  197,108,89,202                      ; vmulps        %ymm2,%ymm2,%ymm9
  DB  196,65,108,89,211                   ; vmulps        %ymm11,%ymm2,%ymm10
  DB  196,65,44,88,213                    ; vaddps        %ymm13,%ymm10,%ymm10
  DB  196,65,52,89,202                    ; vmulps        %ymm10,%ymm9,%ymm9
  DB  196,65,52,88,204                    ; vaddps        %ymm12,%ymm9,%ymm9
  DB  196,193,108,194,214,1               ; vcmpltps      %ymm14,%ymm2,%ymm2
  DB  196,195,53,74,208,32                ; vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_to_srgb_avx
_sk_to_srgb_avx LABEL PROC
  DB  197,124,82,192                      ; vrsqrtps      %ymm0,%ymm8
  DB  196,65,124,83,200                   ; vrcpps        %ymm8,%ymm9
  DB  196,65,124,82,208                   ; vrsqrtps      %ymm8,%ymm10
  DB  196,98,125,24,5,144,12,0,0          ; vbroadcastss  0xc90(%rip),%ymm8        # 10a0 <_sk_linear_gradient_2stops_avx+0x8b>
  DB  196,65,124,89,216                   ; vmulps        %ymm8,%ymm0,%ymm11
  DB  196,98,125,24,37,134,12,0,0         ; vbroadcastss  0xc86(%rip),%ymm12        # 10a4 <_sk_linear_gradient_2stops_avx+0x8f>
  DB  196,65,52,89,204                    ; vmulps        %ymm12,%ymm9,%ymm9
  DB  196,98,125,24,45,124,12,0,0         ; vbroadcastss  0xc7c(%rip),%ymm13        # 10a8 <_sk_linear_gradient_2stops_avx+0x93>
  DB  196,65,52,88,205                    ; vaddps        %ymm13,%ymm9,%ymm9
  DB  196,98,125,24,53,114,12,0,0         ; vbroadcastss  0xc72(%rip),%ymm14        # 10ac <_sk_linear_gradient_2stops_avx+0x97>
  DB  196,65,44,89,214                    ; vmulps        %ymm14,%ymm10,%ymm10
  DB  196,65,44,88,201                    ; vaddps        %ymm9,%ymm10,%ymm9
  DB  196,98,125,24,21,99,12,0,0          ; vbroadcastss  0xc63(%rip),%ymm10        # 10b0 <_sk_linear_gradient_2stops_avx+0x9b>
  DB  196,65,44,93,201                    ; vminps        %ymm9,%ymm10,%ymm9
  DB  196,98,125,24,61,89,12,0,0          ; vbroadcastss  0xc59(%rip),%ymm15        # 10b4 <_sk_linear_gradient_2stops_avx+0x9f>
  DB  196,193,124,194,199,1               ; vcmpltps      %ymm15,%ymm0,%ymm0
  DB  196,195,53,74,195,0                 ; vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  DB  197,124,82,201                      ; vrsqrtps      %ymm1,%ymm9
  DB  196,65,124,83,217                   ; vrcpps        %ymm9,%ymm11
  DB  196,65,124,82,201                   ; vrsqrtps      %ymm9,%ymm9
  DB  196,65,36,89,220                    ; vmulps        %ymm12,%ymm11,%ymm11
  DB  196,65,36,88,221                    ; vaddps        %ymm13,%ymm11,%ymm11
  DB  196,65,52,89,206                    ; vmulps        %ymm14,%ymm9,%ymm9
  DB  196,65,52,88,203                    ; vaddps        %ymm11,%ymm9,%ymm9
  DB  196,65,116,89,216                   ; vmulps        %ymm8,%ymm1,%ymm11
  DB  196,65,44,93,201                    ; vminps        %ymm9,%ymm10,%ymm9
  DB  196,193,116,194,207,1               ; vcmpltps      %ymm15,%ymm1,%ymm1
  DB  196,195,53,74,203,16                ; vblendvps     %ymm1,%ymm11,%ymm9,%ymm1
  DB  197,124,82,202                      ; vrsqrtps      %ymm2,%ymm9
  DB  196,65,124,83,217                   ; vrcpps        %ymm9,%ymm11
  DB  196,65,36,89,220                    ; vmulps        %ymm12,%ymm11,%ymm11
  DB  196,65,36,88,221                    ; vaddps        %ymm13,%ymm11,%ymm11
  DB  196,65,124,82,201                   ; vrsqrtps      %ymm9,%ymm9
  DB  196,65,52,89,206                    ; vmulps        %ymm14,%ymm9,%ymm9
  DB  196,65,52,88,203                    ; vaddps        %ymm11,%ymm9,%ymm9
  DB  196,65,44,93,201                    ; vminps        %ymm9,%ymm10,%ymm9
  DB  196,65,108,89,192                   ; vmulps        %ymm8,%ymm2,%ymm8
  DB  196,193,108,194,215,1               ; vcmpltps      %ymm15,%ymm2,%ymm2
  DB  196,195,53,74,208,32                ; vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_scale_1_float_avx
_sk_scale_1_float_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  197,188,89,219                      ; vmulps        %ymm3,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_scale_u8_avx
_sk_scale_u8_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,121,49,68,56,4               ; vpmovzxbd     0x4(%rax,%rdi,1),%xmm8
  DB  196,98,121,49,12,56                 ; vpmovzxbd     (%rax,%rdi,1),%xmm9
  DB  196,67,53,24,192,1                  ; vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,13,154,11,0,0         ; vbroadcastss  0xb9a(%rip),%ymm9        # 10b8 <_sk_linear_gradient_2stops_avx+0xa3>
  DB  196,65,60,89,193                    ; vmulps        %ymm9,%ymm8,%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  197,188,89,219                      ; vmulps        %ymm3,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_1_float_avx
_sk_lerp_1_float_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  197,252,92,196                      ; vsubps        %ymm4,%ymm0,%ymm0
  DB  196,193,124,89,192                  ; vmulps        %ymm8,%ymm0,%ymm0
  DB  197,252,88,196                      ; vaddps        %ymm4,%ymm0,%ymm0
  DB  197,244,92,205                      ; vsubps        %ymm5,%ymm1,%ymm1
  DB  196,193,116,89,200                  ; vmulps        %ymm8,%ymm1,%ymm1
  DB  197,244,88,205                      ; vaddps        %ymm5,%ymm1,%ymm1
  DB  197,236,92,214                      ; vsubps        %ymm6,%ymm2,%ymm2
  DB  196,193,108,89,208                  ; vmulps        %ymm8,%ymm2,%ymm2
  DB  197,236,88,214                      ; vaddps        %ymm6,%ymm2,%ymm2
  DB  197,228,92,223                      ; vsubps        %ymm7,%ymm3,%ymm3
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  197,228,88,223                      ; vaddps        %ymm7,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_u8_avx
_sk_lerp_u8_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,121,49,68,56,4               ; vpmovzxbd     0x4(%rax,%rdi,1),%xmm8
  DB  196,98,121,49,12,56                 ; vpmovzxbd     (%rax,%rdi,1),%xmm9
  DB  196,67,53,24,192,1                  ; vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,13,32,11,0,0          ; vbroadcastss  0xb20(%rip),%ymm9        # 10bc <_sk_linear_gradient_2stops_avx+0xa7>
  DB  196,65,60,89,193                    ; vmulps        %ymm9,%ymm8,%ymm8
  DB  197,252,92,196                      ; vsubps        %ymm4,%ymm0,%ymm0
  DB  196,193,124,89,192                  ; vmulps        %ymm8,%ymm0,%ymm0
  DB  197,252,88,196                      ; vaddps        %ymm4,%ymm0,%ymm0
  DB  197,244,92,205                      ; vsubps        %ymm5,%ymm1,%ymm1
  DB  196,193,116,89,200                  ; vmulps        %ymm8,%ymm1,%ymm1
  DB  197,244,88,205                      ; vaddps        %ymm5,%ymm1,%ymm1
  DB  197,236,92,214                      ; vsubps        %ymm6,%ymm2,%ymm2
  DB  196,193,108,89,208                  ; vmulps        %ymm8,%ymm2,%ymm2
  DB  197,236,88,214                      ; vaddps        %ymm6,%ymm2,%ymm2
  DB  197,228,92,223                      ; vsubps        %ymm7,%ymm3,%ymm3
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  197,228,88,223                      ; vaddps        %ymm7,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_565_avx
_sk_lerp_565_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,226,121,51,92,120,8             ; vpmovzxwd     0x8(%rax,%rdi,2),%xmm3
  DB  196,98,121,51,4,120                 ; vpmovzxwd     (%rax,%rdi,2),%xmm8
  DB  196,227,61,24,219,1                 ; vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  DB  196,98,125,24,5,198,10,0,0          ; vbroadcastss  0xac6(%rip),%ymm8        # 10c0 <_sk_linear_gradient_2stops_avx+0xab>
  DB  196,65,100,84,192                   ; vandps        %ymm8,%ymm3,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,13,183,10,0,0         ; vbroadcastss  0xab7(%rip),%ymm9        # 10c4 <_sk_linear_gradient_2stops_avx+0xaf>
  DB  196,65,60,89,193                    ; vmulps        %ymm9,%ymm8,%ymm8
  DB  196,98,125,24,13,173,10,0,0         ; vbroadcastss  0xaad(%rip),%ymm9        # 10c8 <_sk_linear_gradient_2stops_avx+0xb3>
  DB  196,65,100,84,201                   ; vandps        %ymm9,%ymm3,%ymm9
  DB  196,65,124,91,201                   ; vcvtdq2ps     %ymm9,%ymm9
  DB  196,98,125,24,21,158,10,0,0         ; vbroadcastss  0xa9e(%rip),%ymm10        # 10cc <_sk_linear_gradient_2stops_avx+0xb7>
  DB  196,65,52,89,202                    ; vmulps        %ymm10,%ymm9,%ymm9
  DB  196,98,125,24,21,148,10,0,0         ; vbroadcastss  0xa94(%rip),%ymm10        # 10d0 <_sk_linear_gradient_2stops_avx+0xbb>
  DB  196,193,100,84,218                  ; vandps        %ymm10,%ymm3,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,98,125,24,21,134,10,0,0         ; vbroadcastss  0xa86(%rip),%ymm10        # 10d4 <_sk_linear_gradient_2stops_avx+0xbf>
  DB  196,193,100,89,218                  ; vmulps        %ymm10,%ymm3,%ymm3
  DB  197,252,92,196                      ; vsubps        %ymm4,%ymm0,%ymm0
  DB  196,193,124,89,192                  ; vmulps        %ymm8,%ymm0,%ymm0
  DB  197,252,88,196                      ; vaddps        %ymm4,%ymm0,%ymm0
  DB  197,244,92,205                      ; vsubps        %ymm5,%ymm1,%ymm1
  DB  196,193,116,89,201                  ; vmulps        %ymm9,%ymm1,%ymm1
  DB  197,244,88,205                      ; vaddps        %ymm5,%ymm1,%ymm1
  DB  197,236,92,214                      ; vsubps        %ymm6,%ymm2,%ymm2
  DB  197,236,89,211                      ; vmulps        %ymm3,%ymm2,%ymm2
  DB  197,236,88,214                      ; vaddps        %ymm6,%ymm2,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,29,84,10,0,0         ; vbroadcastss  0xa54(%rip),%ymm3        # 10d8 <_sk_linear_gradient_2stops_avx+0xc3>
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_tables_avx
_sk_load_tables_avx LABEL PROC
  DB  65,87                               ; push          %r15
  DB  65,86                               ; push          %r14
  DB  65,84                               ; push          %r12
  DB  83                                  ; push          %rbx
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,0                            ; mov           (%rax),%r8
  DB  72,139,72,8                         ; mov           0x8(%rax),%rcx
  DB  196,65,124,16,4,184                 ; vmovups       (%r8,%rdi,4),%ymm8
  DB  196,67,125,25,194,1                 ; vextractf128  $0x1,%ymm8,%xmm10
  DB  197,120,40,13,150,10,0,0            ; vmovaps       0xa96(%rip),%xmm9        # 1140 <_sk_linear_gradient_2stops_avx+0x12b>
  DB  196,193,40,84,193                   ; vandps        %xmm9,%xmm10,%xmm0
  DB  196,193,56,84,201                   ; vandps        %xmm9,%xmm8,%xmm1
  DB  196,193,249,126,200                 ; vmovq         %xmm1,%r8
  DB  69,137,193                          ; mov           %r8d,%r9d
  DB  196,195,249,22,202,1                ; vpextrq       $0x1,%xmm1,%r10
  DB  69,137,211                          ; mov           %r10d,%r11d
  DB  73,193,234,32                       ; shr           $0x20,%r10
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  196,193,249,126,199                 ; vmovq         %xmm0,%r15
  DB  69,137,254                          ; mov           %r15d,%r14d
  DB  196,227,249,22,195,1                ; vpextrq       $0x1,%xmm0,%rbx
  DB  65,137,220                          ; mov           %ebx,%r12d
  DB  72,193,235,32                       ; shr           $0x20,%rbx
  DB  73,193,239,32                       ; shr           $0x20,%r15
  DB  196,161,122,16,4,177                ; vmovss        (%rcx,%r14,4),%xmm0
  DB  196,163,121,33,4,185,16             ; vinsertps     $0x10,(%rcx,%r15,4),%xmm0,%xmm0
  DB  196,163,121,33,4,161,32             ; vinsertps     $0x20,(%rcx,%r12,4),%xmm0,%xmm0
  DB  196,227,121,33,4,153,48             ; vinsertps     $0x30,(%rcx,%rbx,4),%xmm0,%xmm0
  DB  196,161,122,16,12,137               ; vmovss        (%rcx,%r9,4),%xmm1
  DB  196,163,113,33,12,129,16            ; vinsertps     $0x10,(%rcx,%r8,4),%xmm1,%xmm1
  DB  196,163,113,33,12,153,32            ; vinsertps     $0x20,(%rcx,%r11,4),%xmm1,%xmm1
  DB  196,163,113,33,12,145,48            ; vinsertps     $0x30,(%rcx,%r10,4),%xmm1,%xmm1
  DB  196,227,117,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  DB  72,139,72,16                        ; mov           0x10(%rax),%rcx
  DB  196,193,113,114,208,8               ; vpsrld        $0x8,%xmm8,%xmm1
  DB  196,193,105,114,210,8               ; vpsrld        $0x8,%xmm10,%xmm2
  DB  196,193,105,219,209                 ; vpand         %xmm9,%xmm2,%xmm2
  DB  196,193,113,219,201                 ; vpand         %xmm9,%xmm1,%xmm1
  DB  196,193,249,126,200                 ; vmovq         %xmm1,%r8
  DB  69,137,194                          ; mov           %r8d,%r10d
  DB  196,195,249,22,201,1                ; vpextrq       $0x1,%xmm1,%r9
  DB  69,137,203                          ; mov           %r9d,%r11d
  DB  73,193,233,32                       ; shr           $0x20,%r9
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  196,193,249,126,215                 ; vmovq         %xmm2,%r15
  DB  69,137,254                          ; mov           %r15d,%r14d
  DB  196,227,249,22,211,1                ; vpextrq       $0x1,%xmm2,%rbx
  DB  65,137,220                          ; mov           %ebx,%r12d
  DB  72,193,235,32                       ; shr           $0x20,%rbx
  DB  73,193,239,32                       ; shr           $0x20,%r15
  DB  196,161,122,16,12,177               ; vmovss        (%rcx,%r14,4),%xmm1
  DB  196,163,113,33,12,185,16            ; vinsertps     $0x10,(%rcx,%r15,4),%xmm1,%xmm1
  DB  196,161,122,16,20,161               ; vmovss        (%rcx,%r12,4),%xmm2
  DB  196,227,113,33,202,32               ; vinsertps     $0x20,%xmm2,%xmm1,%xmm1
  DB  197,250,16,20,153                   ; vmovss        (%rcx,%rbx,4),%xmm2
  DB  196,227,113,33,202,48               ; vinsertps     $0x30,%xmm2,%xmm1,%xmm1
  DB  196,161,122,16,20,145               ; vmovss        (%rcx,%r10,4),%xmm2
  DB  196,163,105,33,20,129,16            ; vinsertps     $0x10,(%rcx,%r8,4),%xmm2,%xmm2
  DB  196,161,122,16,28,153               ; vmovss        (%rcx,%r11,4),%xmm3
  DB  196,227,105,33,211,32               ; vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  DB  196,161,122,16,28,137               ; vmovss        (%rcx,%r9,4),%xmm3
  DB  196,227,105,33,211,48               ; vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  DB  196,227,109,24,201,1                ; vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  DB  72,139,64,24                        ; mov           0x18(%rax),%rax
  DB  196,193,105,114,208,16              ; vpsrld        $0x10,%xmm8,%xmm2
  DB  196,193,97,114,210,16               ; vpsrld        $0x10,%xmm10,%xmm3
  DB  196,193,97,219,217                  ; vpand         %xmm9,%xmm3,%xmm3
  DB  196,193,105,219,209                 ; vpand         %xmm9,%xmm2,%xmm2
  DB  196,193,249,126,208                 ; vmovq         %xmm2,%r8
  DB  69,137,193                          ; mov           %r8d,%r9d
  DB  196,195,249,22,214,1                ; vpextrq       $0x1,%xmm2,%r14
  DB  69,137,242                          ; mov           %r14d,%r10d
  DB  73,193,238,32                       ; shr           $0x20,%r14
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  196,225,249,126,219                 ; vmovq         %xmm3,%rbx
  DB  65,137,219                          ; mov           %ebx,%r11d
  DB  196,227,249,22,217,1                ; vpextrq       $0x1,%xmm3,%rcx
  DB  65,137,207                          ; mov           %ecx,%r15d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  72,193,235,32                       ; shr           $0x20,%rbx
  DB  196,161,122,16,20,152               ; vmovss        (%rax,%r11,4),%xmm2
  DB  196,227,105,33,20,152,16            ; vinsertps     $0x10,(%rax,%rbx,4),%xmm2,%xmm2
  DB  196,161,122,16,28,184               ; vmovss        (%rax,%r15,4),%xmm3
  DB  196,227,105,33,211,32               ; vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  DB  197,250,16,28,136                   ; vmovss        (%rax,%rcx,4),%xmm3
  DB  196,99,105,33,203,48                ; vinsertps     $0x30,%xmm3,%xmm2,%xmm9
  DB  196,161,122,16,28,136               ; vmovss        (%rax,%r9,4),%xmm3
  DB  196,163,97,33,28,128,16             ; vinsertps     $0x10,(%rax,%r8,4),%xmm3,%xmm3
  DB  196,161,122,16,20,144               ; vmovss        (%rax,%r10,4),%xmm2
  DB  196,227,97,33,210,32                ; vinsertps     $0x20,%xmm2,%xmm3,%xmm2
  DB  196,161,122,16,28,176               ; vmovss        (%rax,%r14,4),%xmm3
  DB  196,227,105,33,211,48               ; vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  DB  196,195,109,24,209,1                ; vinsertf128   $0x1,%xmm9,%ymm2,%ymm2
  DB  196,193,57,114,208,24               ; vpsrld        $0x18,%xmm8,%xmm8
  DB  196,193,97,114,210,24               ; vpsrld        $0x18,%xmm10,%xmm3
  DB  196,227,61,24,219,1                 ; vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,98,125,24,5,101,8,0,0           ; vbroadcastss  0x865(%rip),%ymm8        # 10dc <_sk_linear_gradient_2stops_avx+0xc7>
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  91                                  ; pop           %rbx
  DB  65,92                               ; pop           %r12
  DB  65,94                               ; pop           %r14
  DB  65,95                               ; pop           %r15
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_a8_avx
_sk_load_a8_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,226,121,49,68,56,4              ; vpmovzxbd     0x4(%rax,%rdi,1),%xmm0
  DB  196,226,121,49,12,56                ; vpmovzxbd     (%rax,%rdi,1),%xmm1
  DB  196,227,117,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,13,52,8,0,0          ; vbroadcastss  0x834(%rip),%ymm1        # 10e0 <_sk_linear_gradient_2stops_avx+0xcb>
  DB  197,252,89,217                      ; vmulps        %ymm1,%ymm0,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_a8_avx
_sk_store_a8_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,24,5,22,8,0,0            ; vbroadcastss  0x816(%rip),%ymm8        # 10e4 <_sk_linear_gradient_2stops_avx+0xcf>
  DB  196,65,100,89,192                   ; vmulps        %ymm8,%ymm3,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,67,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm9
  DB  196,66,57,43,193                    ; vpackusdw     %xmm9,%xmm8,%xmm8
  DB  196,65,57,103,192                   ; vpackuswb     %xmm8,%xmm8,%xmm8
  DB  197,121,214,4,56                    ; vmovq         %xmm8,(%rax,%rdi,1)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_565_avx
_sk_load_565_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,226,121,51,68,120,8             ; vpmovzxwd     0x8(%rax,%rdi,2),%xmm0
  DB  196,226,121,51,12,120               ; vpmovzxwd     (%rax,%rdi,2),%xmm1
  DB  196,227,117,24,208,1                ; vinsertf128   $0x1,%xmm0,%ymm1,%ymm2
  DB  196,226,125,24,5,214,7,0,0          ; vbroadcastss  0x7d6(%rip),%ymm0        # 10e8 <_sk_linear_gradient_2stops_avx+0xd3>
  DB  197,236,84,192                      ; vandps        %ymm0,%ymm2,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,13,201,7,0,0         ; vbroadcastss  0x7c9(%rip),%ymm1        # 10ec <_sk_linear_gradient_2stops_avx+0xd7>
  DB  197,252,89,193                      ; vmulps        %ymm1,%ymm0,%ymm0
  DB  196,226,125,24,13,192,7,0,0         ; vbroadcastss  0x7c0(%rip),%ymm1        # 10f0 <_sk_linear_gradient_2stops_avx+0xdb>
  DB  197,236,84,201                      ; vandps        %ymm1,%ymm2,%ymm1
  DB  197,252,91,201                      ; vcvtdq2ps     %ymm1,%ymm1
  DB  196,226,125,24,29,179,7,0,0         ; vbroadcastss  0x7b3(%rip),%ymm3        # 10f4 <_sk_linear_gradient_2stops_avx+0xdf>
  DB  197,244,89,203                      ; vmulps        %ymm3,%ymm1,%ymm1
  DB  196,226,125,24,29,170,7,0,0         ; vbroadcastss  0x7aa(%rip),%ymm3        # 10f8 <_sk_linear_gradient_2stops_avx+0xe3>
  DB  197,236,84,211                      ; vandps        %ymm3,%ymm2,%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  196,226,125,24,29,157,7,0,0         ; vbroadcastss  0x79d(%rip),%ymm3        # 10fc <_sk_linear_gradient_2stops_avx+0xe7>
  DB  197,236,89,211                      ; vmulps        %ymm3,%ymm2,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,29,146,7,0,0         ; vbroadcastss  0x792(%rip),%ymm3        # 1100 <_sk_linear_gradient_2stops_avx+0xeb>
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_565_avx
_sk_store_565_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,24,5,134,7,0,0           ; vbroadcastss  0x786(%rip),%ymm8        # 1104 <_sk_linear_gradient_2stops_avx+0xef>
  DB  196,65,124,89,200                   ; vmulps        %ymm8,%ymm0,%ymm9
  DB  196,65,125,91,201                   ; vcvtps2dq     %ymm9,%ymm9
  DB  196,193,41,114,241,11               ; vpslld        $0xb,%xmm9,%xmm10
  DB  196,67,125,25,201,1                 ; vextractf128  $0x1,%ymm9,%xmm9
  DB  196,193,49,114,241,11               ; vpslld        $0xb,%xmm9,%xmm9
  DB  196,67,45,24,201,1                  ; vinsertf128   $0x1,%xmm9,%ymm10,%ymm9
  DB  196,98,125,24,21,95,7,0,0           ; vbroadcastss  0x75f(%rip),%ymm10        # 1108 <_sk_linear_gradient_2stops_avx+0xf3>
  DB  196,65,116,89,210                   ; vmulps        %ymm10,%ymm1,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,33,114,242,5                ; vpslld        $0x5,%xmm10,%xmm11
  DB  196,67,125,25,210,1                 ; vextractf128  $0x1,%ymm10,%xmm10
  DB  196,193,41,114,242,5                ; vpslld        $0x5,%xmm10,%xmm10
  DB  196,67,37,24,210,1                  ; vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  DB  196,65,45,86,201                    ; vorpd         %ymm9,%ymm10,%ymm9
  DB  196,65,108,89,192                   ; vmulps        %ymm8,%ymm2,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,65,53,86,192                    ; vorpd         %ymm8,%ymm9,%ymm8
  DB  196,67,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm9
  DB  196,66,57,43,193                    ; vpackusdw     %xmm9,%xmm8,%xmm8
  DB  197,122,127,4,120                   ; vmovdqu       %xmm8,(%rax,%rdi,2)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_8888_avx
_sk_load_8888_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  197,252,16,28,184                   ; vmovups       (%rax,%rdi,4),%ymm3
  DB  196,195,125,25,216,1                ; vextractf128  $0x1,%ymm3,%xmm8
  DB  197,120,40,29,69,7,0,0              ; vmovaps       0x745(%rip),%xmm11        # 1150 <_sk_linear_gradient_2stops_avx+0x13b>
  DB  196,193,56,84,195                   ; vandps        %xmm11,%xmm8,%xmm0
  DB  196,193,96,84,203                   ; vandps        %xmm11,%xmm3,%xmm1
  DB  196,227,117,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,98,125,24,13,228,6,0,0          ; vbroadcastss  0x6e4(%rip),%ymm9        # 110c <_sk_linear_gradient_2stops_avx+0xf7>
  DB  196,193,124,89,193                  ; vmulps        %ymm9,%ymm0,%ymm0
  DB  197,169,114,211,8                   ; vpsrld        $0x8,%xmm3,%xmm10
  DB  196,193,113,114,208,8               ; vpsrld        $0x8,%xmm8,%xmm1
  DB  196,193,113,219,203                 ; vpand         %xmm11,%xmm1,%xmm1
  DB  196,65,41,219,211                   ; vpand         %xmm11,%xmm10,%xmm10
  DB  196,227,45,24,201,1                 ; vinsertf128   $0x1,%xmm1,%ymm10,%ymm1
  DB  197,252,91,201                      ; vcvtdq2ps     %ymm1,%ymm1
  DB  196,193,116,89,201                  ; vmulps        %ymm9,%ymm1,%ymm1
  DB  197,169,114,211,16                  ; vpsrld        $0x10,%xmm3,%xmm10
  DB  196,193,105,114,208,16              ; vpsrld        $0x10,%xmm8,%xmm2
  DB  196,193,105,219,211                 ; vpand         %xmm11,%xmm2,%xmm2
  DB  196,65,41,219,211                   ; vpand         %xmm11,%xmm10,%xmm10
  DB  196,227,45,24,210,1                 ; vinsertf128   $0x1,%xmm2,%ymm10,%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  196,193,108,89,209                  ; vmulps        %ymm9,%ymm2,%ymm2
  DB  197,169,114,211,24                  ; vpsrld        $0x18,%xmm3,%xmm10
  DB  196,193,97,114,208,24               ; vpsrld        $0x18,%xmm8,%xmm3
  DB  196,227,45,24,219,1                 ; vinsertf128   $0x1,%xmm3,%ymm10,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,193,100,89,217                  ; vmulps        %ymm9,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_8888_avx
_sk_store_8888_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,24,5,111,6,0,0           ; vbroadcastss  0x66f(%rip),%ymm8        # 1110 <_sk_linear_gradient_2stops_avx+0xfb>
  DB  196,65,124,89,200                   ; vmulps        %ymm8,%ymm0,%ymm9
  DB  196,65,125,91,201                   ; vcvtps2dq     %ymm9,%ymm9
  DB  196,65,116,89,208                   ; vmulps        %ymm8,%ymm1,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,33,114,242,8                ; vpslld        $0x8,%xmm10,%xmm11
  DB  196,67,125,25,210,1                 ; vextractf128  $0x1,%ymm10,%xmm10
  DB  196,193,41,114,242,8                ; vpslld        $0x8,%xmm10,%xmm10
  DB  196,67,37,24,210,1                  ; vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  DB  196,65,45,86,201                    ; vorpd         %ymm9,%ymm10,%ymm9
  DB  196,65,108,89,208                   ; vmulps        %ymm8,%ymm2,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,33,114,242,16               ; vpslld        $0x10,%xmm10,%xmm11
  DB  196,67,125,25,210,1                 ; vextractf128  $0x1,%ymm10,%xmm10
  DB  196,193,41,114,242,16               ; vpslld        $0x10,%xmm10,%xmm10
  DB  196,67,37,24,210,1                  ; vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  DB  196,65,53,86,202                    ; vorpd         %ymm10,%ymm9,%ymm9
  DB  196,65,100,89,192                   ; vmulps        %ymm8,%ymm3,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,193,41,114,240,24               ; vpslld        $0x18,%xmm8,%xmm10
  DB  196,67,125,25,192,1                 ; vextractf128  $0x1,%ymm8,%xmm8
  DB  196,193,57,114,240,24               ; vpslld        $0x18,%xmm8,%xmm8
  DB  196,67,45,24,192,1                  ; vinsertf128   $0x1,%xmm8,%ymm10,%ymm8
  DB  196,65,53,86,192                    ; vorpd         %ymm8,%ymm9,%ymm8
  DB  197,125,17,4,184                    ; vmovupd       %ymm8,(%rax,%rdi,4)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_f16_avx
_sk_load_f16_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  197,250,111,4,248                   ; vmovdqu       (%rax,%rdi,8),%xmm0
  DB  197,250,111,76,248,16               ; vmovdqu       0x10(%rax,%rdi,8),%xmm1
  DB  197,250,111,84,248,32               ; vmovdqu       0x20(%rax,%rdi,8),%xmm2
  DB  197,250,111,92,248,48               ; vmovdqu       0x30(%rax,%rdi,8),%xmm3
  DB  197,121,97,193                      ; vpunpcklwd    %xmm1,%xmm0,%xmm8
  DB  197,249,105,193                     ; vpunpckhwd    %xmm1,%xmm0,%xmm0
  DB  197,233,97,203                      ; vpunpcklwd    %xmm3,%xmm2,%xmm1
  DB  197,233,105,211                     ; vpunpckhwd    %xmm3,%xmm2,%xmm2
  DB  197,185,97,216                      ; vpunpcklwd    %xmm0,%xmm8,%xmm3
  DB  197,185,105,192                     ; vpunpckhwd    %xmm0,%xmm8,%xmm0
  DB  197,113,97,194                      ; vpunpcklwd    %xmm2,%xmm1,%xmm8
  DB  197,241,105,202                     ; vpunpckhwd    %xmm2,%xmm1,%xmm1
  DB  197,121,111,13,243,5,0,0            ; vmovdqa       0x5f3(%rip),%xmm9        # 1160 <_sk_linear_gradient_2stops_avx+0x14b>
  DB  197,177,101,211                     ; vpcmpgtw      %xmm3,%xmm9,%xmm2
  DB  197,233,223,211                     ; vpandn        %xmm3,%xmm2,%xmm2
  DB  197,177,101,216                     ; vpcmpgtw      %xmm0,%xmm9,%xmm3
  DB  197,97,223,208                      ; vpandn        %xmm0,%xmm3,%xmm10
  DB  196,193,49,101,216                  ; vpcmpgtw      %xmm8,%xmm9,%xmm3
  DB  196,193,97,223,216                  ; vpandn        %xmm8,%xmm3,%xmm3
  DB  197,177,101,193                     ; vpcmpgtw      %xmm1,%xmm9,%xmm0
  DB  197,249,223,193                     ; vpandn        %xmm1,%xmm0,%xmm0
  DB  196,98,121,51,202                   ; vpmovzxwd     %xmm2,%xmm9
  DB  196,98,121,51,195                   ; vpmovzxwd     %xmm3,%xmm8
  DB  197,241,239,201                     ; vpxor         %xmm1,%xmm1,%xmm1
  DB  197,233,105,209                     ; vpunpckhwd    %xmm1,%xmm2,%xmm2
  DB  197,225,105,217                     ; vpunpckhwd    %xmm1,%xmm3,%xmm3
  DB  196,66,121,51,218                   ; vpmovzxwd     %xmm10,%xmm11
  DB  196,98,121,51,224                   ; vpmovzxwd     %xmm0,%xmm12
  DB  197,41,105,209                      ; vpunpckhwd    %xmm1,%xmm10,%xmm10
  DB  197,121,105,233                     ; vpunpckhwd    %xmm1,%xmm0,%xmm13
  DB  196,193,121,114,241,13              ; vpslld        $0xd,%xmm9,%xmm0
  DB  196,193,113,114,240,13              ; vpslld        $0xd,%xmm8,%xmm1
  DB  196,227,125,24,193,1                ; vinsertf128   $0x1,%xmm1,%ymm0,%ymm0
  DB  196,98,125,24,5,66,5,0,0            ; vbroadcastss  0x542(%rip),%ymm8        # 1114 <_sk_linear_gradient_2stops_avx+0xff>
  DB  196,193,124,89,192                  ; vmulps        %ymm8,%ymm0,%ymm0
  DB  197,241,114,242,13                  ; vpslld        $0xd,%xmm2,%xmm1
  DB  197,233,114,243,13                  ; vpslld        $0xd,%xmm3,%xmm2
  DB  196,227,117,24,202,1                ; vinsertf128   $0x1,%xmm2,%ymm1,%ymm1
  DB  196,193,116,89,200                  ; vmulps        %ymm8,%ymm1,%ymm1
  DB  196,193,105,114,243,13              ; vpslld        $0xd,%xmm11,%xmm2
  DB  196,193,97,114,244,13               ; vpslld        $0xd,%xmm12,%xmm3
  DB  196,227,109,24,211,1                ; vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  DB  196,193,108,89,208                  ; vmulps        %ymm8,%ymm2,%ymm2
  DB  196,193,49,114,242,13               ; vpslld        $0xd,%xmm10,%xmm9
  DB  196,193,97,114,245,13               ; vpslld        $0xd,%xmm13,%xmm3
  DB  196,227,53,24,219,1                 ; vinsertf128   $0x1,%xmm3,%ymm9,%ymm3
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_f16_avx
_sk_store_f16_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,24,5,236,4,0,0           ; vbroadcastss  0x4ec(%rip),%ymm8        # 1118 <_sk_linear_gradient_2stops_avx+0x103>
  DB  196,65,124,89,200                   ; vmulps        %ymm8,%ymm0,%ymm9
  DB  196,67,125,25,202,1                 ; vextractf128  $0x1,%ymm9,%xmm10
  DB  196,193,41,114,210,13               ; vpsrld        $0xd,%xmm10,%xmm10
  DB  196,193,49,114,209,13               ; vpsrld        $0xd,%xmm9,%xmm9
  DB  196,65,116,89,216                   ; vmulps        %ymm8,%ymm1,%ymm11
  DB  196,67,125,25,220,1                 ; vextractf128  $0x1,%ymm11,%xmm12
  DB  196,193,25,114,212,13               ; vpsrld        $0xd,%xmm12,%xmm12
  DB  196,193,33,114,211,13               ; vpsrld        $0xd,%xmm11,%xmm11
  DB  196,65,108,89,232                   ; vmulps        %ymm8,%ymm2,%ymm13
  DB  196,67,125,25,238,1                 ; vextractf128  $0x1,%ymm13,%xmm14
  DB  196,193,9,114,214,13                ; vpsrld        $0xd,%xmm14,%xmm14
  DB  196,193,17,114,213,13               ; vpsrld        $0xd,%xmm13,%xmm13
  DB  196,65,100,89,192                   ; vmulps        %ymm8,%ymm3,%ymm8
  DB  196,67,125,25,199,1                 ; vextractf128  $0x1,%ymm8,%xmm15
  DB  196,193,1,114,215,13                ; vpsrld        $0xd,%xmm15,%xmm15
  DB  196,193,57,114,208,13               ; vpsrld        $0xd,%xmm8,%xmm8
  DB  196,193,33,115,251,2                ; vpslldq       $0x2,%xmm11,%xmm11
  DB  196,65,33,235,201                   ; vpor          %xmm9,%xmm11,%xmm9
  DB  196,193,33,115,252,2                ; vpslldq       $0x2,%xmm12,%xmm11
  DB  196,65,33,235,210                   ; vpor          %xmm10,%xmm11,%xmm10
  DB  196,193,57,115,248,2                ; vpslldq       $0x2,%xmm8,%xmm8
  DB  196,65,57,235,197                   ; vpor          %xmm13,%xmm8,%xmm8
  DB  196,193,33,115,255,2                ; vpslldq       $0x2,%xmm15,%xmm11
  DB  196,65,33,235,222                   ; vpor          %xmm14,%xmm11,%xmm11
  DB  196,65,49,98,224                    ; vpunpckldq    %xmm8,%xmm9,%xmm12
  DB  197,122,127,36,248                  ; vmovdqu       %xmm12,(%rax,%rdi,8)
  DB  196,65,49,106,192                   ; vpunpckhdq    %xmm8,%xmm9,%xmm8
  DB  197,122,127,68,248,16               ; vmovdqu       %xmm8,0x10(%rax,%rdi,8)
  DB  196,65,41,98,195                    ; vpunpckldq    %xmm11,%xmm10,%xmm8
  DB  197,122,127,68,248,32               ; vmovdqu       %xmm8,0x20(%rax,%rdi,8)
  DB  196,65,41,106,195                   ; vpunpckhdq    %xmm11,%xmm10,%xmm8
  DB  197,122,127,68,248,48               ; vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_x_avx
_sk_clamp_x_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  197,60,95,200                       ; vmaxps        %ymm0,%ymm8,%ymm9
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,99,125,25,192,1                 ; vextractf128  $0x1,%ymm8,%xmm0
  DB  196,65,41,118,210                   ; vpcmpeqd      %xmm10,%xmm10,%xmm10
  DB  196,193,121,254,194                 ; vpaddd        %xmm10,%xmm0,%xmm0
  DB  196,65,57,254,194                   ; vpaddd        %xmm10,%xmm8,%xmm8
  DB  196,227,61,24,192,1                 ; vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  DB  197,180,93,192                      ; vminps        %ymm0,%ymm9,%ymm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_y_avx
_sk_clamp_y_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  197,60,95,201                       ; vmaxps        %ymm1,%ymm8,%ymm9
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,99,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm1
  DB  196,65,41,118,210                   ; vpcmpeqd      %xmm10,%xmm10,%xmm10
  DB  196,193,113,254,202                 ; vpaddd        %xmm10,%xmm1,%xmm1
  DB  196,65,57,254,194                   ; vpaddd        %xmm10,%xmm8,%xmm8
  DB  196,227,61,24,201,1                 ; vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  DB  197,180,93,201                      ; vminps        %ymm1,%ymm9,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_repeat_x_avx
_sk_repeat_x_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,65,124,94,200                   ; vdivps        %ymm8,%ymm0,%ymm9
  DB  196,67,125,8,201,1                  ; vroundps      $0x1,%ymm9,%ymm9
  DB  196,65,52,89,200                    ; vmulps        %ymm8,%ymm9,%ymm9
  DB  196,65,124,92,201                   ; vsubps        %ymm9,%ymm0,%ymm9
  DB  196,99,125,25,192,1                 ; vextractf128  $0x1,%ymm8,%xmm0
  DB  196,65,41,118,210                   ; vpcmpeqd      %xmm10,%xmm10,%xmm10
  DB  196,193,121,254,194                 ; vpaddd        %xmm10,%xmm0,%xmm0
  DB  196,65,57,254,194                   ; vpaddd        %xmm10,%xmm8,%xmm8
  DB  196,227,61,24,192,1                 ; vinsertf128   $0x1,%xmm0,%ymm8,%ymm0
  DB  197,180,93,192                      ; vminps        %ymm0,%ymm9,%ymm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_repeat_y_avx
_sk_repeat_y_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,65,116,94,200                   ; vdivps        %ymm8,%ymm1,%ymm9
  DB  196,67,125,8,201,1                  ; vroundps      $0x1,%ymm9,%ymm9
  DB  196,65,52,89,200                    ; vmulps        %ymm8,%ymm9,%ymm9
  DB  196,65,116,92,201                   ; vsubps        %ymm9,%ymm1,%ymm9
  DB  196,99,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm1
  DB  196,65,41,118,210                   ; vpcmpeqd      %xmm10,%xmm10,%xmm10
  DB  196,193,113,254,202                 ; vpaddd        %xmm10,%xmm1,%xmm1
  DB  196,65,57,254,194                   ; vpaddd        %xmm10,%xmm8,%xmm8
  DB  196,227,61,24,201,1                 ; vinsertf128   $0x1,%xmm1,%ymm8,%ymm1
  DB  197,180,93,201                      ; vminps        %ymm1,%ymm9,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_mirror_x_avx
_sk_mirror_x_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,122,16,0                        ; vmovss        (%rax),%xmm8
  DB  196,65,121,112,200,0                ; vpshufd       $0x0,%xmm8,%xmm9
  DB  196,67,53,24,201,1                  ; vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  DB  196,65,124,92,209                   ; vsubps        %ymm9,%ymm0,%ymm10
  DB  196,193,58,88,192                   ; vaddss        %xmm8,%xmm8,%xmm0
  DB  196,227,121,4,192,0                 ; vpermilps     $0x0,%xmm0,%xmm0
  DB  196,227,125,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  DB  197,44,94,192                       ; vdivps        %ymm0,%ymm10,%ymm8
  DB  196,67,125,8,192,1                  ; vroundps      $0x1,%ymm8,%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,172,92,192                      ; vsubps        %ymm0,%ymm10,%ymm0
  DB  196,193,124,92,193                  ; vsubps        %ymm9,%ymm0,%ymm0
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  197,60,92,192                       ; vsubps        %ymm0,%ymm8,%ymm8
  DB  197,60,84,192                       ; vandps        %ymm0,%ymm8,%ymm8
  DB  196,99,125,25,200,1                 ; vextractf128  $0x1,%ymm9,%xmm0
  DB  196,65,41,118,210                   ; vpcmpeqd      %xmm10,%xmm10,%xmm10
  DB  196,193,121,254,194                 ; vpaddd        %xmm10,%xmm0,%xmm0
  DB  196,65,49,254,202                   ; vpaddd        %xmm10,%xmm9,%xmm9
  DB  196,227,53,24,192,1                 ; vinsertf128   $0x1,%xmm0,%ymm9,%ymm0
  DB  197,188,93,192                      ; vminps        %ymm0,%ymm8,%ymm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_mirror_y_avx
_sk_mirror_y_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,122,16,0                        ; vmovss        (%rax),%xmm8
  DB  196,65,121,112,200,0                ; vpshufd       $0x0,%xmm8,%xmm9
  DB  196,67,53,24,201,1                  ; vinsertf128   $0x1,%xmm9,%ymm9,%ymm9
  DB  196,65,116,92,209                   ; vsubps        %ymm9,%ymm1,%ymm10
  DB  196,193,58,88,200                   ; vaddss        %xmm8,%xmm8,%xmm1
  DB  196,227,121,4,201,0                 ; vpermilps     $0x0,%xmm1,%xmm1
  DB  196,227,117,24,201,1                ; vinsertf128   $0x1,%xmm1,%ymm1,%ymm1
  DB  197,44,94,193                       ; vdivps        %ymm1,%ymm10,%ymm8
  DB  196,67,125,8,192,1                  ; vroundps      $0x1,%ymm8,%ymm8
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,172,92,201                      ; vsubps        %ymm1,%ymm10,%ymm1
  DB  196,193,116,92,201                  ; vsubps        %ymm9,%ymm1,%ymm1
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  197,60,92,193                       ; vsubps        %ymm1,%ymm8,%ymm8
  DB  197,60,84,193                       ; vandps        %ymm1,%ymm8,%ymm8
  DB  196,99,125,25,201,1                 ; vextractf128  $0x1,%ymm9,%xmm1
  DB  196,65,41,118,210                   ; vpcmpeqd      %xmm10,%xmm10,%xmm10
  DB  196,193,113,254,202                 ; vpaddd        %xmm10,%xmm1,%xmm1
  DB  196,65,49,254,202                   ; vpaddd        %xmm10,%xmm9,%xmm9
  DB  196,227,53,24,201,1                 ; vinsertf128   $0x1,%xmm1,%ymm9,%ymm1
  DB  197,188,93,201                      ; vminps        %ymm1,%ymm8,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_2x3_avx
_sk_matrix_2x3_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,98,125,24,72,8                  ; vbroadcastss  0x8(%rax),%ymm9
  DB  196,98,125,24,80,16                 ; vbroadcastss  0x10(%rax),%ymm10
  DB  197,52,89,201                       ; vmulps        %ymm1,%ymm9,%ymm9
  DB  196,65,52,88,202                    ; vaddps        %ymm10,%ymm9,%ymm9
  DB  197,60,89,192                       ; vmulps        %ymm0,%ymm8,%ymm8
  DB  196,65,60,88,193                    ; vaddps        %ymm9,%ymm8,%ymm8
  DB  196,98,125,24,72,4                  ; vbroadcastss  0x4(%rax),%ymm9
  DB  196,98,125,24,80,12                 ; vbroadcastss  0xc(%rax),%ymm10
  DB  196,98,125,24,88,20                 ; vbroadcastss  0x14(%rax),%ymm11
  DB  197,172,89,201                      ; vmulps        %ymm1,%ymm10,%ymm1
  DB  196,193,116,88,203                  ; vaddps        %ymm11,%ymm1,%ymm1
  DB  197,180,89,192                      ; vmulps        %ymm0,%ymm9,%ymm0
  DB  197,252,88,201                      ; vaddps        %ymm1,%ymm0,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,124,41,192                      ; vmovaps       %ymm8,%ymm0
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_3x4_avx
_sk_matrix_3x4_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,98,125,24,72,12                 ; vbroadcastss  0xc(%rax),%ymm9
  DB  196,98,125,24,80,24                 ; vbroadcastss  0x18(%rax),%ymm10
  DB  196,98,125,24,88,36                 ; vbroadcastss  0x24(%rax),%ymm11
  DB  197,44,89,210                       ; vmulps        %ymm2,%ymm10,%ymm10
  DB  196,65,44,88,211                    ; vaddps        %ymm11,%ymm10,%ymm10
  DB  197,52,89,201                       ; vmulps        %ymm1,%ymm9,%ymm9
  DB  196,65,52,88,202                    ; vaddps        %ymm10,%ymm9,%ymm9
  DB  197,60,89,192                       ; vmulps        %ymm0,%ymm8,%ymm8
  DB  196,65,60,88,193                    ; vaddps        %ymm9,%ymm8,%ymm8
  DB  196,98,125,24,72,4                  ; vbroadcastss  0x4(%rax),%ymm9
  DB  196,98,125,24,80,16                 ; vbroadcastss  0x10(%rax),%ymm10
  DB  196,98,125,24,88,28                 ; vbroadcastss  0x1c(%rax),%ymm11
  DB  196,98,125,24,96,40                 ; vbroadcastss  0x28(%rax),%ymm12
  DB  197,36,89,218                       ; vmulps        %ymm2,%ymm11,%ymm11
  DB  196,65,36,88,220                    ; vaddps        %ymm12,%ymm11,%ymm11
  DB  197,44,89,209                       ; vmulps        %ymm1,%ymm10,%ymm10
  DB  196,65,44,88,211                    ; vaddps        %ymm11,%ymm10,%ymm10
  DB  197,52,89,200                       ; vmulps        %ymm0,%ymm9,%ymm9
  DB  196,65,52,88,202                    ; vaddps        %ymm10,%ymm9,%ymm9
  DB  196,98,125,24,80,8                  ; vbroadcastss  0x8(%rax),%ymm10
  DB  196,98,125,24,88,20                 ; vbroadcastss  0x14(%rax),%ymm11
  DB  196,98,125,24,96,32                 ; vbroadcastss  0x20(%rax),%ymm12
  DB  196,98,125,24,104,44                ; vbroadcastss  0x2c(%rax),%ymm13
  DB  197,156,89,210                      ; vmulps        %ymm2,%ymm12,%ymm2
  DB  196,193,108,88,213                  ; vaddps        %ymm13,%ymm2,%ymm2
  DB  197,164,89,201                      ; vmulps        %ymm1,%ymm11,%ymm1
  DB  197,244,88,202                      ; vaddps        %ymm2,%ymm1,%ymm1
  DB  197,172,89,192                      ; vmulps        %ymm0,%ymm10,%ymm0
  DB  197,252,88,209                      ; vaddps        %ymm1,%ymm0,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,124,41,192                      ; vmovaps       %ymm8,%ymm0
  DB  197,124,41,201                      ; vmovaps       %ymm9,%ymm1
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_perspective_avx
_sk_matrix_perspective_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,98,125,24,72,4                  ; vbroadcastss  0x4(%rax),%ymm9
  DB  196,98,125,24,80,8                  ; vbroadcastss  0x8(%rax),%ymm10
  DB  197,52,89,201                       ; vmulps        %ymm1,%ymm9,%ymm9
  DB  196,65,52,88,202                    ; vaddps        %ymm10,%ymm9,%ymm9
  DB  197,60,89,192                       ; vmulps        %ymm0,%ymm8,%ymm8
  DB  196,65,60,88,193                    ; vaddps        %ymm9,%ymm8,%ymm8
  DB  196,98,125,24,72,12                 ; vbroadcastss  0xc(%rax),%ymm9
  DB  196,98,125,24,80,16                 ; vbroadcastss  0x10(%rax),%ymm10
  DB  196,98,125,24,88,20                 ; vbroadcastss  0x14(%rax),%ymm11
  DB  197,44,89,209                       ; vmulps        %ymm1,%ymm10,%ymm10
  DB  196,65,44,88,211                    ; vaddps        %ymm11,%ymm10,%ymm10
  DB  197,52,89,200                       ; vmulps        %ymm0,%ymm9,%ymm9
  DB  196,65,52,88,202                    ; vaddps        %ymm10,%ymm9,%ymm9
  DB  196,98,125,24,80,24                 ; vbroadcastss  0x18(%rax),%ymm10
  DB  196,98,125,24,88,28                 ; vbroadcastss  0x1c(%rax),%ymm11
  DB  196,98,125,24,96,32                 ; vbroadcastss  0x20(%rax),%ymm12
  DB  197,164,89,201                      ; vmulps        %ymm1,%ymm11,%ymm1
  DB  196,193,116,88,204                  ; vaddps        %ymm12,%ymm1,%ymm1
  DB  197,172,89,192                      ; vmulps        %ymm0,%ymm10,%ymm0
  DB  197,252,88,193                      ; vaddps        %ymm1,%ymm0,%ymm0
  DB  197,252,83,200                      ; vrcpps        %ymm0,%ymm1
  DB  197,188,89,193                      ; vmulps        %ymm1,%ymm8,%ymm0
  DB  197,180,89,201                      ; vmulps        %ymm1,%ymm9,%ymm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_linear_gradient_2stops_avx
_sk_linear_gradient_2stops_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,226,125,24,72,16                ; vbroadcastss  0x10(%rax),%ymm1
  DB  196,226,125,24,16                   ; vbroadcastss  (%rax),%ymm2
  DB  197,244,89,200                      ; vmulps        %ymm0,%ymm1,%ymm1
  DB  197,108,88,193                      ; vaddps        %ymm1,%ymm2,%ymm8
  DB  196,226,125,24,72,20                ; vbroadcastss  0x14(%rax),%ymm1
  DB  196,226,125,24,80,4                 ; vbroadcastss  0x4(%rax),%ymm2
  DB  197,244,89,200                      ; vmulps        %ymm0,%ymm1,%ymm1
  DB  197,236,88,201                      ; vaddps        %ymm1,%ymm2,%ymm1
  DB  196,226,125,24,80,24                ; vbroadcastss  0x18(%rax),%ymm2
  DB  196,226,125,24,88,8                 ; vbroadcastss  0x8(%rax),%ymm3
  DB  197,236,89,208                      ; vmulps        %ymm0,%ymm2,%ymm2
  DB  197,228,88,210                      ; vaddps        %ymm2,%ymm3,%ymm2
  DB  196,226,125,24,88,28                ; vbroadcastss  0x1c(%rax),%ymm3
  DB  196,98,125,24,72,12                 ; vbroadcastss  0xc(%rax),%ymm9
  DB  197,228,89,192                      ; vmulps        %ymm0,%ymm3,%ymm0
  DB  197,180,88,216                      ; vaddps        %ymm0,%ymm9,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,124,41,192                      ; vmovaps       %ymm8,%ymm0
  DB  255,224                             ; jmpq          *%rax

ALIGN 4
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  145                                 ; xchg          %eax,%ecx
  DB  131,158,61,154,153,153,62           ; sbbl          $0x3e,-0x666665c3(%rsi)
  DB  92                                  ; pop           %rsp
  DB  143                                 ; (bad)
  DB  50,63                               ; xor           (%rdi),%bh
  DB  10,215                              ; or            %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  174                                 ; scas          %es:(%rdi),%al
  DB  71,97                               ; rex.RXB       (bad)
  DB  61,41,92,71,65                      ; cmp           $0x41475c29,%eax
  DB  206                                 ; (bad)
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  168,87                              ; test          $0x57,%al
  DB  202,189,194                         ; lret          $0xc2bd
  DB  135,210                             ; xchg          %edx,%edx
  DB  62,0,0                              ; add           %al,%ds:(%rax)
  DB  128,63,4                            ; cmpb          $0x4,(%rdi)
  DB  231,140                             ; out           %eax,$0x8c
  DB  59,129,128,128,59,129               ; cmp           -0x7ec47f80(%rcx),%eax
  DB  128,128,59,0,248,0,0                ; addb          $0x0,0xf8003b(%rax)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  224,7                               ; loopne        10d1 <.literal4+0x61>
  DB  0,0                                 ; add           %al,(%rax)
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  31                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,8                                 ; add           %cl,(%rax)
  DB  33,4,61,0,0,128,63                  ; and           %eax,0x3f800000(,%rdi,1)
  DB  129,128,128,59,129,128,128,59,0,0   ; addl          $0x3b80,-0x7f7ec480(%rax)
  DB  127,67                              ; jg            112b <_sk_linear_gradient_2stops_avx+0x116>
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  224,7                               ; loopne        10f9 <.literal4+0x89>
  DB  0,0                                 ; add           %al,(%rax)
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  31                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,8                                 ; add           %cl,(%rax)
  DB  33,4,61,0,0,128,63                  ; and           %eax,0x3f800000(,%rdi,1)
  DB  0,0                                 ; add           %al,(%rax)
  DB  248                                 ; clc
  DB  65,0,0                              ; add           %al,(%r8)
  DB  124,66                              ; jl            114e <_sk_linear_gradient_2stops_avx+0x139>
  DB  129,128,128,59,0,0,127,67,0,0       ; addl          $0x437f,0x3b80(%rax)
  DB  128,119,0,0                         ; xorb          $0x0,0x0(%rdi)
  DB  128                                 ; .byte         0x80
  DB  7                                   ; (bad)

ALIGN 16
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  64,0,0                              ; add           %al,(%rax)
  DB  64                                  ; rex
  DB  64,0,0                              ; add           %al,(%rax)
  DB  128,64,0,0                          ; addb          $0x0,0x0(%rax)
  DB  160,64,0,0,192,64,0,0,224           ; movabs        0xe0000040c0000040,%al
  DB  64                                  ; rex

ALIGN 16
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,4,0                               ; add           %al,(%rax,%rax,1)
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4                                   ; .byte         0x4
ALIGN 16

PUBLIC _sk_start_pipeline_sse41
_sk_start_pipeline_sse41 LABEL PROC
  DB  65,87                               ; push          %r15
  DB  65,86                               ; push          %r14
  DB  65,85                               ; push          %r13
  DB  65,84                               ; push          %r12
  DB  86                                  ; push          %rsi
  DB  87                                  ; push          %rdi
  DB  83                                  ; push          %rbx
  DB  72,129,236,160,0,0,0                ; sub           $0xa0,%rsp
  DB  68,15,41,188,36,144,0,0,0           ; movaps        %xmm15,0x90(%rsp)
  DB  68,15,41,180,36,128,0,0,0           ; movaps        %xmm14,0x80(%rsp)
  DB  68,15,41,108,36,112                 ; movaps        %xmm13,0x70(%rsp)
  DB  68,15,41,100,36,96                  ; movaps        %xmm12,0x60(%rsp)
  DB  68,15,41,92,36,80                   ; movaps        %xmm11,0x50(%rsp)
  DB  68,15,41,84,36,64                   ; movaps        %xmm10,0x40(%rsp)
  DB  68,15,41,76,36,48                   ; movaps        %xmm9,0x30(%rsp)
  DB  68,15,41,68,36,32                   ; movaps        %xmm8,0x20(%rsp)
  DB  15,41,124,36,16                     ; movaps        %xmm7,0x10(%rsp)
  DB  15,41,52,36                         ; movaps        %xmm6,(%rsp)
  DB  77,137,207                          ; mov           %r9,%r15
  DB  77,137,198                          ; mov           %r8,%r14
  DB  72,137,203                          ; mov           %rcx,%rbx
  DB  72,137,214                          ; mov           %rdx,%rsi
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  73,137,196                          ; mov           %rax,%r12
  DB  73,137,245                          ; mov           %rsi,%r13
  DB  72,141,67,4                         ; lea           0x4(%rbx),%rax
  DB  76,57,248                           ; cmp           %r15,%rax
  DB  118,5                               ; jbe           73 <_sk_start_pipeline_sse41+0x73>
  DB  72,137,216                          ; mov           %rbx,%rax
  DB  235,52                              ; jmp           a7 <_sk_start_pipeline_sse41+0xa7>
  DB  15,87,192                           ; xorps         %xmm0,%xmm0
  DB  15,87,201                           ; xorps         %xmm1,%xmm1
  DB  15,87,210                           ; xorps         %xmm2,%xmm2
  DB  15,87,219                           ; xorps         %xmm3,%xmm3
  DB  15,87,228                           ; xorps         %xmm4,%xmm4
  DB  15,87,237                           ; xorps         %xmm5,%xmm5
  DB  15,87,246                           ; xorps         %xmm6,%xmm6
  DB  15,87,255                           ; xorps         %xmm7,%xmm7
  DB  72,137,223                          ; mov           %rbx,%rdi
  DB  76,137,238                          ; mov           %r13,%rsi
  DB  76,137,242                          ; mov           %r14,%rdx
  DB  65,255,212                          ; callq         *%r12
  DB  72,141,67,4                         ; lea           0x4(%rbx),%rax
  DB  72,131,195,8                        ; add           $0x8,%rbx
  DB  76,57,251                           ; cmp           %r15,%rbx
  DB  72,137,195                          ; mov           %rax,%rbx
  DB  118,204                             ; jbe           73 <_sk_start_pipeline_sse41+0x73>
  DB  15,40,52,36                         ; movaps        (%rsp),%xmm6
  DB  15,40,124,36,16                     ; movaps        0x10(%rsp),%xmm7
  DB  68,15,40,68,36,32                   ; movaps        0x20(%rsp),%xmm8
  DB  68,15,40,76,36,48                   ; movaps        0x30(%rsp),%xmm9
  DB  68,15,40,84,36,64                   ; movaps        0x40(%rsp),%xmm10
  DB  68,15,40,92,36,80                   ; movaps        0x50(%rsp),%xmm11
  DB  68,15,40,100,36,96                  ; movaps        0x60(%rsp),%xmm12
  DB  68,15,40,108,36,112                 ; movaps        0x70(%rsp),%xmm13
  DB  68,15,40,180,36,128,0,0,0           ; movaps        0x80(%rsp),%xmm14
  DB  68,15,40,188,36,144,0,0,0           ; movaps        0x90(%rsp),%xmm15
  DB  72,129,196,160,0,0,0                ; add           $0xa0,%rsp
  DB  91                                  ; pop           %rbx
  DB  95                                  ; pop           %rdi
  DB  94                                  ; pop           %rsi
  DB  65,92                               ; pop           %r12
  DB  65,93                               ; pop           %r13
  DB  65,94                               ; pop           %r14
  DB  65,95                               ; pop           %r15
  DB  195                                 ; retq

PUBLIC _sk_just_return_sse41
_sk_just_return_sse41 LABEL PROC
  DB  195                                 ; retq

PUBLIC _sk_seed_shader_sse41
_sk_seed_shader_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  102,15,110,199                      ; movd          %edi,%xmm0
  DB  102,15,112,192,0                    ; pshufd        $0x0,%xmm0,%xmm0
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  15,40,21,177,12,0,0                 ; movaps        0xcb1(%rip),%xmm2        # dc0 <_sk_linear_gradient_2stops_sse41+0x6b>
  DB  15,88,194                           ; addps         %xmm2,%xmm0
  DB  15,88,5,183,12,0,0                  ; addps         0xcb7(%rip),%xmm0        # dd0 <_sk_linear_gradient_2stops_sse41+0x7b>
  DB  102,15,110,8                        ; movd          (%rax),%xmm1
  DB  102,15,112,201,0                    ; pshufd        $0x0,%xmm1,%xmm1
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  15,88,202                           ; addps         %xmm2,%xmm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,21,175,12,0,0                 ; movaps        0xcaf(%rip),%xmm2        # de0 <_sk_linear_gradient_2stops_sse41+0x8b>
  DB  15,87,219                           ; xorps         %xmm3,%xmm3
  DB  15,87,228                           ; xorps         %xmm4,%xmm4
  DB  15,87,237                           ; xorps         %xmm5,%xmm5
  DB  15,87,246                           ; xorps         %xmm6,%xmm6
  DB  15,87,255                           ; xorps         %xmm7,%xmm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_constant_color_sse41
_sk_constant_color_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,16,24                            ; movups        (%rax),%xmm3
  DB  15,40,195                           ; movaps        %xmm3,%xmm0
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  15,40,203                           ; movaps        %xmm3,%xmm1
  DB  15,198,201,85                       ; shufps        $0x55,%xmm1,%xmm1
  DB  15,40,211                           ; movaps        %xmm3,%xmm2
  DB  15,198,210,170                      ; shufps        $0xaa,%xmm2,%xmm2
  DB  15,198,219,255                      ; shufps        $0xff,%xmm3,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clear_sse41
_sk_clear_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,87,192                           ; xorps         %xmm0,%xmm0
  DB  15,87,201                           ; xorps         %xmm1,%xmm1
  DB  15,87,210                           ; xorps         %xmm2,%xmm2
  DB  15,87,219                           ; xorps         %xmm3,%xmm3
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_plus__sse41
_sk_plus__sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,88,223                           ; addps         %xmm7,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_srcover_sse41
_sk_srcover_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,40,5,94,12,0,0                ; movaps        0xc5e(%rip),%xmm8        # df0 <_sk_linear_gradient_2stops_sse41+0x9b>
  DB  68,15,92,195                        ; subps         %xmm3,%xmm8
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,204                        ; mulps         %xmm4,%xmm9
  DB  65,15,88,193                        ; addps         %xmm9,%xmm0
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,205                        ; mulps         %xmm5,%xmm9
  DB  65,15,88,201                        ; addps         %xmm9,%xmm1
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,206                        ; mulps         %xmm6,%xmm9
  DB  65,15,88,209                        ; addps         %xmm9,%xmm2
  DB  68,15,89,199                        ; mulps         %xmm7,%xmm8
  DB  65,15,88,216                        ; addps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_dstover_sse41
_sk_dstover_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,40,5,48,12,0,0                ; movaps        0xc30(%rip),%xmm8        # e00 <_sk_linear_gradient_2stops_sse41+0xab>
  DB  68,15,92,199                        ; subps         %xmm7,%xmm8
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  15,88,223                           ; addps         %xmm7,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_0_sse41
_sk_clamp_0_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  69,15,87,192                        ; xorps         %xmm8,%xmm8
  DB  65,15,95,192                        ; maxps         %xmm8,%xmm0
  DB  65,15,95,200                        ; maxps         %xmm8,%xmm1
  DB  65,15,95,208                        ; maxps         %xmm8,%xmm2
  DB  65,15,95,216                        ; maxps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_1_sse41
_sk_clamp_1_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,40,5,248,11,0,0               ; movaps        0xbf8(%rip),%xmm8        # e10 <_sk_linear_gradient_2stops_sse41+0xbb>
  DB  65,15,93,192                        ; minps         %xmm8,%xmm0
  DB  65,15,93,200                        ; minps         %xmm8,%xmm1
  DB  65,15,93,208                        ; minps         %xmm8,%xmm2
  DB  65,15,93,216                        ; minps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_a_sse41
_sk_clamp_a_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,93,29,235,11,0,0                 ; minps         0xbeb(%rip),%xmm3        # e20 <_sk_linear_gradient_2stops_sse41+0xcb>
  DB  15,93,195                           ; minps         %xmm3,%xmm0
  DB  15,93,203                           ; minps         %xmm3,%xmm1
  DB  15,93,211                           ; minps         %xmm3,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_set_rgb_sse41
_sk_set_rgb_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,15,16,0                         ; movss         (%rax),%xmm0
  DB  243,15,16,72,4                      ; movss         0x4(%rax),%xmm1
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  243,15,16,80,8                      ; movss         0x8(%rax),%xmm2
  DB  15,198,210,0                        ; shufps        $0x0,%xmm2,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_swap_rb_sse41
_sk_swap_rb_sse41 LABEL PROC
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,194                           ; movaps        %xmm2,%xmm0
  DB  65,15,40,208                        ; movaps        %xmm8,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_swap_sse41
_sk_swap_sse41 LABEL PROC
  DB  68,15,40,195                        ; movaps        %xmm3,%xmm8
  DB  68,15,40,202                        ; movaps        %xmm2,%xmm9
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  68,15,40,216                        ; movaps        %xmm0,%xmm11
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,196                           ; movaps        %xmm4,%xmm0
  DB  15,40,205                           ; movaps        %xmm5,%xmm1
  DB  15,40,214                           ; movaps        %xmm6,%xmm2
  DB  15,40,223                           ; movaps        %xmm7,%xmm3
  DB  65,15,40,227                        ; movaps        %xmm11,%xmm4
  DB  65,15,40,234                        ; movaps        %xmm10,%xmm5
  DB  65,15,40,241                        ; movaps        %xmm9,%xmm6
  DB  65,15,40,248                        ; movaps        %xmm8,%xmm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_move_src_dst_sse41
_sk_move_src_dst_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,224                           ; movaps        %xmm0,%xmm4
  DB  15,40,233                           ; movaps        %xmm1,%xmm5
  DB  15,40,242                           ; movaps        %xmm2,%xmm6
  DB  15,40,251                           ; movaps        %xmm3,%xmm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_move_dst_src_sse41
_sk_move_dst_src_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,196                           ; movaps        %xmm4,%xmm0
  DB  15,40,205                           ; movaps        %xmm5,%xmm1
  DB  15,40,214                           ; movaps        %xmm6,%xmm2
  DB  15,40,223                           ; movaps        %xmm7,%xmm3
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_premul_sse41
_sk_premul_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,89,195                           ; mulps         %xmm3,%xmm0
  DB  15,89,203                           ; mulps         %xmm3,%xmm1
  DB  15,89,211                           ; mulps         %xmm3,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_unpremul_sse41
_sk_unpremul_sse41 LABEL PROC
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  69,15,87,201                        ; xorps         %xmm9,%xmm9
  DB  68,15,40,21,70,11,0,0               ; movaps        0xb46(%rip),%xmm10        # e30 <_sk_linear_gradient_2stops_sse41+0xdb>
  DB  68,15,94,211                        ; divps         %xmm3,%xmm10
  DB  15,40,195                           ; movaps        %xmm3,%xmm0
  DB  65,15,194,193,0                     ; cmpeqps       %xmm9,%xmm0
  DB  102,69,15,56,20,209                 ; blendvps      %xmm0,%xmm9,%xmm10
  DB  69,15,89,194                        ; mulps         %xmm10,%xmm8
  DB  65,15,89,202                        ; mulps         %xmm10,%xmm1
  DB  65,15,89,210                        ; mulps         %xmm10,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,192                        ; movaps        %xmm8,%xmm0
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_from_srgb_sse41
_sk_from_srgb_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,40,29,38,11,0,0               ; movaps        0xb26(%rip),%xmm11        # e40 <_sk_linear_gradient_2stops_sse41+0xeb>
  DB  68,15,40,200                        ; movaps        %xmm0,%xmm9
  DB  69,15,89,203                        ; mulps         %xmm11,%xmm9
  DB  68,15,40,208                        ; movaps        %xmm0,%xmm10
  DB  69,15,89,210                        ; mulps         %xmm10,%xmm10
  DB  68,15,40,37,30,11,0,0               ; movaps        0xb1e(%rip),%xmm12        # e50 <_sk_linear_gradient_2stops_sse41+0xfb>
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  69,15,89,196                        ; mulps         %xmm12,%xmm8
  DB  68,15,40,45,30,11,0,0               ; movaps        0xb1e(%rip),%xmm13        # e60 <_sk_linear_gradient_2stops_sse41+0x10b>
  DB  69,15,88,197                        ; addps         %xmm13,%xmm8
  DB  69,15,89,194                        ; mulps         %xmm10,%xmm8
  DB  68,15,40,53,30,11,0,0               ; movaps        0xb1e(%rip),%xmm14        # e70 <_sk_linear_gradient_2stops_sse41+0x11b>
  DB  69,15,88,198                        ; addps         %xmm14,%xmm8
  DB  68,15,40,61,34,11,0,0               ; movaps        0xb22(%rip),%xmm15        # e80 <_sk_linear_gradient_2stops_sse41+0x12b>
  DB  65,15,194,199,1                     ; cmpltps       %xmm15,%xmm0
  DB  102,69,15,56,20,193                 ; blendvps      %xmm0,%xmm9,%xmm8
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  69,15,89,211                        ; mulps         %xmm11,%xmm10
  DB  15,40,193                           ; movaps        %xmm1,%xmm0
  DB  15,89,192                           ; mulps         %xmm0,%xmm0
  DB  68,15,40,201                        ; movaps        %xmm1,%xmm9
  DB  69,15,89,204                        ; mulps         %xmm12,%xmm9
  DB  69,15,88,205                        ; addps         %xmm13,%xmm9
  DB  68,15,89,200                        ; mulps         %xmm0,%xmm9
  DB  69,15,88,206                        ; addps         %xmm14,%xmm9
  DB  65,15,194,207,1                     ; cmpltps       %xmm15,%xmm1
  DB  15,40,193                           ; movaps        %xmm1,%xmm0
  DB  102,69,15,56,20,202                 ; blendvps      %xmm0,%xmm10,%xmm9
  DB  68,15,89,218                        ; mulps         %xmm2,%xmm11
  DB  68,15,40,210                        ; movaps        %xmm2,%xmm10
  DB  69,15,89,210                        ; mulps         %xmm10,%xmm10
  DB  68,15,89,226                        ; mulps         %xmm2,%xmm12
  DB  69,15,88,229                        ; addps         %xmm13,%xmm12
  DB  69,15,89,212                        ; mulps         %xmm12,%xmm10
  DB  69,15,88,214                        ; addps         %xmm14,%xmm10
  DB  65,15,194,215,1                     ; cmpltps       %xmm15,%xmm2
  DB  15,40,194                           ; movaps        %xmm2,%xmm0
  DB  102,69,15,56,20,211                 ; blendvps      %xmm0,%xmm11,%xmm10
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,192                        ; movaps        %xmm8,%xmm0
  DB  65,15,40,201                        ; movaps        %xmm9,%xmm1
  DB  65,15,40,210                        ; movaps        %xmm10,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_to_srgb_sse41
_sk_to_srgb_sse41 LABEL PROC
  DB  72,131,236,24                       ; sub           $0x18,%rsp
  DB  15,41,60,36                         ; movaps        %xmm7,(%rsp)
  DB  15,40,254                           ; movaps        %xmm6,%xmm7
  DB  15,40,245                           ; movaps        %xmm5,%xmm6
  DB  15,40,236                           ; movaps        %xmm4,%xmm5
  DB  15,40,227                           ; movaps        %xmm3,%xmm4
  DB  15,40,218                           ; movaps        %xmm2,%xmm3
  DB  15,40,209                           ; movaps        %xmm1,%xmm2
  DB  68,15,82,192                        ; rsqrtps       %xmm0,%xmm8
  DB  69,15,83,200                        ; rcpps         %xmm8,%xmm9
  DB  69,15,82,208                        ; rsqrtps       %xmm8,%xmm10
  DB  68,15,40,29,143,10,0,0              ; movaps        0xa8f(%rip),%xmm11        # e90 <_sk_linear_gradient_2stops_sse41+0x13b>
  DB  15,40,200                           ; movaps        %xmm0,%xmm1
  DB  65,15,89,203                        ; mulps         %xmm11,%xmm1
  DB  68,15,40,37,144,10,0,0              ; movaps        0xa90(%rip),%xmm12        # ea0 <_sk_linear_gradient_2stops_sse41+0x14b>
  DB  69,15,89,204                        ; mulps         %xmm12,%xmm9
  DB  68,15,40,45,148,10,0,0              ; movaps        0xa94(%rip),%xmm13        # eb0 <_sk_linear_gradient_2stops_sse41+0x15b>
  DB  69,15,88,205                        ; addps         %xmm13,%xmm9
  DB  68,15,40,53,152,10,0,0              ; movaps        0xa98(%rip),%xmm14        # ec0 <_sk_linear_gradient_2stops_sse41+0x16b>
  DB  69,15,89,214                        ; mulps         %xmm14,%xmm10
  DB  69,15,88,209                        ; addps         %xmm9,%xmm10
  DB  68,15,40,5,152,10,0,0               ; movaps        0xa98(%rip),%xmm8        # ed0 <_sk_linear_gradient_2stops_sse41+0x17b>
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  69,15,93,202                        ; minps         %xmm10,%xmm9
  DB  68,15,40,61,152,10,0,0              ; movaps        0xa98(%rip),%xmm15        # ee0 <_sk_linear_gradient_2stops_sse41+0x18b>
  DB  65,15,194,199,1                     ; cmpltps       %xmm15,%xmm0
  DB  102,68,15,56,20,201                 ; blendvps      %xmm0,%xmm1,%xmm9
  DB  15,82,194                           ; rsqrtps       %xmm2,%xmm0
  DB  15,83,200                           ; rcpps         %xmm0,%xmm1
  DB  15,82,192                           ; rsqrtps       %xmm0,%xmm0
  DB  65,15,89,204                        ; mulps         %xmm12,%xmm1
  DB  65,15,88,205                        ; addps         %xmm13,%xmm1
  DB  65,15,89,198                        ; mulps         %xmm14,%xmm0
  DB  15,88,193                           ; addps         %xmm1,%xmm0
  DB  69,15,40,208                        ; movaps        %xmm8,%xmm10
  DB  68,15,93,208                        ; minps         %xmm0,%xmm10
  DB  15,40,202                           ; movaps        %xmm2,%xmm1
  DB  65,15,89,203                        ; mulps         %xmm11,%xmm1
  DB  65,15,194,215,1                     ; cmpltps       %xmm15,%xmm2
  DB  15,40,194                           ; movaps        %xmm2,%xmm0
  DB  102,68,15,56,20,209                 ; blendvps      %xmm0,%xmm1,%xmm10
  DB  15,82,195                           ; rsqrtps       %xmm3,%xmm0
  DB  15,83,200                           ; rcpps         %xmm0,%xmm1
  DB  65,15,89,204                        ; mulps         %xmm12,%xmm1
  DB  65,15,88,205                        ; addps         %xmm13,%xmm1
  DB  15,82,192                           ; rsqrtps       %xmm0,%xmm0
  DB  65,15,89,198                        ; mulps         %xmm14,%xmm0
  DB  15,88,193                           ; addps         %xmm1,%xmm0
  DB  68,15,93,192                        ; minps         %xmm0,%xmm8
  DB  68,15,89,219                        ; mulps         %xmm3,%xmm11
  DB  65,15,194,223,1                     ; cmpltps       %xmm15,%xmm3
  DB  15,40,195                           ; movaps        %xmm3,%xmm0
  DB  102,69,15,56,20,195                 ; blendvps      %xmm0,%xmm11,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,193                        ; movaps        %xmm9,%xmm0
  DB  65,15,40,202                        ; movaps        %xmm10,%xmm1
  DB  65,15,40,208                        ; movaps        %xmm8,%xmm2
  DB  15,40,220                           ; movaps        %xmm4,%xmm3
  DB  15,40,229                           ; movaps        %xmm5,%xmm4
  DB  15,40,238                           ; movaps        %xmm6,%xmm5
  DB  15,40,247                           ; movaps        %xmm7,%xmm6
  DB  15,40,60,36                         ; movaps        (%rsp),%xmm7
  DB  72,131,196,24                       ; add           $0x18,%rsp
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_scale_1_float_sse41
_sk_scale_1_float_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_scale_u8_sse41
_sk_scale_u8_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,68,15,56,49,4,56                ; pmovzxbd      (%rax,%rdi,1),%xmm8
  DB  69,15,91,192                        ; cvtdq2ps      %xmm8,%xmm8
  DB  68,15,89,5,220,9,0,0                ; mulps         0x9dc(%rip),%xmm8        # ef0 <_sk_linear_gradient_2stops_sse41+0x19b>
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_1_float_sse41
_sk_lerp_1_float_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,92,223                           ; subps         %xmm7,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  15,88,223                           ; addps         %xmm7,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_u8_sse41
_sk_lerp_u8_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,68,15,56,49,4,56                ; pmovzxbd      (%rax,%rdi,1),%xmm8
  DB  69,15,91,192                        ; cvtdq2ps      %xmm8,%xmm8
  DB  68,15,89,5,136,9,0,0                ; mulps         0x988(%rip),%xmm8        # f00 <_sk_linear_gradient_2stops_sse41+0x1ab>
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,92,223                           ; subps         %xmm7,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  15,88,223                           ; addps         %xmm7,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_565_sse41
_sk_lerp_565_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,68,15,56,51,4,120               ; pmovzxwd      (%rax,%rdi,2),%xmm8
  DB  102,15,111,29,88,9,0,0              ; movdqa        0x958(%rip),%xmm3        # f10 <_sk_linear_gradient_2stops_sse41+0x1bb>
  DB  102,65,15,219,216                   ; pand          %xmm8,%xmm3
  DB  68,15,91,203                        ; cvtdq2ps      %xmm3,%xmm9
  DB  68,15,89,13,87,9,0,0                ; mulps         0x957(%rip),%xmm9        # f20 <_sk_linear_gradient_2stops_sse41+0x1cb>
  DB  102,15,111,29,95,9,0,0              ; movdqa        0x95f(%rip),%xmm3        # f30 <_sk_linear_gradient_2stops_sse41+0x1db>
  DB  102,65,15,219,216                   ; pand          %xmm8,%xmm3
  DB  15,91,219                           ; cvtdq2ps      %xmm3,%xmm3
  DB  15,89,29,96,9,0,0                   ; mulps         0x960(%rip),%xmm3        # f40 <_sk_linear_gradient_2stops_sse41+0x1eb>
  DB  102,68,15,219,5,103,9,0,0           ; pand          0x967(%rip),%xmm8        # f50 <_sk_linear_gradient_2stops_sse41+0x1fb>
  DB  69,15,91,192                        ; cvtdq2ps      %xmm8,%xmm8
  DB  68,15,89,5,107,9,0,0                ; mulps         0x96b(%rip),%xmm8        # f60 <_sk_linear_gradient_2stops_sse41+0x20b>
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,193                        ; mulps         %xmm9,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  15,89,203                           ; mulps         %xmm3,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,29,85,9,0,0                   ; movaps        0x955(%rip),%xmm3        # f70 <_sk_linear_gradient_2stops_sse41+0x21b>
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_tables_sse41
_sk_load_tables_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,0                            ; mov           (%rax),%r8
  DB  76,139,80,8                         ; mov           0x8(%rax),%r10
  DB  243,69,15,111,12,184                ; movdqu        (%r8,%rdi,4),%xmm9
  DB  102,68,15,111,5,75,9,0,0            ; movdqa        0x94b(%rip),%xmm8        # f80 <_sk_linear_gradient_2stops_sse41+0x22b>
  DB  102,65,15,111,193                   ; movdqa        %xmm9,%xmm0
  DB  102,65,15,219,192                   ; pand          %xmm8,%xmm0
  DB  102,73,15,58,22,192,1               ; pextrq        $0x1,%xmm0,%r8
  DB  69,137,193                          ; mov           %r8d,%r9d
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  102,72,15,126,193                   ; movq          %xmm0,%rcx
  DB  65,137,203                          ; mov           %ecx,%r11d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  243,67,15,16,4,154                  ; movss         (%r10,%r11,4),%xmm0
  DB  102,65,15,58,33,4,138,16            ; insertps      $0x10,(%r10,%rcx,4),%xmm0
  DB  102,67,15,58,33,4,138,32            ; insertps      $0x20,(%r10,%r9,4),%xmm0
  DB  102,67,15,58,33,4,130,48            ; insertps      $0x30,(%r10,%r8,4),%xmm0
  DB  76,139,80,16                        ; mov           0x10(%rax),%r10
  DB  102,65,15,111,201                   ; movdqa        %xmm9,%xmm1
  DB  102,15,114,209,8                    ; psrld         $0x8,%xmm1
  DB  102,65,15,219,200                   ; pand          %xmm8,%xmm1
  DB  102,73,15,58,22,200,1               ; pextrq        $0x1,%xmm1,%r8
  DB  69,137,193                          ; mov           %r8d,%r9d
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  102,72,15,126,201                   ; movq          %xmm1,%rcx
  DB  65,137,203                          ; mov           %ecx,%r11d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  243,67,15,16,12,154                 ; movss         (%r10,%r11,4),%xmm1
  DB  102,65,15,58,33,12,138,16           ; insertps      $0x10,(%r10,%rcx,4),%xmm1
  DB  243,67,15,16,20,138                 ; movss         (%r10,%r9,4),%xmm2
  DB  102,15,58,33,202,32                 ; insertps      $0x20,%xmm2,%xmm1
  DB  243,67,15,16,20,130                 ; movss         (%r10,%r8,4),%xmm2
  DB  102,15,58,33,202,48                 ; insertps      $0x30,%xmm2,%xmm1
  DB  76,139,72,24                        ; mov           0x18(%rax),%r9
  DB  102,65,15,111,209                   ; movdqa        %xmm9,%xmm2
  DB  102,15,114,210,16                   ; psrld         $0x10,%xmm2
  DB  102,65,15,219,208                   ; pand          %xmm8,%xmm2
  DB  102,72,15,58,22,209,1               ; pextrq        $0x1,%xmm2,%rcx
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  102,72,15,126,208                   ; movq          %xmm2,%rax
  DB  65,137,194                          ; mov           %eax,%r10d
  DB  72,193,232,32                       ; shr           $0x20,%rax
  DB  243,67,15,16,20,145                 ; movss         (%r9,%r10,4),%xmm2
  DB  102,65,15,58,33,20,129,16           ; insertps      $0x10,(%r9,%rax,4),%xmm2
  DB  243,67,15,16,28,129                 ; movss         (%r9,%r8,4),%xmm3
  DB  102,15,58,33,211,32                 ; insertps      $0x20,%xmm3,%xmm2
  DB  243,65,15,16,28,137                 ; movss         (%r9,%rcx,4),%xmm3
  DB  102,15,58,33,211,48                 ; insertps      $0x30,%xmm3,%xmm2
  DB  102,65,15,114,209,24                ; psrld         $0x18,%xmm9
  DB  65,15,91,217                        ; cvtdq2ps      %xmm9,%xmm3
  DB  15,89,29,98,8,0,0                   ; mulps         0x862(%rip),%xmm3        # f90 <_sk_linear_gradient_2stops_sse41+0x23b>
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_a8_sse41
_sk_load_a8_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,15,56,49,4,56                   ; pmovzxbd      (%rax,%rdi,1),%xmm0
  DB  15,91,216                           ; cvtdq2ps      %xmm0,%xmm3
  DB  15,89,29,89,8,0,0                   ; mulps         0x859(%rip),%xmm3        # fa0 <_sk_linear_gradient_2stops_sse41+0x24b>
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,87,192                           ; xorps         %xmm0,%xmm0
  DB  15,87,201                           ; xorps         %xmm1,%xmm1
  DB  15,87,210                           ; xorps         %xmm2,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_a8_sse41
_sk_store_a8_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,40,5,79,8,0,0                 ; movaps        0x84f(%rip),%xmm8        # fb0 <_sk_linear_gradient_2stops_sse41+0x25b>
  DB  68,15,89,195                        ; mulps         %xmm3,%xmm8
  DB  102,69,15,91,192                    ; cvtps2dq      %xmm8,%xmm8
  DB  102,69,15,56,43,192                 ; packusdw      %xmm8,%xmm8
  DB  102,69,15,103,192                   ; packuswb      %xmm8,%xmm8
  DB  102,68,15,126,4,56                  ; movd          %xmm8,(%rax,%rdi,1)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_565_sse41
_sk_load_565_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,15,56,51,20,120                 ; pmovzxwd      (%rax,%rdi,2),%xmm2
  DB  102,15,111,5,46,8,0,0               ; movdqa        0x82e(%rip),%xmm0        # fc0 <_sk_linear_gradient_2stops_sse41+0x26b>
  DB  102,15,219,194                      ; pand          %xmm2,%xmm0
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  15,89,5,48,8,0,0                    ; mulps         0x830(%rip),%xmm0        # fd0 <_sk_linear_gradient_2stops_sse41+0x27b>
  DB  102,15,111,13,56,8,0,0              ; movdqa        0x838(%rip),%xmm1        # fe0 <_sk_linear_gradient_2stops_sse41+0x28b>
  DB  102,15,219,202                      ; pand          %xmm2,%xmm1
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  15,89,13,58,8,0,0                   ; mulps         0x83a(%rip),%xmm1        # ff0 <_sk_linear_gradient_2stops_sse41+0x29b>
  DB  102,15,219,21,66,8,0,0              ; pand          0x842(%rip),%xmm2        # 1000 <_sk_linear_gradient_2stops_sse41+0x2ab>
  DB  15,91,210                           ; cvtdq2ps      %xmm2,%xmm2
  DB  15,89,21,72,8,0,0                   ; mulps         0x848(%rip),%xmm2        # 1010 <_sk_linear_gradient_2stops_sse41+0x2bb>
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,29,79,8,0,0                   ; movaps        0x84f(%rip),%xmm3        # 1020 <_sk_linear_gradient_2stops_sse41+0x2cb>
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_565_sse41
_sk_store_565_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,40,5,80,8,0,0                 ; movaps        0x850(%rip),%xmm8        # 1030 <_sk_linear_gradient_2stops_sse41+0x2db>
  DB  68,15,40,200                        ; movaps        %xmm0,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  102,65,15,114,241,11                ; pslld         $0xb,%xmm9
  DB  68,15,40,21,69,8,0,0                ; movaps        0x845(%rip),%xmm10        # 1040 <_sk_linear_gradient_2stops_sse41+0x2eb>
  DB  68,15,89,209                        ; mulps         %xmm1,%xmm10
  DB  102,69,15,91,210                    ; cvtps2dq      %xmm10,%xmm10
  DB  102,65,15,114,242,5                 ; pslld         $0x5,%xmm10
  DB  102,69,15,235,209                   ; por           %xmm9,%xmm10
  DB  68,15,89,194                        ; mulps         %xmm2,%xmm8
  DB  102,69,15,91,192                    ; cvtps2dq      %xmm8,%xmm8
  DB  102,69,15,86,194                    ; orpd          %xmm10,%xmm8
  DB  102,69,15,56,43,192                 ; packusdw      %xmm8,%xmm8
  DB  102,68,15,214,4,120                 ; movq          %xmm8,(%rax,%rdi,2)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_8888_sse41
_sk_load_8888_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,16,12,184                     ; movups        (%rax,%rdi,4),%xmm9
  DB  15,40,21,18,8,0,0                   ; movaps        0x812(%rip),%xmm2        # 1050 <_sk_linear_gradient_2stops_sse41+0x2fb>
  DB  65,15,40,193                        ; movaps        %xmm9,%xmm0
  DB  15,84,194                           ; andps         %xmm2,%xmm0
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  68,15,40,5,16,8,0,0                 ; movaps        0x810(%rip),%xmm8        # 1060 <_sk_linear_gradient_2stops_sse41+0x30b>
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,40,201                        ; movaps        %xmm9,%xmm1
  DB  102,15,114,209,8                    ; psrld         $0x8,%xmm1
  DB  102,15,219,202                      ; pand          %xmm2,%xmm1
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,40,217                        ; movaps        %xmm9,%xmm3
  DB  102,15,114,211,16                   ; psrld         $0x10,%xmm3
  DB  102,15,219,218                      ; pand          %xmm2,%xmm3
  DB  15,91,211                           ; cvtdq2ps      %xmm3,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  102,65,15,114,209,24                ; psrld         $0x18,%xmm9
  DB  65,15,91,217                        ; cvtdq2ps      %xmm9,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_8888_sse41
_sk_store_8888_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,40,5,213,7,0,0                ; movaps        0x7d5(%rip),%xmm8        # 1070 <_sk_linear_gradient_2stops_sse41+0x31b>
  DB  68,15,40,200                        ; movaps        %xmm0,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  102,69,15,91,210                    ; cvtps2dq      %xmm10,%xmm10
  DB  102,65,15,114,242,8                 ; pslld         $0x8,%xmm10
  DB  102,69,15,235,209                   ; por           %xmm9,%xmm10
  DB  68,15,40,202                        ; movaps        %xmm2,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  102,65,15,114,241,16                ; pslld         $0x10,%xmm9
  DB  68,15,89,195                        ; mulps         %xmm3,%xmm8
  DB  102,69,15,91,192                    ; cvtps2dq      %xmm8,%xmm8
  DB  102,65,15,114,240,24                ; pslld         $0x18,%xmm8
  DB  102,69,15,235,193                   ; por           %xmm9,%xmm8
  DB  102,69,15,235,194                   ; por           %xmm10,%xmm8
  DB  243,68,15,127,4,184                 ; movdqu        %xmm8,(%rax,%rdi,4)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_f16_sse41
_sk_load_f16_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,15,111,4,248                    ; movdqu        (%rax,%rdi,8),%xmm0
  DB  243,15,111,76,248,16                ; movdqu        0x10(%rax,%rdi,8),%xmm1
  DB  102,15,111,208                      ; movdqa        %xmm0,%xmm2
  DB  102,15,97,209                       ; punpcklwd     %xmm1,%xmm2
  DB  102,15,105,193                      ; punpckhwd     %xmm1,%xmm0
  DB  102,68,15,111,194                   ; movdqa        %xmm2,%xmm8
  DB  102,68,15,97,192                    ; punpcklwd     %xmm0,%xmm8
  DB  102,15,105,208                      ; punpckhwd     %xmm0,%xmm2
  DB  102,15,111,29,88,7,0,0              ; movdqa        0x758(%rip),%xmm3        # 1080 <_sk_linear_gradient_2stops_sse41+0x32b>
  DB  102,15,111,203                      ; movdqa        %xmm3,%xmm1
  DB  102,65,15,101,200                   ; pcmpgtw       %xmm8,%xmm1
  DB  102,65,15,223,200                   ; pandn         %xmm8,%xmm1
  DB  102,15,101,218                      ; pcmpgtw       %xmm2,%xmm3
  DB  102,15,223,218                      ; pandn         %xmm2,%xmm3
  DB  102,15,56,51,193                    ; pmovzxwd      %xmm1,%xmm0
  DB  102,15,114,240,13                   ; pslld         $0xd,%xmm0
  DB  68,15,40,5,64,7,0,0                 ; movaps        0x740(%rip),%xmm8        # 1090 <_sk_linear_gradient_2stops_sse41+0x33b>
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  102,69,15,239,201                   ; pxor          %xmm9,%xmm9
  DB  102,65,15,105,201                   ; punpckhwd     %xmm9,%xmm1
  DB  102,15,114,241,13                   ; pslld         $0xd,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  102,15,56,51,211                    ; pmovzxwd      %xmm3,%xmm2
  DB  102,15,114,242,13                   ; pslld         $0xd,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  102,65,15,105,217                   ; punpckhwd     %xmm9,%xmm3
  DB  102,15,114,243,13                   ; pslld         $0xd,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_f16_sse41
_sk_store_f16_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,40,5,12,7,0,0                 ; movaps        0x70c(%rip),%xmm8        # 10a0 <_sk_linear_gradient_2stops_sse41+0x34b>
  DB  68,15,40,200                        ; movaps        %xmm0,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  102,65,15,114,209,13                ; psrld         $0xd,%xmm9
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  102,65,15,114,210,13                ; psrld         $0xd,%xmm10
  DB  68,15,40,218                        ; movaps        %xmm2,%xmm11
  DB  69,15,89,216                        ; mulps         %xmm8,%xmm11
  DB  102,65,15,114,211,13                ; psrld         $0xd,%xmm11
  DB  68,15,89,195                        ; mulps         %xmm3,%xmm8
  DB  102,65,15,114,208,13                ; psrld         $0xd,%xmm8
  DB  102,65,15,115,250,2                 ; pslldq        $0x2,%xmm10
  DB  102,69,15,235,209                   ; por           %xmm9,%xmm10
  DB  102,65,15,115,248,2                 ; pslldq        $0x2,%xmm8
  DB  102,69,15,235,195                   ; por           %xmm11,%xmm8
  DB  102,69,15,111,202                   ; movdqa        %xmm10,%xmm9
  DB  102,69,15,98,200                    ; punpckldq     %xmm8,%xmm9
  DB  243,68,15,127,12,248                ; movdqu        %xmm9,(%rax,%rdi,8)
  DB  102,69,15,106,208                   ; punpckhdq     %xmm8,%xmm10
  DB  243,68,15,127,84,248,16             ; movdqu        %xmm10,0x10(%rax,%rdi,8)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_x_sse41
_sk_clamp_x_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  69,15,87,192                        ; xorps         %xmm8,%xmm8
  DB  68,15,95,192                        ; maxps         %xmm0,%xmm8
  DB  243,68,15,16,8                      ; movss         (%rax),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  102,15,118,192                      ; pcmpeqd       %xmm0,%xmm0
  DB  102,65,15,254,193                   ; paddd         %xmm9,%xmm0
  DB  68,15,93,192                        ; minps         %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,192                        ; movaps        %xmm8,%xmm0
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_y_sse41
_sk_clamp_y_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  69,15,87,192                        ; xorps         %xmm8,%xmm8
  DB  68,15,95,193                        ; maxps         %xmm1,%xmm8
  DB  243,68,15,16,8                      ; movss         (%rax),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  102,15,118,201                      ; pcmpeqd       %xmm1,%xmm1
  DB  102,65,15,254,201                   ; paddd         %xmm9,%xmm1
  DB  68,15,93,193                        ; minps         %xmm1,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,200                        ; movaps        %xmm8,%xmm1
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_repeat_x_sse41
_sk_repeat_x_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  68,15,40,200                        ; movaps        %xmm0,%xmm9
  DB  69,15,94,200                        ; divps         %xmm8,%xmm9
  DB  102,69,15,58,8,201,1                ; roundps       $0x1,%xmm9,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  65,15,92,193                        ; subps         %xmm9,%xmm0
  DB  102,69,15,118,201                   ; pcmpeqd       %xmm9,%xmm9
  DB  102,69,15,254,200                   ; paddd         %xmm8,%xmm9
  DB  65,15,93,193                        ; minps         %xmm9,%xmm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_repeat_y_sse41
_sk_repeat_y_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  68,15,40,201                        ; movaps        %xmm1,%xmm9
  DB  69,15,94,200                        ; divps         %xmm8,%xmm9
  DB  102,69,15,58,8,201,1                ; roundps       $0x1,%xmm9,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  65,15,92,201                        ; subps         %xmm9,%xmm1
  DB  102,69,15,118,201                   ; pcmpeqd       %xmm9,%xmm9
  DB  102,69,15,254,200                   ; paddd         %xmm8,%xmm9
  DB  65,15,93,201                        ; minps         %xmm9,%xmm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_mirror_x_sse41
_sk_mirror_x_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  65,15,92,193                        ; subps         %xmm9,%xmm0
  DB  243,69,15,88,192                    ; addss         %xmm8,%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  68,15,40,208                        ; movaps        %xmm0,%xmm10
  DB  69,15,94,208                        ; divps         %xmm8,%xmm10
  DB  102,69,15,58,8,210,1                ; roundps       $0x1,%xmm10,%xmm10
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  65,15,92,194                        ; subps         %xmm10,%xmm0
  DB  65,15,92,193                        ; subps         %xmm9,%xmm0
  DB  69,15,87,192                        ; xorps         %xmm8,%xmm8
  DB  68,15,92,192                        ; subps         %xmm0,%xmm8
  DB  65,15,84,192                        ; andps         %xmm8,%xmm0
  DB  102,69,15,118,192                   ; pcmpeqd       %xmm8,%xmm8
  DB  102,69,15,254,193                   ; paddd         %xmm9,%xmm8
  DB  65,15,93,192                        ; minps         %xmm8,%xmm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_mirror_y_sse41
_sk_mirror_y_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  65,15,92,201                        ; subps         %xmm9,%xmm1
  DB  243,69,15,88,192                    ; addss         %xmm8,%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  69,15,94,208                        ; divps         %xmm8,%xmm10
  DB  102,69,15,58,8,210,1                ; roundps       $0x1,%xmm10,%xmm10
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  65,15,92,202                        ; subps         %xmm10,%xmm1
  DB  65,15,92,201                        ; subps         %xmm9,%xmm1
  DB  69,15,87,192                        ; xorps         %xmm8,%xmm8
  DB  68,15,92,193                        ; subps         %xmm1,%xmm8
  DB  65,15,84,200                        ; andps         %xmm8,%xmm1
  DB  102,69,15,118,192                   ; pcmpeqd       %xmm8,%xmm8
  DB  102,69,15,254,193                   ; paddd         %xmm9,%xmm8
  DB  65,15,93,200                        ; minps         %xmm8,%xmm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_2x3_sse41
_sk_matrix_2x3_sse41 LABEL PROC
  DB  68,15,40,201                        ; movaps        %xmm1,%xmm9
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,15,16,0                         ; movss         (%rax),%xmm0
  DB  243,15,16,72,4                      ; movss         0x4(%rax),%xmm1
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  243,68,15,16,80,8                   ; movss         0x8(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,16                  ; movss         0x10(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,88,194                        ; addps         %xmm10,%xmm0
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  243,68,15,16,80,12                  ; movss         0xc(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,20                  ; movss         0x14(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,88,202                        ; addps         %xmm10,%xmm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_3x4_sse41
_sk_matrix_3x4_sse41 LABEL PROC
  DB  68,15,40,201                        ; movaps        %xmm1,%xmm9
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,15,16,0                         ; movss         (%rax),%xmm0
  DB  243,15,16,72,4                      ; movss         0x4(%rax),%xmm1
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  243,68,15,16,80,12                  ; movss         0xc(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,24                  ; movss         0x18(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  243,68,15,16,96,36                  ; movss         0x24(%rax),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  68,15,89,218                        ; mulps         %xmm2,%xmm11
  DB  69,15,88,220                        ; addps         %xmm12,%xmm11
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,88,194                        ; addps         %xmm10,%xmm0
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  243,68,15,16,80,16                  ; movss         0x10(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,28                  ; movss         0x1c(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  243,68,15,16,96,40                  ; movss         0x28(%rax),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  68,15,89,218                        ; mulps         %xmm2,%xmm11
  DB  69,15,88,220                        ; addps         %xmm12,%xmm11
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,88,202                        ; addps         %xmm10,%xmm1
  DB  243,68,15,16,80,8                   ; movss         0x8(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,20                  ; movss         0x14(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  243,68,15,16,96,32                  ; movss         0x20(%rax),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  243,68,15,16,104,44                 ; movss         0x2c(%rax),%xmm13
  DB  69,15,198,237,0                     ; shufps        $0x0,%xmm13,%xmm13
  DB  68,15,89,226                        ; mulps         %xmm2,%xmm12
  DB  69,15,88,229                        ; addps         %xmm13,%xmm12
  DB  69,15,89,217                        ; mulps         %xmm9,%xmm11
  DB  69,15,88,220                        ; addps         %xmm12,%xmm11
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,210                        ; movaps        %xmm10,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_perspective_sse41
_sk_matrix_perspective_sse41 LABEL PROC
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,15,16,0                         ; movss         (%rax),%xmm0
  DB  243,68,15,16,72,4                   ; movss         0x4(%rax),%xmm9
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  243,68,15,16,80,8                   ; movss         0x8(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  68,15,89,201                        ; mulps         %xmm1,%xmm9
  DB  69,15,88,202                        ; addps         %xmm10,%xmm9
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,88,193                        ; addps         %xmm9,%xmm0
  DB  243,68,15,16,72,12                  ; movss         0xc(%rax),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  243,68,15,16,80,16                  ; movss         0x10(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,20                  ; movss         0x14(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  68,15,89,209                        ; mulps         %xmm1,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  69,15,88,202                        ; addps         %xmm10,%xmm9
  DB  243,68,15,16,80,24                  ; movss         0x18(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,28                  ; movss         0x1c(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  243,68,15,16,96,32                  ; movss         0x20(%rax),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  68,15,89,217                        ; mulps         %xmm1,%xmm11
  DB  69,15,88,220                        ; addps         %xmm12,%xmm11
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,83,202                        ; rcpps         %xmm10,%xmm1
  DB  15,89,193                           ; mulps         %xmm1,%xmm0
  DB  68,15,89,201                        ; mulps         %xmm1,%xmm9
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,201                        ; movaps        %xmm9,%xmm1
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_linear_gradient_2stops_sse41
_sk_linear_gradient_2stops_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,16,8                          ; movups        (%rax),%xmm9
  DB  15,16,88,16                         ; movups        0x10(%rax),%xmm3
  DB  68,15,40,195                        ; movaps        %xmm3,%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,40,201                        ; movaps        %xmm9,%xmm1
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  68,15,89,192                        ; mulps         %xmm0,%xmm8
  DB  68,15,88,193                        ; addps         %xmm1,%xmm8
  DB  15,40,203                           ; movaps        %xmm3,%xmm1
  DB  15,198,201,85                       ; shufps        $0x55,%xmm1,%xmm1
  DB  65,15,40,209                        ; movaps        %xmm9,%xmm2
  DB  15,198,210,85                       ; shufps        $0x55,%xmm2,%xmm2
  DB  15,89,200                           ; mulps         %xmm0,%xmm1
  DB  15,88,202                           ; addps         %xmm2,%xmm1
  DB  15,40,211                           ; movaps        %xmm3,%xmm2
  DB  15,198,210,170                      ; shufps        $0xaa,%xmm2,%xmm2
  DB  69,15,40,209                        ; movaps        %xmm9,%xmm10
  DB  69,15,198,210,170                   ; shufps        $0xaa,%xmm10,%xmm10
  DB  15,89,208                           ; mulps         %xmm0,%xmm2
  DB  65,15,88,210                        ; addps         %xmm10,%xmm2
  DB  15,198,219,255                      ; shufps        $0xff,%xmm3,%xmm3
  DB  69,15,198,201,255                   ; shufps        $0xff,%xmm9,%xmm9
  DB  15,89,216                           ; mulps         %xmm0,%xmm3
  DB  65,15,88,217                        ; addps         %xmm9,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,192                        ; movaps        %xmm8,%xmm0
  DB  255,224                             ; jmpq          *%rax

ALIGN 16
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  64,0,0                              ; add           %al,(%rax)
  DB  64                                  ; rex
  DB  64,0,0                              ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  145                                 ; xchg          %eax,%ecx
  DB  131,158,61,145,131,158,61           ; sbbl          $0x3d,-0x617c6ec3(%rsi)
  DB  145                                 ; xchg          %eax,%ecx
  DB  131,158,61,145,131,158,61           ; sbbl          $0x3d,-0x617c6ec3(%rsi)
  DB  154                                 ; (bad)
  DB  153                                 ; cltd
  DB  153                                 ; cltd
  DB  62,154                              ; ds            (bad)
  DB  153                                 ; cltd
  DB  153                                 ; cltd
  DB  62,154                              ; ds            (bad)
  DB  153                                 ; cltd
  DB  153                                 ; cltd
  DB  62,154                              ; ds            (bad)
  DB  153                                 ; cltd
  DB  153                                 ; cltd
  DB  62,92                               ; ds            pop %rsp
  DB  143                                 ; (bad)
  DB  50,63                               ; xor           (%rdi),%bh
  DB  92                                  ; pop           %rsp
  DB  143                                 ; (bad)
  DB  50,63                               ; xor           (%rdi),%bh
  DB  92                                  ; pop           %rsp
  DB  143                                 ; (bad)
  DB  50,63                               ; xor           (%rdi),%bh
  DB  92                                  ; pop           %rsp
  DB  143                                 ; (bad)
  DB  50,63                               ; xor           (%rdi),%bh
  DB  10,215                              ; or            %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  10,215                              ; or            %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  10,215                              ; or            %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  10,215                              ; or            %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  174                                 ; scas          %es:(%rdi),%al
  DB  71,97                               ; rex.RXB       (bad)
  DB  61,174,71,97,61                     ; cmp           $0x3d6147ae,%eax
  DB  174                                 ; scas          %es:(%rdi),%al
  DB  71,97                               ; rex.RXB       (bad)
  DB  61,174,71,97,61                     ; cmp           $0x3d6147ae,%eax
  DB  41,92,71,65                         ; sub           %ebx,0x41(%rdi,%rax,2)
  DB  41,92,71,65                         ; sub           %ebx,0x41(%rdi,%rax,2)
  DB  41,92,71,65                         ; sub           %ebx,0x41(%rdi,%rax,2)
  DB  41,92,71,65                         ; sub           %ebx,0x41(%rdi,%rax,2)
  DB  206                                 ; (bad)
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  206                                 ; (bad)
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  206                                 ; (bad)
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  206                                 ; (bad)
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  168,87                              ; test          $0x57,%al
  DB  202,189,168                         ; lret          $0xa8bd
  DB  87                                  ; push          %rdi
  DB  202,189,168                         ; lret          $0xa8bd
  DB  87                                  ; push          %rdi
  DB  202,189,168                         ; lret          $0xa8bd
  DB  87                                  ; push          %rdi
  DB  202,189,194                         ; lret          $0xc2bd
  DB  135,210                             ; xchg          %edx,%edx
  DB  62,194,135,210                      ; ds            retq $0xd287
  DB  62,194,135,210                      ; ds            retq $0xd287
  DB  62,194,135,210                      ; ds            retq $0xd287
  DB  62,0,0                              ; add           %al,%ds:(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,4                            ; cmpb          $0x4,(%rdi)
  DB  231,140                             ; out           %eax,$0x8c
  DB  59,4,231                            ; cmp           (%rdi,%riz,8),%eax
  DB  140,59                              ; mov           %?,(%rbx)
  DB  4,231                               ; add           $0xe7,%al
  DB  140,59                              ; mov           %?,(%rbx)
  DB  4,231                               ; add           $0xe7,%al
  DB  140,59                              ; mov           %?,(%rbx)
  DB  129,128,128,59,129,128,128,59,129,128; addl          $0x80813b80,-0x7f7ec480(%rax)
  DB  128,59,129                          ; cmpb          $0x81,(%rbx)
  DB  128,128,59,129,128,128,59           ; addb          $0x3b,-0x7f7f7ec5(%rax)
  DB  129,128,128,59,129,128,128,59,129,128; addl          $0x80813b80,-0x7f7ec480(%rax)
  DB  128,59,0                            ; cmpb          $0x0,(%rbx)
  DB  248                                 ; clc
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  224,7                               ; loopne        f39 <.literal16+0x179>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        f3d <.literal16+0x17d>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        f41 <.literal16+0x181>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        f45 <.literal16+0x185>
  DB  0,0                                 ; add           %al,(%rax)
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  31                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,8                                 ; add           %cl,(%rax)
  DB  33,4,61,8,33,4,61                   ; and           %eax,0x3d042108(,%rdi,1)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  4,61                                ; add           $0x3d,%al
  DB  8,33                                ; or            %ah,(%rcx)
  DB  4,61                                ; add           $0x3d,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,255                          ; cmpb          $0xff,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,255                               ; add           %bh,%bh
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,255                               ; add           %bh,%bh
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,255                               ; add           %bh,%bh
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,129,128,128,59,129                ; add           %al,-0x7ec47f80(%rcx)
  DB  128,128,59,129,128,128,59           ; addb          $0x3b,-0x7f7f7ec5(%rax)
  DB  129,128,128,59,129,128,128,59,129,128; addl          $0x80813b80,-0x7f7ec480(%rax)
  DB  128,59,129                          ; cmpb          $0x81,(%rbx)
  DB  128,128,59,129,128,128,59           ; addb          $0x3b,-0x7f7f7ec5(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            ff7 <.literal16+0x237>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            ffb <.literal16+0x23b>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            fff <.literal16+0x23f>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            1003 <.literal16+0x243>
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  224,7                               ; loopne        fe9 <.literal16+0x229>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        fed <.literal16+0x22d>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        ff1 <.literal16+0x231>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        ff5 <.literal16+0x235>
  DB  0,0                                 ; add           %al,(%rax)
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  31                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,8                                 ; add           %cl,(%rax)
  DB  33,4,61,8,33,4,61                   ; and           %eax,0x3d042108(,%rdi,1)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  4,61                                ; add           $0x3d,%al
  DB  8,33                                ; or            %ah,(%rcx)
  DB  4,61                                ; add           $0x3d,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,248                               ; add           %bh,%al
  DB  65,0,0                              ; add           %al,(%r8)
  DB  248                                 ; clc
  DB  65,0,0                              ; add           %al,(%r8)
  DB  248                                 ; clc
  DB  65,0,0                              ; add           %al,(%r8)
  DB  248                                 ; clc
  DB  65,0,0                              ; add           %al,(%r8)
  DB  124,66                              ; jl            1086 <.literal16+0x2c6>
  DB  0,0                                 ; add           %al,(%rax)
  DB  124,66                              ; jl            108a <.literal16+0x2ca>
  DB  0,0                                 ; add           %al,(%rax)
  DB  124,66                              ; jl            108e <.literal16+0x2ce>
  DB  0,0                                 ; add           %al,(%rax)
  DB  124,66                              ; jl            1092 <.literal16+0x2d2>
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  129,128,128,59,129,128,128,59,129,128; addl          $0x80813b80,-0x7f7ec480(%rax)
  DB  128,59,129                          ; cmpb          $0x81,(%rbx)
  DB  128,128,59,0,0,127,67               ; addb          $0x43,0x7f00003b(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            10bb <_sk_linear_gradient_2stops_sse41+0x366>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            10bf <_sk_linear_gradient_2stops_sse41+0x36a>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            10c3 <_sk_linear_gradient_2stops_sse41+0x36e>
  DB  0,4,0                               ; add           %al,(%rax,%rax,1)
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  0,128,119,0,0,128                   ; add           %al,-0x7fffff89(%rax)
  DB  119,0                               ; ja            1099 <.literal16+0x2d9>
  DB  0,128,119,0,0,128                   ; add           %al,-0x7fffff89(%rax)
  DB  119,0                               ; ja            10a1 <.literal16+0x2e1>
  DB  0,128,7,0,0,128                     ; add           %al,-0x7ffffff9(%rax)
  DB  7                                   ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,7,0                             ; addb          $0x0,(%rdi)
  DB  0                                   ; .byte         0x0
  DB  128                                 ; .byte         0x80
  DB  7                                   ; (bad)
ALIGN 16

PUBLIC _sk_start_pipeline_sse2
_sk_start_pipeline_sse2 LABEL PROC
  DB  65,87                               ; push          %r15
  DB  65,86                               ; push          %r14
  DB  65,85                               ; push          %r13
  DB  65,84                               ; push          %r12
  DB  86                                  ; push          %rsi
  DB  87                                  ; push          %rdi
  DB  83                                  ; push          %rbx
  DB  72,129,236,160,0,0,0                ; sub           $0xa0,%rsp
  DB  68,15,41,188,36,144,0,0,0           ; movaps        %xmm15,0x90(%rsp)
  DB  68,15,41,180,36,128,0,0,0           ; movaps        %xmm14,0x80(%rsp)
  DB  68,15,41,108,36,112                 ; movaps        %xmm13,0x70(%rsp)
  DB  68,15,41,100,36,96                  ; movaps        %xmm12,0x60(%rsp)
  DB  68,15,41,92,36,80                   ; movaps        %xmm11,0x50(%rsp)
  DB  68,15,41,84,36,64                   ; movaps        %xmm10,0x40(%rsp)
  DB  68,15,41,76,36,48                   ; movaps        %xmm9,0x30(%rsp)
  DB  68,15,41,68,36,32                   ; movaps        %xmm8,0x20(%rsp)
  DB  15,41,124,36,16                     ; movaps        %xmm7,0x10(%rsp)
  DB  15,41,52,36                         ; movaps        %xmm6,(%rsp)
  DB  77,137,207                          ; mov           %r9,%r15
  DB  77,137,198                          ; mov           %r8,%r14
  DB  72,137,203                          ; mov           %rcx,%rbx
  DB  72,137,214                          ; mov           %rdx,%rsi
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  73,137,196                          ; mov           %rax,%r12
  DB  73,137,245                          ; mov           %rsi,%r13
  DB  72,141,67,4                         ; lea           0x4(%rbx),%rax
  DB  76,57,248                           ; cmp           %r15,%rax
  DB  118,5                               ; jbe           73 <_sk_start_pipeline_sse2+0x73>
  DB  72,137,216                          ; mov           %rbx,%rax
  DB  235,52                              ; jmp           a7 <_sk_start_pipeline_sse2+0xa7>
  DB  15,87,192                           ; xorps         %xmm0,%xmm0
  DB  15,87,201                           ; xorps         %xmm1,%xmm1
  DB  15,87,210                           ; xorps         %xmm2,%xmm2
  DB  15,87,219                           ; xorps         %xmm3,%xmm3
  DB  15,87,228                           ; xorps         %xmm4,%xmm4
  DB  15,87,237                           ; xorps         %xmm5,%xmm5
  DB  15,87,246                           ; xorps         %xmm6,%xmm6
  DB  15,87,255                           ; xorps         %xmm7,%xmm7
  DB  72,137,223                          ; mov           %rbx,%rdi
  DB  76,137,238                          ; mov           %r13,%rsi
  DB  76,137,242                          ; mov           %r14,%rdx
  DB  65,255,212                          ; callq         *%r12
  DB  72,141,67,4                         ; lea           0x4(%rbx),%rax
  DB  72,131,195,8                        ; add           $0x8,%rbx
  DB  76,57,251                           ; cmp           %r15,%rbx
  DB  72,137,195                          ; mov           %rax,%rbx
  DB  118,204                             ; jbe           73 <_sk_start_pipeline_sse2+0x73>
  DB  15,40,52,36                         ; movaps        (%rsp),%xmm6
  DB  15,40,124,36,16                     ; movaps        0x10(%rsp),%xmm7
  DB  68,15,40,68,36,32                   ; movaps        0x20(%rsp),%xmm8
  DB  68,15,40,76,36,48                   ; movaps        0x30(%rsp),%xmm9
  DB  68,15,40,84,36,64                   ; movaps        0x40(%rsp),%xmm10
  DB  68,15,40,92,36,80                   ; movaps        0x50(%rsp),%xmm11
  DB  68,15,40,100,36,96                  ; movaps        0x60(%rsp),%xmm12
  DB  68,15,40,108,36,112                 ; movaps        0x70(%rsp),%xmm13
  DB  68,15,40,180,36,128,0,0,0           ; movaps        0x80(%rsp),%xmm14
  DB  68,15,40,188,36,144,0,0,0           ; movaps        0x90(%rsp),%xmm15
  DB  72,129,196,160,0,0,0                ; add           $0xa0,%rsp
  DB  91                                  ; pop           %rbx
  DB  95                                  ; pop           %rdi
  DB  94                                  ; pop           %rsi
  DB  65,92                               ; pop           %r12
  DB  65,93                               ; pop           %r13
  DB  65,94                               ; pop           %r14
  DB  65,95                               ; pop           %r15
  DB  195                                 ; retq

PUBLIC _sk_just_return_sse2
_sk_just_return_sse2 LABEL PROC
  DB  195                                 ; retq

PUBLIC _sk_seed_shader_sse2
_sk_seed_shader_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  102,15,110,199                      ; movd          %edi,%xmm0
  DB  102,15,112,192,0                    ; pshufd        $0x0,%xmm0,%xmm0
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  15,40,21,49,13,0,0                  ; movaps        0xd31(%rip),%xmm2        # e40 <_sk_linear_gradient_2stops_sse2+0x75>
  DB  15,88,194                           ; addps         %xmm2,%xmm0
  DB  15,88,5,55,13,0,0                   ; addps         0xd37(%rip),%xmm0        # e50 <_sk_linear_gradient_2stops_sse2+0x85>
  DB  102,15,110,8                        ; movd          (%rax),%xmm1
  DB  102,15,112,201,0                    ; pshufd        $0x0,%xmm1,%xmm1
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  15,88,202                           ; addps         %xmm2,%xmm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,21,47,13,0,0                  ; movaps        0xd2f(%rip),%xmm2        # e60 <_sk_linear_gradient_2stops_sse2+0x95>
  DB  15,87,219                           ; xorps         %xmm3,%xmm3
  DB  15,87,228                           ; xorps         %xmm4,%xmm4
  DB  15,87,237                           ; xorps         %xmm5,%xmm5
  DB  15,87,246                           ; xorps         %xmm6,%xmm6
  DB  15,87,255                           ; xorps         %xmm7,%xmm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_constant_color_sse2
_sk_constant_color_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,16,24                            ; movups        (%rax),%xmm3
  DB  15,40,195                           ; movaps        %xmm3,%xmm0
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  15,40,203                           ; movaps        %xmm3,%xmm1
  DB  15,198,201,85                       ; shufps        $0x55,%xmm1,%xmm1
  DB  15,40,211                           ; movaps        %xmm3,%xmm2
  DB  15,198,210,170                      ; shufps        $0xaa,%xmm2,%xmm2
  DB  15,198,219,255                      ; shufps        $0xff,%xmm3,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clear_sse2
_sk_clear_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,87,192                           ; xorps         %xmm0,%xmm0
  DB  15,87,201                           ; xorps         %xmm1,%xmm1
  DB  15,87,210                           ; xorps         %xmm2,%xmm2
  DB  15,87,219                           ; xorps         %xmm3,%xmm3
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_plus__sse2
_sk_plus__sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,88,223                           ; addps         %xmm7,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_srcover_sse2
_sk_srcover_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,40,5,222,12,0,0               ; movaps        0xcde(%rip),%xmm8        # e70 <_sk_linear_gradient_2stops_sse2+0xa5>
  DB  68,15,92,195                        ; subps         %xmm3,%xmm8
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,204                        ; mulps         %xmm4,%xmm9
  DB  65,15,88,193                        ; addps         %xmm9,%xmm0
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,205                        ; mulps         %xmm5,%xmm9
  DB  65,15,88,201                        ; addps         %xmm9,%xmm1
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,206                        ; mulps         %xmm6,%xmm9
  DB  65,15,88,209                        ; addps         %xmm9,%xmm2
  DB  68,15,89,199                        ; mulps         %xmm7,%xmm8
  DB  65,15,88,216                        ; addps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_dstover_sse2
_sk_dstover_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,40,5,176,12,0,0               ; movaps        0xcb0(%rip),%xmm8        # e80 <_sk_linear_gradient_2stops_sse2+0xb5>
  DB  68,15,92,199                        ; subps         %xmm7,%xmm8
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  15,88,223                           ; addps         %xmm7,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_0_sse2
_sk_clamp_0_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  69,15,87,192                        ; xorps         %xmm8,%xmm8
  DB  65,15,95,192                        ; maxps         %xmm8,%xmm0
  DB  65,15,95,200                        ; maxps         %xmm8,%xmm1
  DB  65,15,95,208                        ; maxps         %xmm8,%xmm2
  DB  65,15,95,216                        ; maxps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_1_sse2
_sk_clamp_1_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,40,5,120,12,0,0               ; movaps        0xc78(%rip),%xmm8        # e90 <_sk_linear_gradient_2stops_sse2+0xc5>
  DB  65,15,93,192                        ; minps         %xmm8,%xmm0
  DB  65,15,93,200                        ; minps         %xmm8,%xmm1
  DB  65,15,93,208                        ; minps         %xmm8,%xmm2
  DB  65,15,93,216                        ; minps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_a_sse2
_sk_clamp_a_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,93,29,107,12,0,0                 ; minps         0xc6b(%rip),%xmm3        # ea0 <_sk_linear_gradient_2stops_sse2+0xd5>
  DB  15,93,195                           ; minps         %xmm3,%xmm0
  DB  15,93,203                           ; minps         %xmm3,%xmm1
  DB  15,93,211                           ; minps         %xmm3,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_set_rgb_sse2
_sk_set_rgb_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,15,16,0                         ; movss         (%rax),%xmm0
  DB  243,15,16,72,4                      ; movss         0x4(%rax),%xmm1
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  243,15,16,80,8                      ; movss         0x8(%rax),%xmm2
  DB  15,198,210,0                        ; shufps        $0x0,%xmm2,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_swap_rb_sse2
_sk_swap_rb_sse2 LABEL PROC
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,194                           ; movaps        %xmm2,%xmm0
  DB  65,15,40,208                        ; movaps        %xmm8,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_swap_sse2
_sk_swap_sse2 LABEL PROC
  DB  68,15,40,195                        ; movaps        %xmm3,%xmm8
  DB  68,15,40,202                        ; movaps        %xmm2,%xmm9
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  68,15,40,216                        ; movaps        %xmm0,%xmm11
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,196                           ; movaps        %xmm4,%xmm0
  DB  15,40,205                           ; movaps        %xmm5,%xmm1
  DB  15,40,214                           ; movaps        %xmm6,%xmm2
  DB  15,40,223                           ; movaps        %xmm7,%xmm3
  DB  65,15,40,227                        ; movaps        %xmm11,%xmm4
  DB  65,15,40,234                        ; movaps        %xmm10,%xmm5
  DB  65,15,40,241                        ; movaps        %xmm9,%xmm6
  DB  65,15,40,248                        ; movaps        %xmm8,%xmm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_move_src_dst_sse2
_sk_move_src_dst_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,224                           ; movaps        %xmm0,%xmm4
  DB  15,40,233                           ; movaps        %xmm1,%xmm5
  DB  15,40,242                           ; movaps        %xmm2,%xmm6
  DB  15,40,251                           ; movaps        %xmm3,%xmm7
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_move_dst_src_sse2
_sk_move_dst_src_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,196                           ; movaps        %xmm4,%xmm0
  DB  15,40,205                           ; movaps        %xmm5,%xmm1
  DB  15,40,214                           ; movaps        %xmm6,%xmm2
  DB  15,40,223                           ; movaps        %xmm7,%xmm3
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_premul_sse2
_sk_premul_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,89,195                           ; mulps         %xmm3,%xmm0
  DB  15,89,203                           ; mulps         %xmm3,%xmm1
  DB  15,89,211                           ; mulps         %xmm3,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_unpremul_sse2
_sk_unpremul_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  69,15,87,192                        ; xorps         %xmm8,%xmm8
  DB  68,15,194,195,0                     ; cmpeqps       %xmm3,%xmm8
  DB  68,15,40,13,197,11,0,0              ; movaps        0xbc5(%rip),%xmm9        # eb0 <_sk_linear_gradient_2stops_sse2+0xe5>
  DB  68,15,94,203                        ; divps         %xmm3,%xmm9
  DB  69,15,85,193                        ; andnps        %xmm9,%xmm8
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_from_srgb_sse2
_sk_from_srgb_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,40,5,179,11,0,0               ; movaps        0xbb3(%rip),%xmm8        # ec0 <_sk_linear_gradient_2stops_sse2+0xf5>
  DB  68,15,40,232                        ; movaps        %xmm0,%xmm13
  DB  69,15,89,232                        ; mulps         %xmm8,%xmm13
  DB  68,15,40,216                        ; movaps        %xmm0,%xmm11
  DB  69,15,89,219                        ; mulps         %xmm11,%xmm11
  DB  68,15,40,13,171,11,0,0              ; movaps        0xbab(%rip),%xmm9        # ed0 <_sk_linear_gradient_2stops_sse2+0x105>
  DB  68,15,40,240                        ; movaps        %xmm0,%xmm14
  DB  69,15,89,241                        ; mulps         %xmm9,%xmm14
  DB  68,15,40,21,171,11,0,0              ; movaps        0xbab(%rip),%xmm10        # ee0 <_sk_linear_gradient_2stops_sse2+0x115>
  DB  69,15,88,242                        ; addps         %xmm10,%xmm14
  DB  69,15,89,243                        ; mulps         %xmm11,%xmm14
  DB  68,15,40,29,171,11,0,0              ; movaps        0xbab(%rip),%xmm11        # ef0 <_sk_linear_gradient_2stops_sse2+0x125>
  DB  69,15,88,243                        ; addps         %xmm11,%xmm14
  DB  68,15,40,37,175,11,0,0              ; movaps        0xbaf(%rip),%xmm12        # f00 <_sk_linear_gradient_2stops_sse2+0x135>
  DB  65,15,194,196,1                     ; cmpltps       %xmm12,%xmm0
  DB  68,15,84,232                        ; andps         %xmm0,%xmm13
  DB  65,15,85,198                        ; andnps        %xmm14,%xmm0
  DB  65,15,86,197                        ; orps          %xmm13,%xmm0
  DB  68,15,40,233                        ; movaps        %xmm1,%xmm13
  DB  69,15,89,232                        ; mulps         %xmm8,%xmm13
  DB  68,15,40,241                        ; movaps        %xmm1,%xmm14
  DB  69,15,89,246                        ; mulps         %xmm14,%xmm14
  DB  68,15,40,249                        ; movaps        %xmm1,%xmm15
  DB  69,15,89,249                        ; mulps         %xmm9,%xmm15
  DB  69,15,88,250                        ; addps         %xmm10,%xmm15
  DB  69,15,89,254                        ; mulps         %xmm14,%xmm15
  DB  69,15,88,251                        ; addps         %xmm11,%xmm15
  DB  65,15,194,204,1                     ; cmpltps       %xmm12,%xmm1
  DB  68,15,84,233                        ; andps         %xmm1,%xmm13
  DB  65,15,85,207                        ; andnps        %xmm15,%xmm1
  DB  65,15,86,205                        ; orps          %xmm13,%xmm1
  DB  68,15,89,194                        ; mulps         %xmm2,%xmm8
  DB  68,15,40,234                        ; movaps        %xmm2,%xmm13
  DB  69,15,89,237                        ; mulps         %xmm13,%xmm13
  DB  68,15,89,202                        ; mulps         %xmm2,%xmm9
  DB  69,15,88,202                        ; addps         %xmm10,%xmm9
  DB  69,15,89,205                        ; mulps         %xmm13,%xmm9
  DB  69,15,88,203                        ; addps         %xmm11,%xmm9
  DB  65,15,194,212,1                     ; cmpltps       %xmm12,%xmm2
  DB  68,15,84,194                        ; andps         %xmm2,%xmm8
  DB  65,15,85,209                        ; andnps        %xmm9,%xmm2
  DB  65,15,86,208                        ; orps          %xmm8,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_to_srgb_sse2
_sk_to_srgb_sse2 LABEL PROC
  DB  68,15,82,192                        ; rsqrtps       %xmm0,%xmm8
  DB  69,15,83,200                        ; rcpps         %xmm8,%xmm9
  DB  69,15,82,232                        ; rsqrtps       %xmm8,%xmm13
  DB  68,15,40,5,52,11,0,0                ; movaps        0xb34(%rip),%xmm8        # f10 <_sk_linear_gradient_2stops_sse2+0x145>
  DB  68,15,40,240                        ; movaps        %xmm0,%xmm14
  DB  69,15,89,240                        ; mulps         %xmm8,%xmm14
  DB  68,15,40,21,52,11,0,0               ; movaps        0xb34(%rip),%xmm10        # f20 <_sk_linear_gradient_2stops_sse2+0x155>
  DB  69,15,89,202                        ; mulps         %xmm10,%xmm9
  DB  68,15,40,29,56,11,0,0               ; movaps        0xb38(%rip),%xmm11        # f30 <_sk_linear_gradient_2stops_sse2+0x165>
  DB  69,15,88,203                        ; addps         %xmm11,%xmm9
  DB  68,15,40,37,60,11,0,0               ; movaps        0xb3c(%rip),%xmm12        # f40 <_sk_linear_gradient_2stops_sse2+0x175>
  DB  69,15,89,236                        ; mulps         %xmm12,%xmm13
  DB  69,15,88,233                        ; addps         %xmm9,%xmm13
  DB  68,15,40,13,60,11,0,0               ; movaps        0xb3c(%rip),%xmm9        # f50 <_sk_linear_gradient_2stops_sse2+0x185>
  DB  69,15,40,249                        ; movaps        %xmm9,%xmm15
  DB  69,15,93,253                        ; minps         %xmm13,%xmm15
  DB  68,15,40,45,60,11,0,0               ; movaps        0xb3c(%rip),%xmm13        # f60 <_sk_linear_gradient_2stops_sse2+0x195>
  DB  65,15,194,197,1                     ; cmpltps       %xmm13,%xmm0
  DB  68,15,84,240                        ; andps         %xmm0,%xmm14
  DB  65,15,85,199                        ; andnps        %xmm15,%xmm0
  DB  65,15,86,198                        ; orps          %xmm14,%xmm0
  DB  68,15,82,241                        ; rsqrtps       %xmm1,%xmm14
  DB  69,15,83,254                        ; rcpps         %xmm14,%xmm15
  DB  69,15,82,246                        ; rsqrtps       %xmm14,%xmm14
  DB  69,15,89,250                        ; mulps         %xmm10,%xmm15
  DB  69,15,88,251                        ; addps         %xmm11,%xmm15
  DB  69,15,89,244                        ; mulps         %xmm12,%xmm14
  DB  69,15,88,247                        ; addps         %xmm15,%xmm14
  DB  69,15,40,249                        ; movaps        %xmm9,%xmm15
  DB  69,15,93,254                        ; minps         %xmm14,%xmm15
  DB  68,15,40,241                        ; movaps        %xmm1,%xmm14
  DB  69,15,89,240                        ; mulps         %xmm8,%xmm14
  DB  65,15,194,205,1                     ; cmpltps       %xmm13,%xmm1
  DB  68,15,84,241                        ; andps         %xmm1,%xmm14
  DB  65,15,85,207                        ; andnps        %xmm15,%xmm1
  DB  65,15,86,206                        ; orps          %xmm14,%xmm1
  DB  68,15,82,242                        ; rsqrtps       %xmm2,%xmm14
  DB  69,15,83,254                        ; rcpps         %xmm14,%xmm15
  DB  69,15,89,250                        ; mulps         %xmm10,%xmm15
  DB  69,15,88,251                        ; addps         %xmm11,%xmm15
  DB  69,15,82,214                        ; rsqrtps       %xmm14,%xmm10
  DB  69,15,89,212                        ; mulps         %xmm12,%xmm10
  DB  69,15,88,215                        ; addps         %xmm15,%xmm10
  DB  69,15,93,202                        ; minps         %xmm10,%xmm9
  DB  68,15,89,194                        ; mulps         %xmm2,%xmm8
  DB  65,15,194,213,1                     ; cmpltps       %xmm13,%xmm2
  DB  68,15,84,194                        ; andps         %xmm2,%xmm8
  DB  65,15,85,209                        ; andnps        %xmm9,%xmm2
  DB  65,15,86,208                        ; orps          %xmm8,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_scale_1_float_sse2
_sk_scale_1_float_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_scale_u8_sse2
_sk_scale_u8_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,68,15,110,4,56                  ; movd          (%rax,%rdi,1),%xmm8
  DB  102,69,15,239,201                   ; pxor          %xmm9,%xmm9
  DB  102,69,15,96,193                    ; punpcklbw     %xmm9,%xmm8
  DB  102,69,15,97,193                    ; punpcklwd     %xmm9,%xmm8
  DB  69,15,91,192                        ; cvtdq2ps      %xmm8,%xmm8
  DB  68,15,89,5,125,10,0,0               ; mulps         0xa7d(%rip),%xmm8        # f70 <_sk_linear_gradient_2stops_sse2+0x1a5>
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_1_float_sse2
_sk_lerp_1_float_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,92,223                           ; subps         %xmm7,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  15,88,223                           ; addps         %xmm7,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_u8_sse2
_sk_lerp_u8_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,68,15,110,4,56                  ; movd          (%rax,%rdi,1),%xmm8
  DB  102,69,15,239,201                   ; pxor          %xmm9,%xmm9
  DB  102,69,15,96,193                    ; punpcklbw     %xmm9,%xmm8
  DB  102,69,15,97,193                    ; punpcklwd     %xmm9,%xmm8
  DB  69,15,91,192                        ; cvtdq2ps      %xmm8,%xmm8
  DB  68,15,89,5,27,10,0,0                ; mulps         0xa1b(%rip),%xmm8        # f80 <_sk_linear_gradient_2stops_sse2+0x1b5>
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,92,223                           ; subps         %xmm7,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  15,88,223                           ; addps         %xmm7,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_565_sse2
_sk_lerp_565_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,68,15,126,4,120                 ; movq          (%rax,%rdi,2),%xmm8
  DB  102,15,239,219                      ; pxor          %xmm3,%xmm3
  DB  102,68,15,97,195                    ; punpcklwd     %xmm3,%xmm8
  DB  102,15,111,29,227,9,0,0             ; movdqa        0x9e3(%rip),%xmm3        # f90 <_sk_linear_gradient_2stops_sse2+0x1c5>
  DB  102,65,15,219,216                   ; pand          %xmm8,%xmm3
  DB  68,15,91,203                        ; cvtdq2ps      %xmm3,%xmm9
  DB  68,15,89,13,226,9,0,0               ; mulps         0x9e2(%rip),%xmm9        # fa0 <_sk_linear_gradient_2stops_sse2+0x1d5>
  DB  102,15,111,29,234,9,0,0             ; movdqa        0x9ea(%rip),%xmm3        # fb0 <_sk_linear_gradient_2stops_sse2+0x1e5>
  DB  102,65,15,219,216                   ; pand          %xmm8,%xmm3
  DB  15,91,219                           ; cvtdq2ps      %xmm3,%xmm3
  DB  15,89,29,235,9,0,0                  ; mulps         0x9eb(%rip),%xmm3        # fc0 <_sk_linear_gradient_2stops_sse2+0x1f5>
  DB  102,68,15,219,5,242,9,0,0           ; pand          0x9f2(%rip),%xmm8        # fd0 <_sk_linear_gradient_2stops_sse2+0x205>
  DB  69,15,91,192                        ; cvtdq2ps      %xmm8,%xmm8
  DB  68,15,89,5,246,9,0,0                ; mulps         0x9f6(%rip),%xmm8        # fe0 <_sk_linear_gradient_2stops_sse2+0x215>
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,193                        ; mulps         %xmm9,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  15,89,203                           ; mulps         %xmm3,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,29,224,9,0,0                  ; movaps        0x9e0(%rip),%xmm3        # ff0 <_sk_linear_gradient_2stops_sse2+0x225>
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_tables_sse2
_sk_load_tables_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,0                            ; mov           (%rax),%r8
  DB  76,139,80,8                         ; mov           0x8(%rax),%r10
  DB  243,69,15,111,12,184                ; movdqu        (%r8,%rdi,4),%xmm9
  DB  102,68,15,111,5,214,9,0,0           ; movdqa        0x9d6(%rip),%xmm8        # 1000 <_sk_linear_gradient_2stops_sse2+0x235>
  DB  102,65,15,111,193                   ; movdqa        %xmm9,%xmm0
  DB  102,65,15,219,192                   ; pand          %xmm8,%xmm0
  DB  102,15,112,200,78                   ; pshufd        $0x4e,%xmm0,%xmm1
  DB  102,73,15,126,200                   ; movq          %xmm1,%r8
  DB  69,137,193                          ; mov           %r8d,%r9d
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  102,72,15,126,193                   ; movq          %xmm0,%rcx
  DB  65,137,203                          ; mov           %ecx,%r11d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  243,65,15,16,12,138                 ; movss         (%r10,%rcx,4),%xmm1
  DB  243,67,15,16,4,130                  ; movss         (%r10,%r8,4),%xmm0
  DB  15,20,200                           ; unpcklps      %xmm0,%xmm1
  DB  243,67,15,16,4,154                  ; movss         (%r10,%r11,4),%xmm0
  DB  243,67,15,16,20,138                 ; movss         (%r10,%r9,4),%xmm2
  DB  15,20,194                           ; unpcklps      %xmm2,%xmm0
  DB  15,20,193                           ; unpcklps      %xmm1,%xmm0
  DB  76,139,80,16                        ; mov           0x10(%rax),%r10
  DB  102,65,15,111,201                   ; movdqa        %xmm9,%xmm1
  DB  102,15,114,209,8                    ; psrld         $0x8,%xmm1
  DB  102,65,15,219,200                   ; pand          %xmm8,%xmm1
  DB  102,15,112,209,78                   ; pshufd        $0x4e,%xmm1,%xmm2
  DB  102,73,15,126,208                   ; movq          %xmm2,%r8
  DB  69,137,193                          ; mov           %r8d,%r9d
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  102,72,15,126,201                   ; movq          %xmm1,%rcx
  DB  65,137,203                          ; mov           %ecx,%r11d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  243,65,15,16,20,138                 ; movss         (%r10,%rcx,4),%xmm2
  DB  243,67,15,16,12,130                 ; movss         (%r10,%r8,4),%xmm1
  DB  15,20,209                           ; unpcklps      %xmm1,%xmm2
  DB  243,67,15,16,12,154                 ; movss         (%r10,%r11,4),%xmm1
  DB  243,67,15,16,28,138                 ; movss         (%r10,%r9,4),%xmm3
  DB  15,20,203                           ; unpcklps      %xmm3,%xmm1
  DB  15,20,202                           ; unpcklps      %xmm2,%xmm1
  DB  76,139,72,24                        ; mov           0x18(%rax),%r9
  DB  102,65,15,111,209                   ; movdqa        %xmm9,%xmm2
  DB  102,15,114,210,16                   ; psrld         $0x10,%xmm2
  DB  102,65,15,219,208                   ; pand          %xmm8,%xmm2
  DB  102,15,112,218,78                   ; pshufd        $0x4e,%xmm2,%xmm3
  DB  102,72,15,126,217                   ; movq          %xmm3,%rcx
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  102,72,15,126,208                   ; movq          %xmm2,%rax
  DB  65,137,194                          ; mov           %eax,%r10d
  DB  72,193,232,32                       ; shr           $0x20,%rax
  DB  243,69,15,16,4,129                  ; movss         (%r9,%rax,4),%xmm8
  DB  243,65,15,16,20,137                 ; movss         (%r9,%rcx,4),%xmm2
  DB  68,15,20,194                        ; unpcklps      %xmm2,%xmm8
  DB  243,67,15,16,20,145                 ; movss         (%r9,%r10,4),%xmm2
  DB  243,67,15,16,28,129                 ; movss         (%r9,%r8,4),%xmm3
  DB  15,20,211                           ; unpcklps      %xmm3,%xmm2
  DB  65,15,20,208                        ; unpcklps      %xmm8,%xmm2
  DB  102,65,15,114,209,24                ; psrld         $0x18,%xmm9
  DB  65,15,91,217                        ; cvtdq2ps      %xmm9,%xmm3
  DB  15,89,29,233,8,0,0                  ; mulps         0x8e9(%rip),%xmm3        # 1010 <_sk_linear_gradient_2stops_sse2+0x245>
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_a8_sse2
_sk_load_a8_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,15,110,4,56                     ; movd          (%rax,%rdi,1),%xmm0
  DB  102,15,239,201                      ; pxor          %xmm1,%xmm1
  DB  102,15,96,193                       ; punpcklbw     %xmm1,%xmm0
  DB  102,15,97,193                       ; punpcklwd     %xmm1,%xmm0
  DB  15,91,216                           ; cvtdq2ps      %xmm0,%xmm3
  DB  15,89,29,213,8,0,0                  ; mulps         0x8d5(%rip),%xmm3        # 1020 <_sk_linear_gradient_2stops_sse2+0x255>
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,87,192                           ; xorps         %xmm0,%xmm0
  DB  102,15,239,201                      ; pxor          %xmm1,%xmm1
  DB  15,87,210                           ; xorps         %xmm2,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_a8_sse2
_sk_store_a8_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,40,5,202,8,0,0                ; movaps        0x8ca(%rip),%xmm8        # 1030 <_sk_linear_gradient_2stops_sse2+0x265>
  DB  68,15,89,195                        ; mulps         %xmm3,%xmm8
  DB  102,69,15,91,192                    ; cvtps2dq      %xmm8,%xmm8
  DB  102,65,15,114,240,16                ; pslld         $0x10,%xmm8
  DB  102,65,15,114,224,16                ; psrad         $0x10,%xmm8
  DB  102,69,15,107,192                   ; packssdw      %xmm8,%xmm8
  DB  102,69,15,103,192                   ; packuswb      %xmm8,%xmm8
  DB  102,68,15,126,4,56                  ; movd          %xmm8,(%rax,%rdi,1)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_565_sse2
_sk_load_565_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,15,126,20,120                   ; movq          (%rax,%rdi,2),%xmm2
  DB  102,15,239,192                      ; pxor          %xmm0,%xmm0
  DB  102,15,97,208                       ; punpcklwd     %xmm0,%xmm2
  DB  102,15,111,5,151,8,0,0              ; movdqa        0x897(%rip),%xmm0        # 1040 <_sk_linear_gradient_2stops_sse2+0x275>
  DB  102,15,219,194                      ; pand          %xmm2,%xmm0
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  15,89,5,153,8,0,0                   ; mulps         0x899(%rip),%xmm0        # 1050 <_sk_linear_gradient_2stops_sse2+0x285>
  DB  102,15,111,13,161,8,0,0             ; movdqa        0x8a1(%rip),%xmm1        # 1060 <_sk_linear_gradient_2stops_sse2+0x295>
  DB  102,15,219,202                      ; pand          %xmm2,%xmm1
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  15,89,13,163,8,0,0                  ; mulps         0x8a3(%rip),%xmm1        # 1070 <_sk_linear_gradient_2stops_sse2+0x2a5>
  DB  102,15,219,21,171,8,0,0             ; pand          0x8ab(%rip),%xmm2        # 1080 <_sk_linear_gradient_2stops_sse2+0x2b5>
  DB  15,91,210                           ; cvtdq2ps      %xmm2,%xmm2
  DB  15,89,21,177,8,0,0                  ; mulps         0x8b1(%rip),%xmm2        # 1090 <_sk_linear_gradient_2stops_sse2+0x2c5>
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,29,184,8,0,0                  ; movaps        0x8b8(%rip),%xmm3        # 10a0 <_sk_linear_gradient_2stops_sse2+0x2d5>
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_565_sse2
_sk_store_565_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,40,5,185,8,0,0                ; movaps        0x8b9(%rip),%xmm8        # 10b0 <_sk_linear_gradient_2stops_sse2+0x2e5>
  DB  68,15,40,200                        ; movaps        %xmm0,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  102,65,15,114,241,11                ; pslld         $0xb,%xmm9
  DB  68,15,40,21,174,8,0,0               ; movaps        0x8ae(%rip),%xmm10        # 10c0 <_sk_linear_gradient_2stops_sse2+0x2f5>
  DB  68,15,89,209                        ; mulps         %xmm1,%xmm10
  DB  102,69,15,91,210                    ; cvtps2dq      %xmm10,%xmm10
  DB  102,65,15,114,242,5                 ; pslld         $0x5,%xmm10
  DB  102,69,15,235,209                   ; por           %xmm9,%xmm10
  DB  68,15,89,194                        ; mulps         %xmm2,%xmm8
  DB  102,69,15,91,192                    ; cvtps2dq      %xmm8,%xmm8
  DB  102,69,15,86,194                    ; orpd          %xmm10,%xmm8
  DB  102,65,15,114,240,16                ; pslld         $0x10,%xmm8
  DB  102,65,15,114,224,16                ; psrad         $0x10,%xmm8
  DB  102,69,15,107,192                   ; packssdw      %xmm8,%xmm8
  DB  102,68,15,214,4,120                 ; movq          %xmm8,(%rax,%rdi,2)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_8888_sse2
_sk_load_8888_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,16,12,184                     ; movups        (%rax,%rdi,4),%xmm9
  DB  15,40,21,112,8,0,0                  ; movaps        0x870(%rip),%xmm2        # 10d0 <_sk_linear_gradient_2stops_sse2+0x305>
  DB  65,15,40,193                        ; movaps        %xmm9,%xmm0
  DB  15,84,194                           ; andps         %xmm2,%xmm0
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  68,15,40,5,110,8,0,0                ; movaps        0x86e(%rip),%xmm8        # 10e0 <_sk_linear_gradient_2stops_sse2+0x315>
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,40,201                        ; movaps        %xmm9,%xmm1
  DB  102,15,114,209,8                    ; psrld         $0x8,%xmm1
  DB  102,15,219,202                      ; pand          %xmm2,%xmm1
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,40,217                        ; movaps        %xmm9,%xmm3
  DB  102,15,114,211,16                   ; psrld         $0x10,%xmm3
  DB  102,15,219,218                      ; pand          %xmm2,%xmm3
  DB  15,91,211                           ; cvtdq2ps      %xmm3,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  102,65,15,114,209,24                ; psrld         $0x18,%xmm9
  DB  65,15,91,217                        ; cvtdq2ps      %xmm9,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_8888_sse2
_sk_store_8888_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,40,5,51,8,0,0                 ; movaps        0x833(%rip),%xmm8        # 10f0 <_sk_linear_gradient_2stops_sse2+0x325>
  DB  68,15,40,200                        ; movaps        %xmm0,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  102,69,15,91,210                    ; cvtps2dq      %xmm10,%xmm10
  DB  102,65,15,114,242,8                 ; pslld         $0x8,%xmm10
  DB  102,69,15,235,209                   ; por           %xmm9,%xmm10
  DB  68,15,40,202                        ; movaps        %xmm2,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  102,65,15,114,241,16                ; pslld         $0x10,%xmm9
  DB  68,15,89,195                        ; mulps         %xmm3,%xmm8
  DB  102,69,15,91,192                    ; cvtps2dq      %xmm8,%xmm8
  DB  102,65,15,114,240,24                ; pslld         $0x18,%xmm8
  DB  102,69,15,235,193                   ; por           %xmm9,%xmm8
  DB  102,69,15,235,194                   ; por           %xmm10,%xmm8
  DB  243,68,15,127,4,184                 ; movdqu        %xmm8,(%rax,%rdi,4)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_f16_sse2
_sk_load_f16_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,15,111,4,248                    ; movdqu        (%rax,%rdi,8),%xmm0
  DB  243,15,111,76,248,16                ; movdqu        0x10(%rax,%rdi,8),%xmm1
  DB  102,15,111,208                      ; movdqa        %xmm0,%xmm2
  DB  102,15,97,209                       ; punpcklwd     %xmm1,%xmm2
  DB  102,15,105,193                      ; punpckhwd     %xmm1,%xmm0
  DB  102,68,15,111,194                   ; movdqa        %xmm2,%xmm8
  DB  102,68,15,97,192                    ; punpcklwd     %xmm0,%xmm8
  DB  102,15,105,208                      ; punpckhwd     %xmm0,%xmm2
  DB  102,15,111,29,182,7,0,0             ; movdqa        0x7b6(%rip),%xmm3        # 1100 <_sk_linear_gradient_2stops_sse2+0x335>
  DB  102,15,111,203                      ; movdqa        %xmm3,%xmm1
  DB  102,65,15,101,200                   ; pcmpgtw       %xmm8,%xmm1
  DB  102,65,15,223,200                   ; pandn         %xmm8,%xmm1
  DB  102,15,101,218                      ; pcmpgtw       %xmm2,%xmm3
  DB  102,15,223,218                      ; pandn         %xmm2,%xmm3
  DB  102,69,15,239,192                   ; pxor          %xmm8,%xmm8
  DB  102,15,111,193                      ; movdqa        %xmm1,%xmm0
  DB  102,65,15,97,192                    ; punpcklwd     %xmm8,%xmm0
  DB  102,15,114,240,13                   ; pslld         $0xd,%xmm0
  DB  68,15,40,13,149,7,0,0               ; movaps        0x795(%rip),%xmm9        # 1110 <_sk_linear_gradient_2stops_sse2+0x345>
  DB  65,15,89,193                        ; mulps         %xmm9,%xmm0
  DB  102,65,15,105,200                   ; punpckhwd     %xmm8,%xmm1
  DB  102,15,114,241,13                   ; pslld         $0xd,%xmm1
  DB  65,15,89,201                        ; mulps         %xmm9,%xmm1
  DB  102,15,111,211                      ; movdqa        %xmm3,%xmm2
  DB  102,65,15,97,208                    ; punpcklwd     %xmm8,%xmm2
  DB  102,15,114,242,13                   ; pslld         $0xd,%xmm2
  DB  65,15,89,209                        ; mulps         %xmm9,%xmm2
  DB  102,65,15,105,216                   ; punpckhwd     %xmm8,%xmm3
  DB  102,15,114,243,13                   ; pslld         $0xd,%xmm3
  DB  65,15,89,217                        ; mulps         %xmm9,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_f16_sse2
_sk_store_f16_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  68,15,40,5,98,7,0,0                 ; movaps        0x762(%rip),%xmm8        # 1120 <_sk_linear_gradient_2stops_sse2+0x355>
  DB  68,15,40,200                        ; movaps        %xmm0,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  102,65,15,114,209,13                ; psrld         $0xd,%xmm9
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  102,65,15,114,210,13                ; psrld         $0xd,%xmm10
  DB  68,15,40,218                        ; movaps        %xmm2,%xmm11
  DB  69,15,89,216                        ; mulps         %xmm8,%xmm11
  DB  102,65,15,114,211,13                ; psrld         $0xd,%xmm11
  DB  68,15,89,195                        ; mulps         %xmm3,%xmm8
  DB  102,65,15,114,208,13                ; psrld         $0xd,%xmm8
  DB  102,65,15,115,250,2                 ; pslldq        $0x2,%xmm10
  DB  102,69,15,235,209                   ; por           %xmm9,%xmm10
  DB  102,65,15,115,248,2                 ; pslldq        $0x2,%xmm8
  DB  102,69,15,235,195                   ; por           %xmm11,%xmm8
  DB  102,69,15,111,202                   ; movdqa        %xmm10,%xmm9
  DB  102,69,15,98,200                    ; punpckldq     %xmm8,%xmm9
  DB  243,68,15,127,12,248                ; movdqu        %xmm9,(%rax,%rdi,8)
  DB  102,69,15,106,208                   ; punpckhdq     %xmm8,%xmm10
  DB  243,68,15,127,84,248,16             ; movdqu        %xmm10,0x10(%rax,%rdi,8)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_x_sse2
_sk_clamp_x_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  69,15,87,192                        ; xorps         %xmm8,%xmm8
  DB  68,15,95,192                        ; maxps         %xmm0,%xmm8
  DB  243,68,15,16,8                      ; movss         (%rax),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  102,15,118,192                      ; pcmpeqd       %xmm0,%xmm0
  DB  102,65,15,254,193                   ; paddd         %xmm9,%xmm0
  DB  68,15,93,192                        ; minps         %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,192                        ; movaps        %xmm8,%xmm0
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_y_sse2
_sk_clamp_y_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  69,15,87,192                        ; xorps         %xmm8,%xmm8
  DB  68,15,95,193                        ; maxps         %xmm1,%xmm8
  DB  243,68,15,16,8                      ; movss         (%rax),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  102,15,118,201                      ; pcmpeqd       %xmm1,%xmm1
  DB  102,65,15,254,201                   ; paddd         %xmm9,%xmm1
  DB  68,15,93,193                        ; minps         %xmm1,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,200                        ; movaps        %xmm8,%xmm1
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_repeat_x_sse2
_sk_repeat_x_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  68,15,40,200                        ; movaps        %xmm0,%xmm9
  DB  69,15,94,200                        ; divps         %xmm8,%xmm9
  DB  243,69,15,91,209                    ; cvttps2dq     %xmm9,%xmm10
  DB  69,15,91,210                        ; cvtdq2ps      %xmm10,%xmm10
  DB  69,15,194,202,1                     ; cmpltps       %xmm10,%xmm9
  DB  68,15,84,13,140,6,0,0               ; andps         0x68c(%rip),%xmm9        # 1130 <_sk_linear_gradient_2stops_sse2+0x365>
  DB  69,15,92,209                        ; subps         %xmm9,%xmm10
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  65,15,92,194                        ; subps         %xmm10,%xmm0
  DB  102,69,15,118,201                   ; pcmpeqd       %xmm9,%xmm9
  DB  102,69,15,254,200                   ; paddd         %xmm8,%xmm9
  DB  65,15,93,193                        ; minps         %xmm9,%xmm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_repeat_y_sse2
_sk_repeat_y_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  68,15,40,201                        ; movaps        %xmm1,%xmm9
  DB  69,15,94,200                        ; divps         %xmm8,%xmm9
  DB  243,69,15,91,209                    ; cvttps2dq     %xmm9,%xmm10
  DB  69,15,91,210                        ; cvtdq2ps      %xmm10,%xmm10
  DB  69,15,194,202,1                     ; cmpltps       %xmm10,%xmm9
  DB  68,15,84,13,84,6,0,0                ; andps         0x654(%rip),%xmm9        # 1140 <_sk_linear_gradient_2stops_sse2+0x375>
  DB  69,15,92,209                        ; subps         %xmm9,%xmm10
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  65,15,92,202                        ; subps         %xmm10,%xmm1
  DB  102,69,15,118,201                   ; pcmpeqd       %xmm9,%xmm9
  DB  102,69,15,254,200                   ; paddd         %xmm8,%xmm9
  DB  65,15,93,201                        ; minps         %xmm9,%xmm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_mirror_x_sse2
_sk_mirror_x_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  65,15,92,193                        ; subps         %xmm9,%xmm0
  DB  243,69,15,88,192                    ; addss         %xmm8,%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  68,15,40,208                        ; movaps        %xmm0,%xmm10
  DB  69,15,94,208                        ; divps         %xmm8,%xmm10
  DB  243,69,15,91,218                    ; cvttps2dq     %xmm10,%xmm11
  DB  69,15,91,219                        ; cvtdq2ps      %xmm11,%xmm11
  DB  69,15,194,211,1                     ; cmpltps       %xmm11,%xmm10
  DB  68,15,84,21,10,6,0,0                ; andps         0x60a(%rip),%xmm10        # 1150 <_sk_linear_gradient_2stops_sse2+0x385>
  DB  69,15,87,228                        ; xorps         %xmm12,%xmm12
  DB  69,15,92,218                        ; subps         %xmm10,%xmm11
  DB  69,15,89,216                        ; mulps         %xmm8,%xmm11
  DB  65,15,92,195                        ; subps         %xmm11,%xmm0
  DB  65,15,92,193                        ; subps         %xmm9,%xmm0
  DB  68,15,92,224                        ; subps         %xmm0,%xmm12
  DB  65,15,84,196                        ; andps         %xmm12,%xmm0
  DB  102,69,15,118,192                   ; pcmpeqd       %xmm8,%xmm8
  DB  102,69,15,254,193                   ; paddd         %xmm9,%xmm8
  DB  65,15,93,192                        ; minps         %xmm8,%xmm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_mirror_y_sse2
_sk_mirror_y_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,0                      ; movss         (%rax),%xmm8
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  65,15,92,201                        ; subps         %xmm9,%xmm1
  DB  243,69,15,88,192                    ; addss         %xmm8,%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  69,15,94,208                        ; divps         %xmm8,%xmm10
  DB  243,69,15,91,218                    ; cvttps2dq     %xmm10,%xmm11
  DB  69,15,91,219                        ; cvtdq2ps      %xmm11,%xmm11
  DB  69,15,194,211,1                     ; cmpltps       %xmm11,%xmm10
  DB  68,15,84,21,176,5,0,0               ; andps         0x5b0(%rip),%xmm10        # 1160 <_sk_linear_gradient_2stops_sse2+0x395>
  DB  69,15,87,228                        ; xorps         %xmm12,%xmm12
  DB  69,15,92,218                        ; subps         %xmm10,%xmm11
  DB  69,15,89,216                        ; mulps         %xmm8,%xmm11
  DB  65,15,92,203                        ; subps         %xmm11,%xmm1
  DB  65,15,92,201                        ; subps         %xmm9,%xmm1
  DB  68,15,92,225                        ; subps         %xmm1,%xmm12
  DB  65,15,84,204                        ; andps         %xmm12,%xmm1
  DB  102,69,15,118,192                   ; pcmpeqd       %xmm8,%xmm8
  DB  102,69,15,254,193                   ; paddd         %xmm9,%xmm8
  DB  65,15,93,200                        ; minps         %xmm8,%xmm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_2x3_sse2
_sk_matrix_2x3_sse2 LABEL PROC
  DB  68,15,40,201                        ; movaps        %xmm1,%xmm9
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,15,16,0                         ; movss         (%rax),%xmm0
  DB  243,15,16,72,4                      ; movss         0x4(%rax),%xmm1
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  243,68,15,16,80,8                   ; movss         0x8(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,16                  ; movss         0x10(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,88,194                        ; addps         %xmm10,%xmm0
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  243,68,15,16,80,12                  ; movss         0xc(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,20                  ; movss         0x14(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,88,202                        ; addps         %xmm10,%xmm1
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_3x4_sse2
_sk_matrix_3x4_sse2 LABEL PROC
  DB  68,15,40,201                        ; movaps        %xmm1,%xmm9
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,15,16,0                         ; movss         (%rax),%xmm0
  DB  243,15,16,72,4                      ; movss         0x4(%rax),%xmm1
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  243,68,15,16,80,12                  ; movss         0xc(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,24                  ; movss         0x18(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  243,68,15,16,96,36                  ; movss         0x24(%rax),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  68,15,89,218                        ; mulps         %xmm2,%xmm11
  DB  69,15,88,220                        ; addps         %xmm12,%xmm11
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,88,194                        ; addps         %xmm10,%xmm0
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  243,68,15,16,80,16                  ; movss         0x10(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,28                  ; movss         0x1c(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  243,68,15,16,96,40                  ; movss         0x28(%rax),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  68,15,89,218                        ; mulps         %xmm2,%xmm11
  DB  69,15,88,220                        ; addps         %xmm12,%xmm11
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  65,15,88,202                        ; addps         %xmm10,%xmm1
  DB  243,68,15,16,80,8                   ; movss         0x8(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,20                  ; movss         0x14(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  243,68,15,16,96,32                  ; movss         0x20(%rax),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  243,68,15,16,104,44                 ; movss         0x2c(%rax),%xmm13
  DB  69,15,198,237,0                     ; shufps        $0x0,%xmm13,%xmm13
  DB  68,15,89,226                        ; mulps         %xmm2,%xmm12
  DB  69,15,88,229                        ; addps         %xmm13,%xmm12
  DB  69,15,89,217                        ; mulps         %xmm9,%xmm11
  DB  69,15,88,220                        ; addps         %xmm12,%xmm11
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,210                        ; movaps        %xmm10,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_matrix_perspective_sse2
_sk_matrix_perspective_sse2 LABEL PROC
  DB  68,15,40,192                        ; movaps        %xmm0,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,15,16,0                         ; movss         (%rax),%xmm0
  DB  243,68,15,16,72,4                   ; movss         0x4(%rax),%xmm9
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  243,68,15,16,80,8                   ; movss         0x8(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  68,15,89,201                        ; mulps         %xmm1,%xmm9
  DB  69,15,88,202                        ; addps         %xmm10,%xmm9
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  65,15,88,193                        ; addps         %xmm9,%xmm0
  DB  243,68,15,16,72,12                  ; movss         0xc(%rax),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  243,68,15,16,80,16                  ; movss         0x10(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,20                  ; movss         0x14(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  68,15,89,209                        ; mulps         %xmm1,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  69,15,88,202                        ; addps         %xmm10,%xmm9
  DB  243,68,15,16,80,24                  ; movss         0x18(%rax),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,88,28                  ; movss         0x1c(%rax),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  243,68,15,16,96,32                  ; movss         0x20(%rax),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  68,15,89,217                        ; mulps         %xmm1,%xmm11
  DB  69,15,88,220                        ; addps         %xmm12,%xmm11
  DB  69,15,89,208                        ; mulps         %xmm8,%xmm10
  DB  69,15,88,211                        ; addps         %xmm11,%xmm10
  DB  65,15,83,202                        ; rcpps         %xmm10,%xmm1
  DB  15,89,193                           ; mulps         %xmm1,%xmm0
  DB  68,15,89,201                        ; mulps         %xmm1,%xmm9
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,201                        ; movaps        %xmm9,%xmm1
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_linear_gradient_2stops_sse2
_sk_linear_gradient_2stops_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  68,15,16,8                          ; movups        (%rax),%xmm9
  DB  15,16,88,16                         ; movups        0x10(%rax),%xmm3
  DB  68,15,40,195                        ; movaps        %xmm3,%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,40,201                        ; movaps        %xmm9,%xmm1
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  68,15,89,192                        ; mulps         %xmm0,%xmm8
  DB  68,15,88,193                        ; addps         %xmm1,%xmm8
  DB  15,40,203                           ; movaps        %xmm3,%xmm1
  DB  15,198,201,85                       ; shufps        $0x55,%xmm1,%xmm1
  DB  65,15,40,209                        ; movaps        %xmm9,%xmm2
  DB  15,198,210,85                       ; shufps        $0x55,%xmm2,%xmm2
  DB  15,89,200                           ; mulps         %xmm0,%xmm1
  DB  15,88,202                           ; addps         %xmm2,%xmm1
  DB  15,40,211                           ; movaps        %xmm3,%xmm2
  DB  15,198,210,170                      ; shufps        $0xaa,%xmm2,%xmm2
  DB  69,15,40,209                        ; movaps        %xmm9,%xmm10
  DB  69,15,198,210,170                   ; shufps        $0xaa,%xmm10,%xmm10
  DB  15,89,208                           ; mulps         %xmm0,%xmm2
  DB  65,15,88,210                        ; addps         %xmm10,%xmm2
  DB  15,198,219,255                      ; shufps        $0xff,%xmm3,%xmm3
  DB  69,15,198,201,255                   ; shufps        $0xff,%xmm9,%xmm9
  DB  15,89,216                           ; mulps         %xmm0,%xmm3
  DB  65,15,88,217                        ; addps         %xmm9,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,192                        ; movaps        %xmm8,%xmm0
  DB  255,224                             ; jmpq          *%rax

ALIGN 16
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,63                                ; add           %bh,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  64,0,0                              ; add           %al,(%rax)
  DB  64                                  ; rex
  DB  64,0,0                              ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  145                                 ; xchg          %eax,%ecx
  DB  131,158,61,145,131,158,61           ; sbbl          $0x3d,-0x617c6ec3(%rsi)
  DB  145                                 ; xchg          %eax,%ecx
  DB  131,158,61,145,131,158,61           ; sbbl          $0x3d,-0x617c6ec3(%rsi)
  DB  154                                 ; (bad)
  DB  153                                 ; cltd
  DB  153                                 ; cltd
  DB  62,154                              ; ds            (bad)
  DB  153                                 ; cltd
  DB  153                                 ; cltd
  DB  62,154                              ; ds            (bad)
  DB  153                                 ; cltd
  DB  153                                 ; cltd
  DB  62,154                              ; ds            (bad)
  DB  153                                 ; cltd
  DB  153                                 ; cltd
  DB  62,92                               ; ds            pop %rsp
  DB  143                                 ; (bad)
  DB  50,63                               ; xor           (%rdi),%bh
  DB  92                                  ; pop           %rsp
  DB  143                                 ; (bad)
  DB  50,63                               ; xor           (%rdi),%bh
  DB  92                                  ; pop           %rsp
  DB  143                                 ; (bad)
  DB  50,63                               ; xor           (%rdi),%bh
  DB  92                                  ; pop           %rsp
  DB  143                                 ; (bad)
  DB  50,63                               ; xor           (%rdi),%bh
  DB  10,215                              ; or            %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  10,215                              ; or            %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  10,215                              ; or            %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  10,215                              ; or            %bh,%dl
  DB  35,59                               ; and           (%rbx),%edi
  DB  174                                 ; scas          %es:(%rdi),%al
  DB  71,97                               ; rex.RXB       (bad)
  DB  61,174,71,97,61                     ; cmp           $0x3d6147ae,%eax
  DB  174                                 ; scas          %es:(%rdi),%al
  DB  71,97                               ; rex.RXB       (bad)
  DB  61,174,71,97,61                     ; cmp           $0x3d6147ae,%eax
  DB  41,92,71,65                         ; sub           %ebx,0x41(%rdi,%rax,2)
  DB  41,92,71,65                         ; sub           %ebx,0x41(%rdi,%rax,2)
  DB  41,92,71,65                         ; sub           %ebx,0x41(%rdi,%rax,2)
  DB  41,92,71,65                         ; sub           %ebx,0x41(%rdi,%rax,2)
  DB  206                                 ; (bad)
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  206                                 ; (bad)
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  206                                 ; (bad)
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  206                                 ; (bad)
  DB  111                                 ; outsl         %ds:(%rsi),(%dx)
  DB  48,63                               ; xor           %bh,(%rdi)
  DB  168,87                              ; test          $0x57,%al
  DB  202,189,168                         ; lret          $0xa8bd
  DB  87                                  ; push          %rdi
  DB  202,189,168                         ; lret          $0xa8bd
  DB  87                                  ; push          %rdi
  DB  202,189,168                         ; lret          $0xa8bd
  DB  87                                  ; push          %rdi
  DB  202,189,194                         ; lret          $0xc2bd
  DB  135,210                             ; xchg          %edx,%edx
  DB  62,194,135,210                      ; ds            retq $0xd287
  DB  62,194,135,210                      ; ds            retq $0xd287
  DB  62,194,135,210                      ; ds            retq $0xd287
  DB  62,0,0                              ; add           %al,%ds:(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,4                            ; cmpb          $0x4,(%rdi)
  DB  231,140                             ; out           %eax,$0x8c
  DB  59,4,231                            ; cmp           (%rdi,%riz,8),%eax
  DB  140,59                              ; mov           %?,(%rbx)
  DB  4,231                               ; add           $0xe7,%al
  DB  140,59                              ; mov           %?,(%rbx)
  DB  4,231                               ; add           $0xe7,%al
  DB  140,59                              ; mov           %?,(%rbx)
  DB  129,128,128,59,129,128,128,59,129,128; addl          $0x80813b80,-0x7f7ec480(%rax)
  DB  128,59,129                          ; cmpb          $0x81,(%rbx)
  DB  128,128,59,129,128,128,59           ; addb          $0x3b,-0x7f7f7ec5(%rax)
  DB  129,128,128,59,129,128,128,59,129,128; addl          $0x80813b80,-0x7f7ec480(%rax)
  DB  128,59,0                            ; cmpb          $0x0,(%rbx)
  DB  248                                 ; clc
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  224,7                               ; loopne        fb9 <.literal16+0x179>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        fbd <.literal16+0x17d>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        fc1 <.literal16+0x181>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        fc5 <.literal16+0x185>
  DB  0,0                                 ; add           %al,(%rax)
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  31                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,8                                 ; add           %cl,(%rax)
  DB  33,4,61,8,33,4,61                   ; and           %eax,0x3d042108(,%rdi,1)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  4,61                                ; add           $0x3d,%al
  DB  8,33                                ; or            %ah,(%rcx)
  DB  4,61                                ; add           $0x3d,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,255                          ; cmpb          $0xff,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,255                               ; add           %bh,%bh
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,255                               ; add           %bh,%bh
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,255                               ; add           %bh,%bh
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,129,128,128,59,129                ; add           %al,-0x7ec47f80(%rcx)
  DB  128,128,59,129,128,128,59           ; addb          $0x3b,-0x7f7f7ec5(%rax)
  DB  129,128,128,59,129,128,128,59,129,128; addl          $0x80813b80,-0x7f7ec480(%rax)
  DB  128,59,129                          ; cmpb          $0x81,(%rbx)
  DB  128,128,59,129,128,128,59           ; addb          $0x3b,-0x7f7f7ec5(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            1077 <.literal16+0x237>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            107b <.literal16+0x23b>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            107f <.literal16+0x23f>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            1083 <.literal16+0x243>
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,248                               ; add           %bh,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  132,55                              ; test          %dh,(%rdi)
  DB  224,7                               ; loopne        1069 <.literal16+0x229>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        106d <.literal16+0x22d>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        1071 <.literal16+0x231>
  DB  0,0                                 ; add           %al,(%rax)
  DB  224,7                               ; loopne        1075 <.literal16+0x235>
  DB  0,0                                 ; add           %al,(%rax)
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  33,8                                ; and           %ecx,(%rax)
  DB  2,58                                ; add           (%rdx),%bh
  DB  31                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,31                                ; add           %bl,(%rdi)
  DB  0,0                                 ; add           %al,(%rax)
  DB  0,8                                 ; add           %cl,(%rax)
  DB  33,4,61,8,33,4,61                   ; and           %eax,0x3d042108(,%rdi,1)
  DB  8,33                                ; or            %ah,(%rcx)
  DB  4,61                                ; add           $0x3d,%al
  DB  8,33                                ; or            %ah,(%rcx)
  DB  4,61                                ; add           $0x3d,%al
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,248                               ; add           %bh,%al
  DB  65,0,0                              ; add           %al,(%r8)
  DB  248                                 ; clc
  DB  65,0,0                              ; add           %al,(%r8)
  DB  248                                 ; clc
  DB  65,0,0                              ; add           %al,(%r8)
  DB  248                                 ; clc
  DB  65,0,0                              ; add           %al,(%r8)
  DB  124,66                              ; jl            1106 <.literal16+0x2c6>
  DB  0,0                                 ; add           %al,(%rax)
  DB  124,66                              ; jl            110a <.literal16+0x2ca>
  DB  0,0                                 ; add           %al,(%rax)
  DB  124,66                              ; jl            110e <.literal16+0x2ce>
  DB  0,0                                 ; add           %al,(%rax)
  DB  124,66                              ; jl            1112 <.literal16+0x2d2>
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  255,0                               ; incl          (%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  129,128,128,59,129,128,128,59,129,128; addl          $0x80813b80,-0x7f7ec480(%rax)
  DB  128,59,129                          ; cmpb          $0x81,(%rbx)
  DB  128,128,59,0,0,127,67               ; addb          $0x43,0x7f00003b(%rax)
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            113b <.literal16+0x2fb>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            113f <.literal16+0x2ff>
  DB  0,0                                 ; add           %al,(%rax)
  DB  127,67                              ; jg            1143 <.literal16+0x303>
  DB  0,4,0                               ; add           %al,(%rax,%rax,1)
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  4,0                                 ; add           $0x0,%al
  DB  0,128,119,0,0,128                   ; add           %al,-0x7fffff89(%rax)
  DB  119,0                               ; ja            1119 <.literal16+0x2d9>
  DB  0,128,119,0,0,128                   ; add           %al,-0x7fffff89(%rax)
  DB  119,0                               ; ja            1121 <.literal16+0x2e1>
  DB  0,128,7,0,0,128                     ; add           %al,-0x7ffffff9(%rax)
  DB  7                                   ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,7,0                             ; addb          $0x0,(%rdi)
  DB  0,128,7,0,0,128                     ; add           %al,-0x7ffffff9(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
  DB  0,0                                 ; add           %al,(%rax)
  DB  128,63,0                            ; cmpb          $0x0,(%rdi)
  DB  0,128,63,0,0,128                    ; add           %al,-0x7fffffc1(%rax)
  DB  63                                  ; (bad)
END
