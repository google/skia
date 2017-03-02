; Copyright 2017 Google Inc.
;
; Use of this source code is governed by a BSD-style license that can be
; found in the LICENSE file.

; This file is generated semi-automatically with this command:
;   $ src/jumper/build_stages.py

_text SEGMENT

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
  DB  77,137,205                          ; mov           %r9,%r13
  DB  77,137,198                          ; mov           %r8,%r14
  DB  72,137,203                          ; mov           %rcx,%rbx
  DB  72,137,214                          ; mov           %rdx,%rsi
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  73,137,199                          ; mov           %rax,%r15
  DB  73,137,244                          ; mov           %rsi,%r12
  DB  72,141,67,8                         ; lea           0x8(%rbx),%rax
  DB  76,57,232                           ; cmp           %r13,%rax
  DB  118,5                               ; jbe           75 <_sk_start_pipeline_hsw+0x75>
  DB  72,137,223                          ; mov           %rbx,%rdi
  DB  235,65                              ; jmp           b6 <_sk_start_pipeline_hsw+0xb6>
  DB  185,0,0,0,0                         ; mov           $0x0,%ecx
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  197,220,87,228                      ; vxorps        %ymm4,%ymm4,%ymm4
  DB  197,212,87,237                      ; vxorps        %ymm5,%ymm5,%ymm5
  DB  197,204,87,246                      ; vxorps        %ymm6,%ymm6,%ymm6
  DB  197,196,87,255                      ; vxorps        %ymm7,%ymm7,%ymm7
  DB  72,137,223                          ; mov           %rbx,%rdi
  DB  76,137,230                          ; mov           %r12,%rsi
  DB  76,137,242                          ; mov           %r14,%rdx
  DB  65,255,215                          ; callq         *%r15
  DB  72,141,123,8                        ; lea           0x8(%rbx),%rdi
  DB  72,131,195,16                       ; add           $0x10,%rbx
  DB  76,57,235                           ; cmp           %r13,%rbx
  DB  72,137,251                          ; mov           %rdi,%rbx
  DB  118,191                             ; jbe           75 <_sk_start_pipeline_hsw+0x75>
  DB  76,137,233                          ; mov           %r13,%rcx
  DB  72,41,249                           ; sub           %rdi,%rcx
  DB  116,41                              ; je            e7 <_sk_start_pipeline_hsw+0xe7>
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  197,220,87,228                      ; vxorps        %ymm4,%ymm4,%ymm4
  DB  197,212,87,237                      ; vxorps        %ymm5,%ymm5,%ymm5
  DB  197,204,87,246                      ; vxorps        %ymm6,%ymm6,%ymm6
  DB  197,196,87,255                      ; vxorps        %ymm7,%ymm7,%ymm7
  DB  76,137,230                          ; mov           %r12,%rsi
  DB  76,137,242                          ; mov           %r14,%rdx
  DB  65,255,215                          ; callq         *%r15
  DB  76,137,232                          ; mov           %r13,%rax
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
  DB  196,226,125,24,74,4                 ; vbroadcastss  0x4(%rdx),%ymm1
  DB  197,252,88,193                      ; vaddps        %ymm1,%ymm0,%ymm0
  DB  197,252,88,66,20                    ; vaddps        0x14(%rdx),%ymm0,%ymm0
  DB  196,226,125,24,16                   ; vbroadcastss  (%rax),%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  197,236,88,201                      ; vaddps        %ymm1,%ymm2,%ymm1
  DB  196,226,125,24,18                   ; vbroadcastss  (%rdx),%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
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
  DB  196,98,125,24,2                     ; vbroadcastss  (%rdx),%ymm8
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
  DB  196,98,125,24,2                     ; vbroadcastss  (%rdx),%ymm8
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
  DB  196,98,125,24,2                     ; vbroadcastss  (%rdx),%ymm8
  DB  196,193,124,93,192                  ; vminps        %ymm8,%ymm0,%ymm0
  DB  196,193,116,93,200                  ; vminps        %ymm8,%ymm1,%ymm1
  DB  196,193,108,93,208                  ; vminps        %ymm8,%ymm2,%ymm2
  DB  196,193,100,93,216                  ; vminps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_a_hsw
_sk_clamp_a_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,2                     ; vbroadcastss  (%rdx),%ymm8
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
  DB  196,98,125,24,18                    ; vbroadcastss  (%rdx),%ymm10
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
  DB  196,98,125,24,66,64                 ; vbroadcastss  0x40(%rdx),%ymm8
  DB  197,60,89,200                       ; vmulps        %ymm0,%ymm8,%ymm9
  DB  197,124,89,208                      ; vmulps        %ymm0,%ymm0,%ymm10
  DB  196,98,125,24,90,60                 ; vbroadcastss  0x3c(%rdx),%ymm11
  DB  196,98,125,24,98,56                 ; vbroadcastss  0x38(%rdx),%ymm12
  DB  196,65,124,40,235                   ; vmovaps       %ymm11,%ymm13
  DB  196,66,125,168,236                  ; vfmadd213ps   %ymm12,%ymm0,%ymm13
  DB  196,98,125,24,114,52                ; vbroadcastss  0x34(%rdx),%ymm14
  DB  196,66,45,168,238                   ; vfmadd213ps   %ymm14,%ymm10,%ymm13
  DB  196,98,125,24,82,68                 ; vbroadcastss  0x44(%rdx),%ymm10
  DB  196,193,124,194,194,1               ; vcmpltps      %ymm10,%ymm0,%ymm0
  DB  196,195,21,74,193,0                 ; vblendvps     %ymm0,%ymm9,%ymm13,%ymm0
  DB  197,60,89,201                       ; vmulps        %ymm1,%ymm8,%ymm9
  DB  197,116,89,233                      ; vmulps        %ymm1,%ymm1,%ymm13
  DB  196,65,124,40,251                   ; vmovaps       %ymm11,%ymm15
  DB  196,66,117,168,252                  ; vfmadd213ps   %ymm12,%ymm1,%ymm15
  DB  196,66,21,168,254                   ; vfmadd213ps   %ymm14,%ymm13,%ymm15
  DB  196,193,116,194,202,1               ; vcmpltps      %ymm10,%ymm1,%ymm1
  DB  196,195,5,74,201,16                 ; vblendvps     %ymm1,%ymm9,%ymm15,%ymm1
  DB  197,60,89,194                       ; vmulps        %ymm2,%ymm8,%ymm8
  DB  197,108,89,202                      ; vmulps        %ymm2,%ymm2,%ymm9
  DB  196,66,109,168,220                  ; vfmadd213ps   %ymm12,%ymm2,%ymm11
  DB  196,66,53,168,222                   ; vfmadd213ps   %ymm14,%ymm9,%ymm11
  DB  196,193,108,194,210,1               ; vcmpltps      %ymm10,%ymm2,%ymm2
  DB  196,195,37,74,208,32                ; vblendvps     %ymm2,%ymm8,%ymm11,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_to_srgb_hsw
_sk_to_srgb_hsw LABEL PROC
  DB  197,124,82,192                      ; vrsqrtps      %ymm0,%ymm8
  DB  196,65,124,83,200                   ; vrcpps        %ymm8,%ymm9
  DB  196,65,124,82,208                   ; vrsqrtps      %ymm8,%ymm10
  DB  196,98,125,24,66,72                 ; vbroadcastss  0x48(%rdx),%ymm8
  DB  197,60,89,216                       ; vmulps        %ymm0,%ymm8,%ymm11
  DB  196,98,125,24,34                    ; vbroadcastss  (%rdx),%ymm12
  DB  196,98,125,24,106,76                ; vbroadcastss  0x4c(%rdx),%ymm13
  DB  196,98,125,24,114,80                ; vbroadcastss  0x50(%rdx),%ymm14
  DB  196,98,125,24,122,84                ; vbroadcastss  0x54(%rdx),%ymm15
  DB  196,66,13,168,207                   ; vfmadd213ps   %ymm15,%ymm14,%ymm9
  DB  196,66,21,184,202                   ; vfmadd231ps   %ymm10,%ymm13,%ymm9
  DB  196,65,28,93,201                    ; vminps        %ymm9,%ymm12,%ymm9
  DB  196,98,125,24,82,88                 ; vbroadcastss  0x58(%rdx),%ymm10
  DB  196,193,124,194,194,1               ; vcmpltps      %ymm10,%ymm0,%ymm0
  DB  196,195,53,74,195,0                 ; vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  DB  197,124,82,201                      ; vrsqrtps      %ymm1,%ymm9
  DB  196,65,124,83,217                   ; vrcpps        %ymm9,%ymm11
  DB  196,65,124,82,201                   ; vrsqrtps      %ymm9,%ymm9
  DB  196,66,13,168,223                   ; vfmadd213ps   %ymm15,%ymm14,%ymm11
  DB  196,66,21,184,217                   ; vfmadd231ps   %ymm9,%ymm13,%ymm11
  DB  197,60,89,201                       ; vmulps        %ymm1,%ymm8,%ymm9
  DB  196,65,28,93,219                    ; vminps        %ymm11,%ymm12,%ymm11
  DB  196,193,116,194,202,1               ; vcmpltps      %ymm10,%ymm1,%ymm1
  DB  196,195,37,74,201,16                ; vblendvps     %ymm1,%ymm9,%ymm11,%ymm1
  DB  197,124,82,202                      ; vrsqrtps      %ymm2,%ymm9
  DB  196,65,124,83,217                   ; vrcpps        %ymm9,%ymm11
  DB  196,66,13,168,223                   ; vfmadd213ps   %ymm15,%ymm14,%ymm11
  DB  196,65,124,82,201                   ; vrsqrtps      %ymm9,%ymm9
  DB  196,66,21,184,217                   ; vfmadd231ps   %ymm9,%ymm13,%ymm11
  DB  196,65,28,93,203                    ; vminps        %ymm11,%ymm12,%ymm9
  DB  197,60,89,194                       ; vmulps        %ymm2,%ymm8,%ymm8
  DB  196,193,108,194,210,1               ; vcmpltps      %ymm10,%ymm2,%ymm2
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
  DB  73,137,200                          ; mov           %rcx,%r8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  72,1,248                            ; add           %rdi,%rax
  DB  77,133,192                          ; test          %r8,%r8
  DB  117,48                              ; jne           4cf <_sk_scale_u8_hsw+0x40>
  DB  197,123,16,0                        ; vmovsd        (%rax),%xmm8
  DB  196,66,125,49,192                   ; vpmovzxbd     %xmm8,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,74,12                 ; vbroadcastss  0xc(%rdx),%ymm9
  DB  196,65,60,89,193                    ; vmulps        %ymm9,%ymm8,%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  197,188,89,219                      ; vmulps        %ymm3,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,137,193                          ; mov           %r8,%rcx
  DB  255,224                             ; jmpq          *%rax
  DB  49,201                              ; xor           %ecx,%ecx
  DB  77,137,194                          ; mov           %r8,%r10
  DB  69,49,201                           ; xor           %r9d,%r9d
  DB  68,15,182,24                        ; movzbl        (%rax),%r11d
  DB  72,255,192                          ; inc           %rax
  DB  73,211,227                          ; shl           %cl,%r11
  DB  77,9,217                            ; or            %r11,%r9
  DB  72,131,193,8                        ; add           $0x8,%rcx
  DB  73,255,202                          ; dec           %r10
  DB  117,234                             ; jne           4d7 <_sk_scale_u8_hsw+0x48>
  DB  196,65,249,110,193                  ; vmovq         %r9,%xmm8
  DB  235,175                             ; jmp           4a3 <_sk_scale_u8_hsw+0x14>

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
  DB  73,137,200                          ; mov           %rcx,%r8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  72,1,248                            ; add           %rdi,%rax
  DB  77,133,192                          ; test          %r8,%r8
  DB  117,68                              ; jne           577 <_sk_lerp_u8_hsw+0x54>
  DB  197,123,16,0                        ; vmovsd        (%rax),%xmm8
  DB  196,66,125,49,192                   ; vpmovzxbd     %xmm8,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,74,12                 ; vbroadcastss  0xc(%rdx),%ymm9
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
  DB  76,137,193                          ; mov           %r8,%rcx
  DB  255,224                             ; jmpq          *%rax
  DB  49,201                              ; xor           %ecx,%ecx
  DB  77,137,194                          ; mov           %r8,%r10
  DB  69,49,201                           ; xor           %r9d,%r9d
  DB  68,15,182,24                        ; movzbl        (%rax),%r11d
  DB  72,255,192                          ; inc           %rax
  DB  73,211,227                          ; shl           %cl,%r11
  DB  77,9,217                            ; or            %r11,%r9
  DB  72,131,193,8                        ; add           $0x8,%rcx
  DB  73,255,202                          ; dec           %r10
  DB  117,234                             ; jne           57f <_sk_lerp_u8_hsw+0x5c>
  DB  196,65,249,110,193                  ; vmovq         %r9,%xmm8
  DB  235,155                             ; jmp           537 <_sk_lerp_u8_hsw+0x14>

PUBLIC _sk_lerp_565_hsw
_sk_lerp_565_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,16                           ; mov           (%rax),%r10
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,123                             ; jne           621 <_sk_lerp_565_hsw+0x85>
  DB  196,193,122,111,28,122              ; vmovdqu       (%r10,%rdi,2),%xmm3
  DB  196,226,125,51,219                  ; vpmovzxwd     %xmm3,%ymm3
  DB  196,98,125,88,66,104                ; vpbroadcastd  0x68(%rdx),%ymm8
  DB  197,61,219,195                      ; vpand         %ymm3,%ymm8,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,74,116                ; vbroadcastss  0x74(%rdx),%ymm9
  DB  196,65,52,89,192                    ; vmulps        %ymm8,%ymm9,%ymm8
  DB  196,98,125,88,74,108                ; vpbroadcastd  0x6c(%rdx),%ymm9
  DB  197,53,219,203                      ; vpand         %ymm3,%ymm9,%ymm9
  DB  196,65,124,91,201                   ; vcvtdq2ps     %ymm9,%ymm9
  DB  196,98,125,24,82,120                ; vbroadcastss  0x78(%rdx),%ymm10
  DB  196,65,44,89,201                    ; vmulps        %ymm9,%ymm10,%ymm9
  DB  196,98,125,88,82,112                ; vpbroadcastd  0x70(%rdx),%ymm10
  DB  197,173,219,219                     ; vpand         %ymm3,%ymm10,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,98,125,24,82,124                ; vbroadcastss  0x7c(%rdx),%ymm10
  DB  197,172,89,219                      ; vmulps        %ymm3,%ymm10,%ymm3
  DB  197,252,92,196                      ; vsubps        %ymm4,%ymm0,%ymm0
  DB  196,226,61,168,196                  ; vfmadd213ps   %ymm4,%ymm8,%ymm0
  DB  197,244,92,205                      ; vsubps        %ymm5,%ymm1,%ymm1
  DB  196,226,53,168,205                  ; vfmadd213ps   %ymm5,%ymm9,%ymm1
  DB  197,236,92,214                      ; vsubps        %ymm6,%ymm2,%ymm2
  DB  196,226,101,168,214                 ; vfmadd213ps   %ymm6,%ymm3,%ymm2
  DB  196,226,125,24,26                   ; vbroadcastss  (%rdx),%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  65,128,224,7                        ; and           $0x7,%r8b
  DB  197,225,239,219                     ; vpxor         %xmm3,%xmm3,%xmm3
  DB  65,254,200                          ; dec           %r8b
  DB  69,15,182,192                       ; movzbl        %r8b,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  15,135,111,255,255,255              ; ja            5ac <_sk_lerp_565_hsw+0x10>
  DB  76,141,13,76,0,0,0                  ; lea           0x4c(%rip),%r9        # 690 <_sk_lerp_565_hsw+0xf4>
  DB  75,99,4,129                         ; movslq        (%r9,%r8,4),%rax
  DB  76,1,200                            ; add           %r9,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  197,225,239,219                     ; vpxor         %xmm3,%xmm3,%xmm3
  DB  196,193,97,196,92,122,12,6          ; vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm3
  DB  196,193,97,196,92,122,10,5          ; vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm3,%xmm3
  DB  196,193,97,196,92,122,8,4           ; vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm3,%xmm3
  DB  196,193,97,196,92,122,6,3           ; vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm3,%xmm3
  DB  196,193,97,196,92,122,4,2           ; vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm3,%xmm3
  DB  196,193,97,196,92,122,2,1           ; vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm3,%xmm3
  DB  196,193,97,196,28,122,0             ; vpinsrw       $0x0,(%r10,%rdi,2),%xmm3,%xmm3
  DB  233,31,255,255,255                  ; jmpq          5ac <_sk_lerp_565_hsw+0x10>
  DB  15,31,0                             ; nopl          (%rax)
  DB  241                                 ; icebp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  233,255,255,255,225                 ; jmpq          ffffffffe2000698 <_sk_linear_gradient_2stops_hsw+0xffffffffe1fff5ea>
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  217,255                             ; fcos
  DB  255                                 ; (bad)
  DB  255,209                             ; callq         *%rcx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,201                             ; dec           %ecx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  189                                 ; .byte         0xbd
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_tables_hsw
_sk_load_tables_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,0                            ; mov           (%rax),%r8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,104                             ; jne           71e <_sk_load_tables_hsw+0x72>
  DB  196,193,126,111,28,184              ; vmovdqu       (%r8,%rdi,4),%ymm3
  DB  196,226,125,88,82,16                ; vpbroadcastd  0x10(%rdx),%ymm2
  DB  197,237,219,203                     ; vpand         %ymm3,%ymm2,%ymm1
  DB  196,65,61,118,192                   ; vpcmpeqd      %ymm8,%ymm8,%ymm8
  DB  76,139,64,8                         ; mov           0x8(%rax),%r8
  DB  76,139,72,16                        ; mov           0x10(%rax),%r9
  DB  196,65,53,118,201                   ; vpcmpeqd      %ymm9,%ymm9,%ymm9
  DB  196,194,53,146,4,136                ; vgatherdps    %ymm9,(%r8,%ymm1,4),%ymm0
  DB  197,245,114,211,8                   ; vpsrld        $0x8,%ymm3,%ymm1
  DB  197,109,219,201                     ; vpand         %ymm1,%ymm2,%ymm9
  DB  196,65,45,118,210                   ; vpcmpeqd      %ymm10,%ymm10,%ymm10
  DB  196,130,45,146,12,137               ; vgatherdps    %ymm10,(%r9,%ymm9,4),%ymm1
  DB  72,139,64,24                        ; mov           0x18(%rax),%rax
  DB  197,181,114,211,16                  ; vpsrld        $0x10,%ymm3,%ymm9
  DB  196,65,109,219,201                  ; vpand         %ymm9,%ymm2,%ymm9
  DB  196,162,61,146,20,136               ; vgatherdps    %ymm8,(%rax,%ymm9,4),%ymm2
  DB  197,229,114,211,24                  ; vpsrld        $0x18,%ymm3,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,98,125,24,66,12                 ; vbroadcastss  0xc(%rdx),%ymm8
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  65,137,201                          ; mov           %ecx,%r9d
  DB  65,128,225,7                        ; and           $0x7,%r9b
  DB  197,229,239,219                     ; vpxor         %ymm3,%ymm3,%ymm3
  DB  65,254,201                          ; dec           %r9b
  DB  69,15,182,201                       ; movzbl        %r9b,%r9d
  DB  65,128,249,6                        ; cmp           $0x6,%r9b
  DB  119,134                             ; ja            6bc <_sk_load_tables_hsw+0x10>
  DB  76,141,21,131,0,0,0                 ; lea           0x83(%rip),%r10        # 7c0 <_sk_load_tables_hsw+0x114>
  DB  79,99,12,138                        ; movslq        (%r10,%r9,4),%r9
  DB  77,1,209                            ; add           %r10,%r9
  DB  65,255,225                          ; jmpq          *%r9
  DB  196,193,121,110,68,184,24           ; vmovd         0x18(%r8,%rdi,4),%xmm0
  DB  196,226,125,89,192                  ; vpbroadcastq  %xmm0,%ymm0
  DB  197,245,239,201                     ; vpxor         %ymm1,%ymm1,%ymm1
  DB  196,227,117,2,216,64                ; vpblendd      $0x40,%ymm0,%ymm1,%ymm3
  DB  196,227,125,57,216,1                ; vextracti128  $0x1,%ymm3,%xmm0
  DB  196,195,121,34,68,184,20,1          ; vpinsrd       $0x1,0x14(%r8,%rdi,4),%xmm0,%xmm0
  DB  196,227,101,56,216,1                ; vinserti128   $0x1,%xmm0,%ymm3,%ymm3
  DB  196,227,125,57,216,1                ; vextracti128  $0x1,%ymm3,%xmm0
  DB  196,195,121,34,68,184,16,0          ; vpinsrd       $0x0,0x10(%r8,%rdi,4),%xmm0,%xmm0
  DB  196,227,101,56,216,1                ; vinserti128   $0x1,%xmm0,%ymm3,%ymm3
  DB  196,195,97,34,68,184,12,3           ; vpinsrd       $0x3,0xc(%r8,%rdi,4),%xmm3,%xmm0
  DB  196,227,101,2,216,15                ; vpblendd      $0xf,%ymm0,%ymm3,%ymm3
  DB  196,195,97,34,68,184,8,2            ; vpinsrd       $0x2,0x8(%r8,%rdi,4),%xmm3,%xmm0
  DB  196,227,101,2,216,15                ; vpblendd      $0xf,%ymm0,%ymm3,%ymm3
  DB  196,195,97,34,68,184,4,1            ; vpinsrd       $0x1,0x4(%r8,%rdi,4),%xmm3,%xmm0
  DB  196,227,101,2,216,15                ; vpblendd      $0xf,%ymm0,%ymm3,%ymm3
  DB  196,193,121,110,4,184               ; vmovd         (%r8,%rdi,4),%xmm0
  DB  196,227,101,2,216,1                 ; vpblendd      $0x1,%ymm0,%ymm3,%ymm3
  DB  233,252,254,255,255                 ; jmpq          6bc <_sk_load_tables_hsw+0x10>
  DB  239                                 ; out           %eax,(%dx)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,225                             ; jmpq          *%rcx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,211                             ; callq         *%rbx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,197                             ; inc           %ebp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,177,255,255,255,157             ; pushq         -0x62000001(%rcx)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff
  DB  135,255                             ; xchg          %edi,%edi
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_a8_hsw
_sk_load_a8_hsw LABEL PROC
  DB  73,137,200                          ; mov           %rcx,%r8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  72,1,248                            ; add           %rdi,%rax
  DB  77,133,192                          ; test          %r8,%r8
  DB  117,42                              ; jne           816 <_sk_load_a8_hsw+0x3a>
  DB  197,251,16,0                        ; vmovsd        (%rax),%xmm0
  DB  196,226,125,49,192                  ; vpmovzxbd     %xmm0,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,74,12                ; vbroadcastss  0xc(%rdx),%ymm1
  DB  197,252,89,217                      ; vmulps        %ymm1,%ymm0,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  76,137,193                          ; mov           %r8,%rcx
  DB  255,224                             ; jmpq          *%rax
  DB  49,201                              ; xor           %ecx,%ecx
  DB  77,137,194                          ; mov           %r8,%r10
  DB  69,49,201                           ; xor           %r9d,%r9d
  DB  68,15,182,24                        ; movzbl        (%rax),%r11d
  DB  72,255,192                          ; inc           %rax
  DB  73,211,227                          ; shl           %cl,%r11
  DB  77,9,217                            ; or            %r11,%r9
  DB  72,131,193,8                        ; add           $0x8,%rcx
  DB  73,255,202                          ; dec           %r10
  DB  117,234                             ; jne           81e <_sk_load_a8_hsw+0x42>
  DB  196,193,249,110,193                 ; vmovq         %r9,%xmm0
  DB  235,181                             ; jmp           7f0 <_sk_load_a8_hsw+0x14>

PUBLIC _sk_store_a8_hsw
_sk_store_a8_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,8                            ; mov           (%rax),%r9
  DB  196,98,125,24,66,8                  ; vbroadcastss  0x8(%rdx),%ymm8
  DB  197,60,89,195                       ; vmulps        %ymm3,%ymm8,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,67,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm9
  DB  196,66,57,43,193                    ; vpackusdw     %xmm9,%xmm8,%xmm8
  DB  196,65,57,103,192                   ; vpackuswb     %xmm8,%xmm8,%xmm8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,10                              ; jne           86e <_sk_store_a8_hsw+0x33>
  DB  196,65,123,17,4,57                  ; vmovsd        %xmm8,(%r9,%rdi,1)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  137,200                             ; mov           %ecx,%eax
  DB  36,7                                ; and           $0x7,%al
  DB  254,200                             ; dec           %al
  DB  68,15,182,192                       ; movzbl        %al,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  119,236                             ; ja            86a <_sk_store_a8_hsw+0x2f>
  DB  196,66,121,48,192                   ; vpmovzxbw     %xmm8,%xmm8
  DB  76,141,21,66,0,0,0                  ; lea           0x42(%rip),%r10        # 8cc <_sk_store_a8_hsw+0x91>
  DB  75,99,4,130                         ; movslq        (%r10,%r8,4),%rax
  DB  76,1,208                            ; add           %r10,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  196,67,121,20,68,57,6,12            ; vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  DB  196,67,121,20,68,57,5,10            ; vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  DB  196,67,121,20,68,57,4,8             ; vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  DB  196,67,121,20,68,57,3,6             ; vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  DB  196,67,121,20,68,57,2,4             ; vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  DB  196,67,121,20,68,57,1,2             ; vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  DB  196,67,121,20,4,57,0                ; vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  DB  235,158                             ; jmp           86a <_sk_store_a8_hsw+0x2f>
  DB  247,255                             ; idiv          %edi
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  239                                 ; out           %eax,(%dx)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,231                             ; jmpq          *%rdi
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  223,255                             ; (bad)
  DB  255                                 ; (bad)
  DB  255,215                             ; callq         *%rdi
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,207                             ; dec           %edi
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,199                             ; inc           %edi
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_565_hsw
_sk_load_565_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,16                           ; mov           (%rax),%r10
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,92                              ; jne           94e <_sk_load_565_hsw+0x66>
  DB  196,193,122,111,4,122               ; vmovdqu       (%r10,%rdi,2),%xmm0
  DB  196,226,125,51,208                  ; vpmovzxwd     %xmm0,%ymm2
  DB  196,226,125,88,66,104               ; vpbroadcastd  0x68(%rdx),%ymm0
  DB  197,253,219,194                     ; vpand         %ymm2,%ymm0,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,74,116               ; vbroadcastss  0x74(%rdx),%ymm1
  DB  197,244,89,192                      ; vmulps        %ymm0,%ymm1,%ymm0
  DB  196,226,125,88,74,108               ; vpbroadcastd  0x6c(%rdx),%ymm1
  DB  197,245,219,202                     ; vpand         %ymm2,%ymm1,%ymm1
  DB  197,252,91,201                      ; vcvtdq2ps     %ymm1,%ymm1
  DB  196,226,125,24,90,120               ; vbroadcastss  0x78(%rdx),%ymm3
  DB  197,228,89,201                      ; vmulps        %ymm1,%ymm3,%ymm1
  DB  196,226,125,88,90,112               ; vpbroadcastd  0x70(%rdx),%ymm3
  DB  197,229,219,210                     ; vpand         %ymm2,%ymm3,%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  196,226,125,24,90,124               ; vbroadcastss  0x7c(%rdx),%ymm3
  DB  197,228,89,210                      ; vmulps        %ymm2,%ymm3,%ymm2
  DB  196,226,125,24,26                   ; vbroadcastss  (%rdx),%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  65,128,224,7                        ; and           $0x7,%r8b
  DB  197,249,239,192                     ; vpxor         %xmm0,%xmm0,%xmm0
  DB  65,254,200                          ; dec           %r8b
  DB  69,15,182,192                       ; movzbl        %r8b,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  119,146                             ; ja            8f8 <_sk_load_565_hsw+0x10>
  DB  76,141,13,75,0,0,0                  ; lea           0x4b(%rip),%r9        # 9b8 <_sk_load_565_hsw+0xd0>
  DB  75,99,4,129                         ; movslq        (%r9,%r8,4),%rax
  DB  76,1,200                            ; add           %r9,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  197,249,239,192                     ; vpxor         %xmm0,%xmm0,%xmm0
  DB  196,193,121,196,68,122,12,6         ; vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,10,5         ; vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,8,4          ; vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,6,3          ; vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,4,2          ; vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,2,1          ; vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,4,122,0             ; vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  DB  233,66,255,255,255                  ; jmpq          8f8 <_sk_load_565_hsw+0x10>
  DB  102,144                             ; xchg          %ax,%ax
  DB  242,255                             ; repnz         (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  234                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,226                             ; jmpq          *%rdx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  218,255                             ; (bad)
  DB  255                                 ; (bad)
  DB  255,210                             ; callq         *%rdx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,202                             ; dec           %edx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  190                                 ; .byte         0xbe
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_store_565_hsw
_sk_store_565_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,8                            ; mov           (%rax),%r9
  DB  196,98,125,24,130,128,0,0,0         ; vbroadcastss  0x80(%rdx),%ymm8
  DB  197,60,89,200                       ; vmulps        %ymm0,%ymm8,%ymm9
  DB  196,65,125,91,201                   ; vcvtps2dq     %ymm9,%ymm9
  DB  196,193,53,114,241,11               ; vpslld        $0xb,%ymm9,%ymm9
  DB  196,98,125,24,146,132,0,0,0         ; vbroadcastss  0x84(%rdx),%ymm10
  DB  197,44,89,209                       ; vmulps        %ymm1,%ymm10,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,45,114,242,5                ; vpslld        $0x5,%ymm10,%ymm10
  DB  196,65,45,235,201                   ; vpor          %ymm9,%ymm10,%ymm9
  DB  197,60,89,194                       ; vmulps        %ymm2,%ymm8,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,65,53,235,192                   ; vpor          %ymm8,%ymm9,%ymm8
  DB  196,67,125,57,193,1                 ; vextracti128  $0x1,%ymm8,%xmm9
  DB  196,66,57,43,193                    ; vpackusdw     %xmm9,%xmm8,%xmm8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,10                              ; jne           a36 <_sk_store_565_hsw+0x62>
  DB  196,65,122,127,4,121                ; vmovdqu       %xmm8,(%r9,%rdi,2)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  137,200                             ; mov           %ecx,%eax
  DB  36,7                                ; and           $0x7,%al
  DB  254,200                             ; dec           %al
  DB  68,15,182,192                       ; movzbl        %al,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  119,236                             ; ja            a32 <_sk_store_565_hsw+0x5e>
  DB  76,141,21,71,0,0,0                  ; lea           0x47(%rip),%r10        # a94 <_sk_store_565_hsw+0xc0>
  DB  75,99,4,130                         ; movslq        (%r10,%r8,4),%rax
  DB  76,1,208                            ; add           %r10,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  196,67,121,21,68,121,12,6           ; vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  DB  196,67,121,21,68,121,10,5           ; vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  DB  196,67,121,21,68,121,8,4            ; vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  DB  196,67,121,21,68,121,6,3            ; vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  DB  196,67,121,21,68,121,4,2            ; vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  DB  196,67,121,21,68,121,2,1            ; vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  DB  197,121,126,192                     ; vmovd         %xmm8,%eax
  DB  102,65,137,4,121                    ; mov           %ax,(%r9,%rdi,2)
  DB  235,161                             ; jmp           a32 <_sk_store_565_hsw+0x5e>
  DB  15,31,0                             ; nopl          (%rax)
  DB  242,255                             ; repnz         (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  234                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,226                             ; jmpq          *%rdx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  218,255                             ; (bad)
  DB  255                                 ; (bad)
  DB  255,210                             ; callq         *%rdx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,202                             ; dec           %edx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,194                             ; inc           %edx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_8888_hsw
_sk_load_8888_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,16                           ; mov           (%rax),%r10
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,83                              ; jne           b0d <_sk_load_8888_hsw+0x5d>
  DB  196,193,126,111,28,186              ; vmovdqu       (%r10,%rdi,4),%ymm3
  DB  196,226,125,88,82,16                ; vpbroadcastd  0x10(%rdx),%ymm2
  DB  197,237,219,195                     ; vpand         %ymm3,%ymm2,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,98,125,24,66,12                 ; vbroadcastss  0xc(%rdx),%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,245,114,211,8                   ; vpsrld        $0x8,%ymm3,%ymm1
  DB  197,237,219,201                     ; vpand         %ymm1,%ymm2,%ymm1
  DB  197,252,91,201                      ; vcvtdq2ps     %ymm1,%ymm1
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,181,114,211,16                  ; vpsrld        $0x10,%ymm3,%ymm9
  DB  196,193,109,219,209                 ; vpand         %ymm9,%ymm2,%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  197,229,114,211,24                  ; vpsrld        $0x18,%ymm3,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  65,128,224,7                        ; and           $0x7,%r8b
  DB  197,229,239,219                     ; vpxor         %ymm3,%ymm3,%ymm3
  DB  65,254,200                          ; dec           %r8b
  DB  69,15,182,192                       ; movzbl        %r8b,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  119,155                             ; ja            ac0 <_sk_load_8888_hsw+0x10>
  DB  76,141,13,132,0,0,0                 ; lea           0x84(%rip),%r9        # bb0 <_sk_load_8888_hsw+0x100>
  DB  75,99,4,129                         ; movslq        (%r9,%r8,4),%rax
  DB  76,1,200                            ; add           %r9,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  196,193,121,110,68,186,24           ; vmovd         0x18(%r10,%rdi,4),%xmm0
  DB  196,226,125,89,192                  ; vpbroadcastq  %xmm0,%ymm0
  DB  197,245,239,201                     ; vpxor         %ymm1,%ymm1,%ymm1
  DB  196,227,117,2,216,64                ; vpblendd      $0x40,%ymm0,%ymm1,%ymm3
  DB  196,227,125,57,216,1                ; vextracti128  $0x1,%ymm3,%xmm0
  DB  196,195,121,34,68,186,20,1          ; vpinsrd       $0x1,0x14(%r10,%rdi,4),%xmm0,%xmm0
  DB  196,227,101,56,216,1                ; vinserti128   $0x1,%xmm0,%ymm3,%ymm3
  DB  196,227,125,57,216,1                ; vextracti128  $0x1,%ymm3,%xmm0
  DB  196,195,121,34,68,186,16,0          ; vpinsrd       $0x0,0x10(%r10,%rdi,4),%xmm0,%xmm0
  DB  196,227,101,56,216,1                ; vinserti128   $0x1,%xmm0,%ymm3,%ymm3
  DB  196,195,97,34,68,186,12,3           ; vpinsrd       $0x3,0xc(%r10,%rdi,4),%xmm3,%xmm0
  DB  196,227,101,2,216,15                ; vpblendd      $0xf,%ymm0,%ymm3,%ymm3
  DB  196,195,97,34,68,186,8,2            ; vpinsrd       $0x2,0x8(%r10,%rdi,4),%xmm3,%xmm0
  DB  196,227,101,2,216,15                ; vpblendd      $0xf,%ymm0,%ymm3,%ymm3
  DB  196,195,97,34,68,186,4,1            ; vpinsrd       $0x1,0x4(%r10,%rdi,4),%xmm3,%xmm0
  DB  196,227,101,2,216,15                ; vpblendd      $0xf,%ymm0,%ymm3,%ymm3
  DB  196,193,121,110,4,186               ; vmovd         (%r10,%rdi,4),%xmm0
  DB  196,227,101,2,216,1                 ; vpblendd      $0x1,%ymm0,%ymm3,%ymm3
  DB  233,18,255,255,255                  ; jmpq          ac0 <_sk_load_8888_hsw+0x10>
  DB  102,144                             ; xchg          %ax,%ax
  DB  237                                 ; in            (%dx),%eax
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  223,255                             ; (bad)
  DB  255                                 ; (bad)
  DB  255,209                             ; callq         *%rcx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,195                             ; inc           %ebx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,175,255,255,255,155             ; ljmp          *-0x64000001(%rdi)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff
  DB  133,255                             ; test          %edi,%edi
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_store_8888_hsw
_sk_store_8888_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,8                            ; mov           (%rax),%r9
  DB  196,98,125,24,66,8                  ; vbroadcastss  0x8(%rdx),%ymm8
  DB  197,60,89,200                       ; vmulps        %ymm0,%ymm8,%ymm9
  DB  196,65,125,91,201                   ; vcvtps2dq     %ymm9,%ymm9
  DB  197,60,89,209                       ; vmulps        %ymm1,%ymm8,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,45,114,242,8                ; vpslld        $0x8,%ymm10,%ymm10
  DB  196,65,45,235,201                   ; vpor          %ymm9,%ymm10,%ymm9
  DB  197,60,89,210                       ; vmulps        %ymm2,%ymm8,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,45,114,242,16               ; vpslld        $0x10,%ymm10,%ymm10
  DB  197,60,89,195                       ; vmulps        %ymm3,%ymm8,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,193,61,114,240,24               ; vpslld        $0x18,%ymm8,%ymm8
  DB  196,65,45,235,192                   ; vpor          %ymm8,%ymm10,%ymm8
  DB  196,65,53,235,192                   ; vpor          %ymm8,%ymm9,%ymm8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,10                              ; jne           c2b <_sk_store_8888_hsw+0x5f>
  DB  196,65,126,127,4,185                ; vmovdqu       %ymm8,(%r9,%rdi,4)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  137,200                             ; mov           %ecx,%eax
  DB  36,7                                ; and           $0x7,%al
  DB  254,200                             ; dec           %al
  DB  68,15,182,192                       ; movzbl        %al,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  119,236                             ; ja            c27 <_sk_store_8888_hsw+0x5b>
  DB  76,141,21,82,0,0,0                  ; lea           0x52(%rip),%r10        # c94 <_sk_store_8888_hsw+0xc8>
  DB  75,99,4,130                         ; movslq        (%r10,%r8,4),%rax
  DB  76,1,208                            ; add           %r10,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  196,67,125,57,193,1                 ; vextracti128  $0x1,%ymm8,%xmm9
  DB  196,67,121,22,76,185,24,2           ; vpextrd       $0x2,%xmm9,0x18(%r9,%rdi,4)
  DB  196,67,125,57,193,1                 ; vextracti128  $0x1,%ymm8,%xmm9
  DB  196,67,121,22,76,185,20,1           ; vpextrd       $0x1,%xmm9,0x14(%r9,%rdi,4)
  DB  196,67,125,57,193,1                 ; vextracti128  $0x1,%ymm8,%xmm9
  DB  196,65,121,126,76,185,16            ; vmovd         %xmm9,0x10(%r9,%rdi,4)
  DB  196,67,121,22,68,185,12,3           ; vpextrd       $0x3,%xmm8,0xc(%r9,%rdi,4)
  DB  196,67,121,22,68,185,8,2            ; vpextrd       $0x2,%xmm8,0x8(%r9,%rdi,4)
  DB  196,67,121,22,68,185,4,1            ; vpextrd       $0x1,%xmm8,0x4(%r9,%rdi,4)
  DB  196,65,121,126,4,185                ; vmovd         %xmm8,(%r9,%rdi,4)
  DB  235,147                             ; jmp           c27 <_sk_store_8888_hsw+0x5b>
  DB  248                                 ; clc
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,240                             ; push          %rax
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  232,255,255,255,224                 ; callq         ffffffffe1000ca0 <_sk_linear_gradient_2stops_hsw+0xffffffffe0fffbf2>
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,211                             ; callq         *%rbx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,197                             ; inc           %ebp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff
  DB  183,255                             ; mov           $0xff,%bh
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_f16_hsw
_sk_load_f16_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,97                              ; jne           d1b <_sk_load_f16_hsw+0x6b>
  DB  197,249,16,12,248                   ; vmovupd       (%rax,%rdi,8),%xmm1
  DB  197,249,16,84,248,16                ; vmovupd       0x10(%rax,%rdi,8),%xmm2
  DB  197,249,16,92,248,32                ; vmovupd       0x20(%rax,%rdi,8),%xmm3
  DB  197,121,16,68,248,48                ; vmovupd       0x30(%rax,%rdi,8),%xmm8
  DB  197,241,97,194                      ; vpunpcklwd    %xmm2,%xmm1,%xmm0
  DB  197,241,105,202                     ; vpunpckhwd    %xmm2,%xmm1,%xmm1
  DB  196,193,97,97,208                   ; vpunpcklwd    %xmm8,%xmm3,%xmm2
  DB  196,193,97,105,216                  ; vpunpckhwd    %xmm8,%xmm3,%xmm3
  DB  197,121,97,193                      ; vpunpcklwd    %xmm1,%xmm0,%xmm8
  DB  197,121,105,201                     ; vpunpckhwd    %xmm1,%xmm0,%xmm9
  DB  197,233,97,203                      ; vpunpcklwd    %xmm3,%xmm2,%xmm1
  DB  197,233,105,219                     ; vpunpckhwd    %xmm3,%xmm2,%xmm3
  DB  197,185,108,193                     ; vpunpcklqdq   %xmm1,%xmm8,%xmm0
  DB  196,226,125,19,192                  ; vcvtph2ps     %xmm0,%ymm0
  DB  197,185,109,201                     ; vpunpckhqdq   %xmm1,%xmm8,%xmm1
  DB  196,226,125,19,201                  ; vcvtph2ps     %xmm1,%ymm1
  DB  197,177,108,211                     ; vpunpcklqdq   %xmm3,%xmm9,%xmm2
  DB  196,226,125,19,210                  ; vcvtph2ps     %xmm2,%ymm2
  DB  197,177,109,219                     ; vpunpckhqdq   %xmm3,%xmm9,%xmm3
  DB  196,226,125,19,219                  ; vcvtph2ps     %xmm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  197,251,16,12,248                   ; vmovsd        (%rax,%rdi,8),%xmm1
  DB  196,65,57,87,192                    ; vxorpd        %xmm8,%xmm8,%xmm8
  DB  72,131,249,1                        ; cmp           $0x1,%rcx
  DB  117,6                               ; jne           d31 <_sk_load_f16_hsw+0x81>
  DB  197,250,126,201                     ; vmovq         %xmm1,%xmm1
  DB  235,30                              ; jmp           d4f <_sk_load_f16_hsw+0x9f>
  DB  197,241,22,76,248,8                 ; vmovhpd       0x8(%rax,%rdi,8),%xmm1,%xmm1
  DB  72,131,249,3                        ; cmp           $0x3,%rcx
  DB  114,18                              ; jb            d4f <_sk_load_f16_hsw+0x9f>
  DB  197,251,16,84,248,16                ; vmovsd        0x10(%rax,%rdi,8),%xmm2
  DB  72,131,249,3                        ; cmp           $0x3,%rcx
  DB  117,19                              ; jne           d5c <_sk_load_f16_hsw+0xac>
  DB  197,250,126,210                     ; vmovq         %xmm2,%xmm2
  DB  235,46                              ; jmp           d7d <_sk_load_f16_hsw+0xcd>
  DB  197,225,87,219                      ; vxorpd        %xmm3,%xmm3,%xmm3
  DB  197,233,87,210                      ; vxorpd        %xmm2,%xmm2,%xmm2
  DB  233,117,255,255,255                 ; jmpq          cd1 <_sk_load_f16_hsw+0x21>
  DB  197,233,22,84,248,24                ; vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  DB  72,131,249,5                        ; cmp           $0x5,%rcx
  DB  114,21                              ; jb            d7d <_sk_load_f16_hsw+0xcd>
  DB  197,251,16,92,248,32                ; vmovsd        0x20(%rax,%rdi,8),%xmm3
  DB  72,131,249,5                        ; cmp           $0x5,%rcx
  DB  117,18                              ; jne           d86 <_sk_load_f16_hsw+0xd6>
  DB  197,250,126,219                     ; vmovq         %xmm3,%xmm3
  DB  233,84,255,255,255                  ; jmpq          cd1 <_sk_load_f16_hsw+0x21>
  DB  197,225,87,219                      ; vxorpd        %xmm3,%xmm3,%xmm3
  DB  233,75,255,255,255                  ; jmpq          cd1 <_sk_load_f16_hsw+0x21>
  DB  197,225,22,92,248,40                ; vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  DB  72,131,249,7                        ; cmp           $0x7,%rcx
  DB  15,130,59,255,255,255               ; jb            cd1 <_sk_load_f16_hsw+0x21>
  DB  197,123,16,68,248,48                ; vmovsd        0x30(%rax,%rdi,8),%xmm8
  DB  233,48,255,255,255                  ; jmpq          cd1 <_sk_load_f16_hsw+0x21>

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
  DB  196,65,41,105,235                   ; vpunpckhwd    %xmm11,%xmm10,%xmm13
  DB  196,65,25,98,217                    ; vpunpckldq    %xmm9,%xmm12,%xmm11
  DB  196,65,25,106,209                   ; vpunpckhdq    %xmm9,%xmm12,%xmm10
  DB  196,65,57,98,205                    ; vpunpckldq    %xmm13,%xmm8,%xmm9
  DB  196,65,57,106,197                   ; vpunpckhdq    %xmm13,%xmm8,%xmm8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,27                              ; jne           e06 <_sk_store_f16_hsw+0x65>
  DB  197,120,17,28,248                   ; vmovups       %xmm11,(%rax,%rdi,8)
  DB  197,120,17,84,248,16                ; vmovups       %xmm10,0x10(%rax,%rdi,8)
  DB  197,120,17,76,248,32                ; vmovups       %xmm9,0x20(%rax,%rdi,8)
  DB  197,122,127,68,248,48               ; vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  197,121,214,28,248                  ; vmovq         %xmm11,(%rax,%rdi,8)
  DB  72,131,249,1                        ; cmp           $0x1,%rcx
  DB  116,241                             ; je            e02 <_sk_store_f16_hsw+0x61>
  DB  197,121,23,92,248,8                 ; vmovhpd       %xmm11,0x8(%rax,%rdi,8)
  DB  72,131,249,3                        ; cmp           $0x3,%rcx
  DB  114,229                             ; jb            e02 <_sk_store_f16_hsw+0x61>
  DB  197,121,214,84,248,16               ; vmovq         %xmm10,0x10(%rax,%rdi,8)
  DB  116,221                             ; je            e02 <_sk_store_f16_hsw+0x61>
  DB  197,121,23,84,248,24                ; vmovhpd       %xmm10,0x18(%rax,%rdi,8)
  DB  72,131,249,5                        ; cmp           $0x5,%rcx
  DB  114,209                             ; jb            e02 <_sk_store_f16_hsw+0x61>
  DB  197,121,214,76,248,32               ; vmovq         %xmm9,0x20(%rax,%rdi,8)
  DB  116,201                             ; je            e02 <_sk_store_f16_hsw+0x61>
  DB  197,121,23,76,248,40                ; vmovhpd       %xmm9,0x28(%rax,%rdi,8)
  DB  72,131,249,7                        ; cmp           $0x7,%rcx
  DB  114,189                             ; jb            e02 <_sk_store_f16_hsw+0x61>
  DB  197,121,214,68,248,48               ; vmovq         %xmm8,0x30(%rax,%rdi,8)
  DB  235,181                             ; jmp           e02 <_sk_store_f16_hsw+0x61>

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
  DB  196,98,61,172,200                   ; vfnmadd213ps  %ymm0,%ymm8,%ymm9
  DB  197,253,118,192                     ; vpcmpeqd      %ymm0,%ymm0,%ymm0
  DB  197,189,254,192                     ; vpaddd        %ymm0,%ymm8,%ymm0
  DB  197,180,93,192                      ; vminps        %ymm0,%ymm9,%ymm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_repeat_y_hsw
_sk_repeat_y_hsw LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,0                     ; vbroadcastss  (%rax),%ymm8
  DB  196,65,116,94,200                   ; vdivps        %ymm8,%ymm1,%ymm9
  DB  196,67,125,8,201,1                  ; vroundps      $0x1,%ymm9,%ymm9
  DB  196,98,61,172,201                   ; vfnmadd213ps  %ymm1,%ymm8,%ymm9
  DB  197,245,118,201                     ; vpcmpeqd      %ymm1,%ymm1,%ymm1
  DB  197,189,254,201                     ; vpaddd        %ymm1,%ymm8,%ymm1
  DB  197,180,93,201                      ; vminps        %ymm1,%ymm9,%ymm1
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
  DB  196,66,125,172,194                  ; vfnmadd213ps  %ymm10,%ymm0,%ymm8
  DB  196,193,60,92,193                   ; vsubps        %ymm9,%ymm8,%ymm0
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
  DB  196,66,117,172,194                  ; vfnmadd213ps  %ymm10,%ymm1,%ymm8
  DB  196,193,60,92,201                   ; vsubps        %ymm9,%ymm8,%ymm1
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
  DB  77,137,205                          ; mov           %r9,%r13
  DB  77,137,198                          ; mov           %r8,%r14
  DB  72,137,203                          ; mov           %rcx,%rbx
  DB  72,137,214                          ; mov           %rdx,%rsi
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  73,137,199                          ; mov           %rax,%r15
  DB  73,137,244                          ; mov           %rsi,%r12
  DB  72,141,67,8                         ; lea           0x8(%rbx),%rax
  DB  76,57,232                           ; cmp           %r13,%rax
  DB  118,5                               ; jbe           75 <_sk_start_pipeline_avx+0x75>
  DB  72,137,223                          ; mov           %rbx,%rdi
  DB  235,65                              ; jmp           b6 <_sk_start_pipeline_avx+0xb6>
  DB  185,0,0,0,0                         ; mov           $0x0,%ecx
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  197,220,87,228                      ; vxorps        %ymm4,%ymm4,%ymm4
  DB  197,212,87,237                      ; vxorps        %ymm5,%ymm5,%ymm5
  DB  197,204,87,246                      ; vxorps        %ymm6,%ymm6,%ymm6
  DB  197,196,87,255                      ; vxorps        %ymm7,%ymm7,%ymm7
  DB  72,137,223                          ; mov           %rbx,%rdi
  DB  76,137,230                          ; mov           %r12,%rsi
  DB  76,137,242                          ; mov           %r14,%rdx
  DB  65,255,215                          ; callq         *%r15
  DB  72,141,123,8                        ; lea           0x8(%rbx),%rdi
  DB  72,131,195,16                       ; add           $0x10,%rbx
  DB  76,57,235                           ; cmp           %r13,%rbx
  DB  72,137,251                          ; mov           %rdi,%rbx
  DB  118,191                             ; jbe           75 <_sk_start_pipeline_avx+0x75>
  DB  76,137,233                          ; mov           %r13,%rcx
  DB  72,41,249                           ; sub           %rdi,%rcx
  DB  116,41                              ; je            e7 <_sk_start_pipeline_avx+0xe7>
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  197,228,87,219                      ; vxorps        %ymm3,%ymm3,%ymm3
  DB  197,220,87,228                      ; vxorps        %ymm4,%ymm4,%ymm4
  DB  197,212,87,237                      ; vxorps        %ymm5,%ymm5,%ymm5
  DB  197,204,87,246                      ; vxorps        %ymm6,%ymm6,%ymm6
  DB  197,196,87,255                      ; vxorps        %ymm7,%ymm7,%ymm7
  DB  76,137,230                          ; mov           %r12,%rsi
  DB  76,137,242                          ; mov           %r14,%rdx
  DB  65,255,215                          ; callq         *%r15
  DB  76,137,232                          ; mov           %r13,%rax
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
  DB  197,249,112,192,0                   ; vpshufd       $0x0,%xmm0,%xmm0
  DB  196,227,125,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,74,4                 ; vbroadcastss  0x4(%rdx),%ymm1
  DB  197,252,88,193                      ; vaddps        %ymm1,%ymm0,%ymm0
  DB  197,252,88,66,20                    ; vaddps        0x14(%rdx),%ymm0,%ymm0
  DB  196,226,125,24,16                   ; vbroadcastss  (%rax),%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  197,236,88,201                      ; vaddps        %ymm1,%ymm2,%ymm1
  DB  196,226,125,24,18                   ; vbroadcastss  (%rdx),%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
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
  DB  196,98,125,24,2                     ; vbroadcastss  (%rdx),%ymm8
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
  DB  196,98,125,24,2                     ; vbroadcastss  (%rdx),%ymm8
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
  DB  196,98,125,24,2                     ; vbroadcastss  (%rdx),%ymm8
  DB  196,193,124,93,192                  ; vminps        %ymm8,%ymm0,%ymm0
  DB  196,193,116,93,200                  ; vminps        %ymm8,%ymm1,%ymm1
  DB  196,193,108,93,208                  ; vminps        %ymm8,%ymm2,%ymm2
  DB  196,193,100,93,216                  ; vminps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_a_avx
_sk_clamp_a_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  196,98,125,24,2                     ; vbroadcastss  (%rdx),%ymm8
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
  DB  196,98,125,24,18                    ; vbroadcastss  (%rdx),%ymm10
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
  DB  196,98,125,24,66,64                 ; vbroadcastss  0x40(%rdx),%ymm8
  DB  197,60,89,200                       ; vmulps        %ymm0,%ymm8,%ymm9
  DB  197,124,89,208                      ; vmulps        %ymm0,%ymm0,%ymm10
  DB  196,98,125,24,90,60                 ; vbroadcastss  0x3c(%rdx),%ymm11
  DB  196,98,125,24,98,56                 ; vbroadcastss  0x38(%rdx),%ymm12
  DB  197,36,89,232                       ; vmulps        %ymm0,%ymm11,%ymm13
  DB  196,65,20,88,236                    ; vaddps        %ymm12,%ymm13,%ymm13
  DB  196,98,125,24,114,52                ; vbroadcastss  0x34(%rdx),%ymm14
  DB  196,65,44,89,213                    ; vmulps        %ymm13,%ymm10,%ymm10
  DB  196,65,12,88,210                    ; vaddps        %ymm10,%ymm14,%ymm10
  DB  196,98,125,24,106,68                ; vbroadcastss  0x44(%rdx),%ymm13
  DB  196,193,124,194,197,1               ; vcmpltps      %ymm13,%ymm0,%ymm0
  DB  196,195,45,74,193,0                 ; vblendvps     %ymm0,%ymm9,%ymm10,%ymm0
  DB  197,60,89,201                       ; vmulps        %ymm1,%ymm8,%ymm9
  DB  197,116,89,209                      ; vmulps        %ymm1,%ymm1,%ymm10
  DB  197,36,89,249                       ; vmulps        %ymm1,%ymm11,%ymm15
  DB  196,65,4,88,252                     ; vaddps        %ymm12,%ymm15,%ymm15
  DB  196,65,44,89,215                    ; vmulps        %ymm15,%ymm10,%ymm10
  DB  196,65,12,88,210                    ; vaddps        %ymm10,%ymm14,%ymm10
  DB  196,193,116,194,205,1               ; vcmpltps      %ymm13,%ymm1,%ymm1
  DB  196,195,45,74,201,16                ; vblendvps     %ymm1,%ymm9,%ymm10,%ymm1
  DB  197,60,89,194                       ; vmulps        %ymm2,%ymm8,%ymm8
  DB  197,108,89,202                      ; vmulps        %ymm2,%ymm2,%ymm9
  DB  197,36,89,210                       ; vmulps        %ymm2,%ymm11,%ymm10
  DB  196,65,44,88,212                    ; vaddps        %ymm12,%ymm10,%ymm10
  DB  196,65,52,89,202                    ; vmulps        %ymm10,%ymm9,%ymm9
  DB  196,65,12,88,201                    ; vaddps        %ymm9,%ymm14,%ymm9
  DB  196,193,108,194,213,1               ; vcmpltps      %ymm13,%ymm2,%ymm2
  DB  196,195,53,74,208,32                ; vblendvps     %ymm2,%ymm8,%ymm9,%ymm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_to_srgb_avx
_sk_to_srgb_avx LABEL PROC
  DB  197,124,82,192                      ; vrsqrtps      %ymm0,%ymm8
  DB  196,65,124,83,200                   ; vrcpps        %ymm8,%ymm9
  DB  196,65,124,82,208                   ; vrsqrtps      %ymm8,%ymm10
  DB  196,98,125,24,66,72                 ; vbroadcastss  0x48(%rdx),%ymm8
  DB  197,60,89,216                       ; vmulps        %ymm0,%ymm8,%ymm11
  DB  196,98,125,24,34                    ; vbroadcastss  (%rdx),%ymm12
  DB  196,98,125,24,106,76                ; vbroadcastss  0x4c(%rdx),%ymm13
  DB  196,98,125,24,114,80                ; vbroadcastss  0x50(%rdx),%ymm14
  DB  196,98,125,24,122,84                ; vbroadcastss  0x54(%rdx),%ymm15
  DB  196,65,52,89,206                    ; vmulps        %ymm14,%ymm9,%ymm9
  DB  196,65,52,88,207                    ; vaddps        %ymm15,%ymm9,%ymm9
  DB  196,65,44,89,213                    ; vmulps        %ymm13,%ymm10,%ymm10
  DB  196,65,44,88,201                    ; vaddps        %ymm9,%ymm10,%ymm9
  DB  196,65,28,93,201                    ; vminps        %ymm9,%ymm12,%ymm9
  DB  196,98,125,24,82,88                 ; vbroadcastss  0x58(%rdx),%ymm10
  DB  196,193,124,194,194,1               ; vcmpltps      %ymm10,%ymm0,%ymm0
  DB  196,195,53,74,195,0                 ; vblendvps     %ymm0,%ymm11,%ymm9,%ymm0
  DB  197,124,82,201                      ; vrsqrtps      %ymm1,%ymm9
  DB  196,65,124,83,217                   ; vrcpps        %ymm9,%ymm11
  DB  196,65,124,82,201                   ; vrsqrtps      %ymm9,%ymm9
  DB  196,65,12,89,219                    ; vmulps        %ymm11,%ymm14,%ymm11
  DB  196,65,4,88,219                     ; vaddps        %ymm11,%ymm15,%ymm11
  DB  196,65,20,89,201                    ; vmulps        %ymm9,%ymm13,%ymm9
  DB  196,65,52,88,203                    ; vaddps        %ymm11,%ymm9,%ymm9
  DB  197,60,89,217                       ; vmulps        %ymm1,%ymm8,%ymm11
  DB  196,65,28,93,201                    ; vminps        %ymm9,%ymm12,%ymm9
  DB  196,193,116,194,202,1               ; vcmpltps      %ymm10,%ymm1,%ymm1
  DB  196,195,53,74,203,16                ; vblendvps     %ymm1,%ymm11,%ymm9,%ymm1
  DB  197,124,82,202                      ; vrsqrtps      %ymm2,%ymm9
  DB  196,65,124,83,217                   ; vrcpps        %ymm9,%ymm11
  DB  196,65,12,89,219                    ; vmulps        %ymm11,%ymm14,%ymm11
  DB  196,65,4,88,219                     ; vaddps        %ymm11,%ymm15,%ymm11
  DB  196,65,124,82,201                   ; vrsqrtps      %ymm9,%ymm9
  DB  196,65,20,89,201                    ; vmulps        %ymm9,%ymm13,%ymm9
  DB  196,65,52,88,203                    ; vaddps        %ymm11,%ymm9,%ymm9
  DB  196,65,28,93,201                    ; vminps        %ymm9,%ymm12,%ymm9
  DB  197,60,89,194                       ; vmulps        %ymm2,%ymm8,%ymm8
  DB  196,193,108,194,210,1               ; vcmpltps      %ymm10,%ymm2,%ymm2
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
  DB  73,137,200                          ; mov           %rcx,%r8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  72,1,248                            ; add           %rdi,%rax
  DB  77,133,192                          ; test          %r8,%r8
  DB  117,65                              ; jne           52d <_sk_scale_u8_avx+0x51>
  DB  197,123,16,0                        ; vmovsd        (%rax),%xmm8
  DB  196,66,121,49,200                   ; vpmovzxbd     %xmm8,%xmm9
  DB  196,67,121,4,192,229                ; vpermilps     $0xe5,%xmm8,%xmm8
  DB  196,66,121,49,192                   ; vpmovzxbd     %xmm8,%xmm8
  DB  196,67,53,24,192,1                  ; vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,74,12                 ; vbroadcastss  0xc(%rdx),%ymm9
  DB  196,65,60,89,193                    ; vmulps        %ymm9,%ymm8,%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  197,188,89,219                      ; vmulps        %ymm3,%ymm8,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,137,193                          ; mov           %r8,%rcx
  DB  255,224                             ; jmpq          *%rax
  DB  49,201                              ; xor           %ecx,%ecx
  DB  77,137,194                          ; mov           %r8,%r10
  DB  69,49,201                           ; xor           %r9d,%r9d
  DB  68,15,182,24                        ; movzbl        (%rax),%r11d
  DB  72,255,192                          ; inc           %rax
  DB  73,211,227                          ; shl           %cl,%r11
  DB  77,9,217                            ; or            %r11,%r9
  DB  72,131,193,8                        ; add           $0x8,%rcx
  DB  73,255,202                          ; dec           %r10
  DB  117,234                             ; jne           535 <_sk_scale_u8_avx+0x59>
  DB  196,65,249,110,193                  ; vmovq         %r9,%xmm8
  DB  235,158                             ; jmp           4f0 <_sk_scale_u8_avx+0x14>

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
  DB  73,137,200                          ; mov           %rcx,%r8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  72,1,248                            ; add           %rdi,%rax
  DB  77,133,192                          ; test          %r8,%r8
  DB  117,101                             ; jne           606 <_sk_lerp_u8_avx+0x75>
  DB  197,123,16,0                        ; vmovsd        (%rax),%xmm8
  DB  196,66,121,49,200                   ; vpmovzxbd     %xmm8,%xmm9
  DB  196,67,121,4,192,229                ; vpermilps     $0xe5,%xmm8,%xmm8
  DB  196,66,121,49,192                   ; vpmovzxbd     %xmm8,%xmm8
  DB  196,67,53,24,192,1                  ; vinsertf128   $0x1,%xmm8,%ymm9,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,74,12                 ; vbroadcastss  0xc(%rdx),%ymm9
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
  DB  76,137,193                          ; mov           %r8,%rcx
  DB  255,224                             ; jmpq          *%rax
  DB  49,201                              ; xor           %ecx,%ecx
  DB  77,137,194                          ; mov           %r8,%r10
  DB  69,49,201                           ; xor           %r9d,%r9d
  DB  68,15,182,24                        ; movzbl        (%rax),%r11d
  DB  72,255,192                          ; inc           %rax
  DB  73,211,227                          ; shl           %cl,%r11
  DB  77,9,217                            ; or            %r11,%r9
  DB  72,131,193,8                        ; add           $0x8,%rcx
  DB  73,255,202                          ; dec           %r10
  DB  117,234                             ; jne           60e <_sk_lerp_u8_avx+0x7d>
  DB  196,65,249,110,193                  ; vmovq         %r9,%xmm8
  DB  233,119,255,255,255                 ; jmpq          5a5 <_sk_lerp_u8_avx+0x14>

PUBLIC _sk_lerp_565_avx
_sk_lerp_565_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,16                           ; mov           (%rax),%r10
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  15,133,148,0,0,0                    ; jne           6d0 <_sk_lerp_565_avx+0xa2>
  DB  196,65,122,111,4,122                ; vmovdqu       (%r10,%rdi,2),%xmm8
  DB  197,225,239,219                     ; vpxor         %xmm3,%xmm3,%xmm3
  DB  197,185,105,219                     ; vpunpckhwd    %xmm3,%xmm8,%xmm3
  DB  196,66,121,51,192                   ; vpmovzxwd     %xmm8,%xmm8
  DB  196,227,61,24,219,1                 ; vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  DB  196,98,125,24,66,104                ; vbroadcastss  0x68(%rdx),%ymm8
  DB  197,60,84,195                       ; vandps        %ymm3,%ymm8,%ymm8
  DB  196,65,124,91,192                   ; vcvtdq2ps     %ymm8,%ymm8
  DB  196,98,125,24,74,116                ; vbroadcastss  0x74(%rdx),%ymm9
  DB  196,65,52,89,192                    ; vmulps        %ymm8,%ymm9,%ymm8
  DB  196,98,125,24,74,108                ; vbroadcastss  0x6c(%rdx),%ymm9
  DB  197,52,84,203                       ; vandps        %ymm3,%ymm9,%ymm9
  DB  196,65,124,91,201                   ; vcvtdq2ps     %ymm9,%ymm9
  DB  196,98,125,24,82,120                ; vbroadcastss  0x78(%rdx),%ymm10
  DB  196,65,44,89,201                    ; vmulps        %ymm9,%ymm10,%ymm9
  DB  196,98,125,24,82,112                ; vbroadcastss  0x70(%rdx),%ymm10
  DB  197,172,84,219                      ; vandps        %ymm3,%ymm10,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,98,125,24,82,124                ; vbroadcastss  0x7c(%rdx),%ymm10
  DB  197,172,89,219                      ; vmulps        %ymm3,%ymm10,%ymm3
  DB  197,252,92,196                      ; vsubps        %ymm4,%ymm0,%ymm0
  DB  196,193,124,89,192                  ; vmulps        %ymm8,%ymm0,%ymm0
  DB  197,252,88,196                      ; vaddps        %ymm4,%ymm0,%ymm0
  DB  197,244,92,205                      ; vsubps        %ymm5,%ymm1,%ymm1
  DB  196,193,116,89,201                  ; vmulps        %ymm9,%ymm1,%ymm1
  DB  197,244,88,205                      ; vaddps        %ymm5,%ymm1,%ymm1
  DB  197,236,92,214                      ; vsubps        %ymm6,%ymm2,%ymm2
  DB  197,236,89,211                      ; vmulps        %ymm3,%ymm2,%ymm2
  DB  197,236,88,214                      ; vaddps        %ymm6,%ymm2,%ymm2
  DB  196,226,125,24,26                   ; vbroadcastss  (%rdx),%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  65,128,224,7                        ; and           $0x7,%r8b
  DB  196,65,57,239,192                   ; vpxor         %xmm8,%xmm8,%xmm8
  DB  65,254,200                          ; dec           %r8b
  DB  69,15,182,192                       ; movzbl        %r8b,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  15,135,85,255,255,255               ; ja            642 <_sk_lerp_565_avx+0x14>
  DB  76,141,13,76,0,0,0                  ; lea           0x4c(%rip),%r9        # 740 <_sk_lerp_565_avx+0x112>
  DB  75,99,4,129                         ; movslq        (%r9,%r8,4),%rax
  DB  76,1,200                            ; add           %r9,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  197,225,239,219                     ; vpxor         %xmm3,%xmm3,%xmm3
  DB  196,65,97,196,68,122,12,6           ; vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm3,%xmm8
  DB  196,65,57,196,68,122,10,5           ; vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm8,%xmm8
  DB  196,65,57,196,68,122,8,4            ; vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm8,%xmm8
  DB  196,65,57,196,68,122,6,3            ; vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm8,%xmm8
  DB  196,65,57,196,68,122,4,2            ; vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm8,%xmm8
  DB  196,65,57,196,68,122,2,1            ; vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm8,%xmm8
  DB  196,65,57,196,4,122,0               ; vpinsrw       $0x0,(%r10,%rdi,2),%xmm8,%xmm8
  DB  233,5,255,255,255                   ; jmpq          642 <_sk_lerp_565_avx+0x14>
  DB  15,31,0                             ; nopl          (%rax)
  DB  241                                 ; icebp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  233,255,255,255,225                 ; jmpq          ffffffffe2000748 <_sk_linear_gradient_2stops_avx+0xffffffffe1fff1b4>
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  217,255                             ; fcos
  DB  255                                 ; (bad)
  DB  255,209                             ; callq         *%rcx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,201                             ; dec           %ecx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  189                                 ; .byte         0xbd
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_tables_avx
_sk_load_tables_avx LABEL PROC
  DB  85                                  ; push          %rbp
  DB  65,87                               ; push          %r15
  DB  65,86                               ; push          %r14
  DB  65,85                               ; push          %r13
  DB  65,84                               ; push          %r12
  DB  83                                  ; push          %rbx
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,0                            ; mov           (%rax),%r8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  15,133,18,2,0,0                     ; jne           986 <_sk_load_tables_avx+0x22a>
  DB  196,65,124,16,4,184                 ; vmovups       (%r8,%rdi,4),%ymm8
  DB  196,98,125,24,74,16                 ; vbroadcastss  0x10(%rdx),%ymm9
  DB  196,193,52,84,192                   ; vandps        %ymm8,%ymm9,%ymm0
  DB  196,193,249,126,193                 ; vmovq         %xmm0,%r9
  DB  69,137,203                          ; mov           %r9d,%r11d
  DB  196,195,249,22,194,1                ; vpextrq       $0x1,%xmm0,%r10
  DB  69,137,214                          ; mov           %r10d,%r14d
  DB  73,193,234,32                       ; shr           $0x20,%r10
  DB  73,193,233,32                       ; shr           $0x20,%r9
  DB  196,227,125,25,192,1                ; vextractf128  $0x1,%ymm0,%xmm0
  DB  196,193,249,126,196                 ; vmovq         %xmm0,%r12
  DB  69,137,231                          ; mov           %r12d,%r15d
  DB  196,227,249,22,195,1                ; vpextrq       $0x1,%xmm0,%rbx
  DB  65,137,221                          ; mov           %ebx,%r13d
  DB  72,193,235,32                       ; shr           $0x20,%rbx
  DB  73,193,236,32                       ; shr           $0x20,%r12
  DB  72,139,104,8                        ; mov           0x8(%rax),%rbp
  DB  76,139,64,16                        ; mov           0x10(%rax),%r8
  DB  196,161,122,16,68,189,0             ; vmovss        0x0(%rbp,%r15,4),%xmm0
  DB  196,163,121,33,68,165,0,16          ; vinsertps     $0x10,0x0(%rbp,%r12,4),%xmm0,%xmm0
  DB  196,163,121,33,68,173,0,32          ; vinsertps     $0x20,0x0(%rbp,%r13,4),%xmm0,%xmm0
  DB  197,250,16,76,157,0                 ; vmovss        0x0(%rbp,%rbx,4),%xmm1
  DB  196,227,121,33,193,48               ; vinsertps     $0x30,%xmm1,%xmm0,%xmm0
  DB  196,161,122,16,76,157,0             ; vmovss        0x0(%rbp,%r11,4),%xmm1
  DB  196,163,113,33,76,141,0,16          ; vinsertps     $0x10,0x0(%rbp,%r9,4),%xmm1,%xmm1
  DB  196,163,113,33,76,181,0,32          ; vinsertps     $0x20,0x0(%rbp,%r14,4),%xmm1,%xmm1
  DB  196,161,122,16,92,149,0             ; vmovss        0x0(%rbp,%r10,4),%xmm3
  DB  196,227,113,33,203,48               ; vinsertps     $0x30,%xmm3,%xmm1,%xmm1
  DB  196,227,117,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  DB  196,193,113,114,208,8               ; vpsrld        $0x8,%xmm8,%xmm1
  DB  196,67,125,25,194,1                 ; vextractf128  $0x1,%ymm8,%xmm10
  DB  196,193,105,114,210,8               ; vpsrld        $0x8,%xmm10,%xmm2
  DB  196,227,117,24,202,1                ; vinsertf128   $0x1,%xmm2,%ymm1,%ymm1
  DB  197,180,84,201                      ; vandps        %ymm1,%ymm9,%ymm1
  DB  196,193,249,126,201                 ; vmovq         %xmm1,%r9
  DB  69,137,203                          ; mov           %r9d,%r11d
  DB  196,195,249,22,202,1                ; vpextrq       $0x1,%xmm1,%r10
  DB  69,137,214                          ; mov           %r10d,%r14d
  DB  73,193,234,32                       ; shr           $0x20,%r10
  DB  73,193,233,32                       ; shr           $0x20,%r9
  DB  196,227,125,25,201,1                ; vextractf128  $0x1,%ymm1,%xmm1
  DB  196,225,249,126,205                 ; vmovq         %xmm1,%rbp
  DB  65,137,239                          ; mov           %ebp,%r15d
  DB  196,227,249,22,203,1                ; vpextrq       $0x1,%xmm1,%rbx
  DB  65,137,220                          ; mov           %ebx,%r12d
  DB  72,193,235,32                       ; shr           $0x20,%rbx
  DB  72,193,237,32                       ; shr           $0x20,%rbp
  DB  196,129,122,16,12,184               ; vmovss        (%r8,%r15,4),%xmm1
  DB  196,195,113,33,12,168,16            ; vinsertps     $0x10,(%r8,%rbp,4),%xmm1,%xmm1
  DB  196,129,122,16,20,160               ; vmovss        (%r8,%r12,4),%xmm2
  DB  196,227,113,33,202,32               ; vinsertps     $0x20,%xmm2,%xmm1,%xmm1
  DB  196,193,122,16,20,152               ; vmovss        (%r8,%rbx,4),%xmm2
  DB  196,227,113,33,202,48               ; vinsertps     $0x30,%xmm2,%xmm1,%xmm1
  DB  196,129,122,16,20,152               ; vmovss        (%r8,%r11,4),%xmm2
  DB  196,131,105,33,20,136,16            ; vinsertps     $0x10,(%r8,%r9,4),%xmm2,%xmm2
  DB  196,129,122,16,28,176               ; vmovss        (%r8,%r14,4),%xmm3
  DB  196,227,105,33,211,32               ; vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  DB  196,129,122,16,28,144               ; vmovss        (%r8,%r10,4),%xmm3
  DB  196,227,105,33,211,48               ; vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  DB  196,227,109,24,201,1                ; vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  DB  72,139,64,24                        ; mov           0x18(%rax),%rax
  DB  196,193,105,114,208,16              ; vpsrld        $0x10,%xmm8,%xmm2
  DB  196,193,97,114,210,16               ; vpsrld        $0x10,%xmm10,%xmm3
  DB  196,227,109,24,211,1                ; vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  DB  197,180,84,210                      ; vandps        %ymm2,%ymm9,%ymm2
  DB  196,193,249,126,208                 ; vmovq         %xmm2,%r8
  DB  69,137,194                          ; mov           %r8d,%r10d
  DB  196,195,249,22,209,1                ; vpextrq       $0x1,%xmm2,%r9
  DB  69,137,203                          ; mov           %r9d,%r11d
  DB  73,193,233,32                       ; shr           $0x20,%r9
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  196,227,125,25,210,1                ; vextractf128  $0x1,%ymm2,%xmm2
  DB  196,225,249,126,213                 ; vmovq         %xmm2,%rbp
  DB  65,137,238                          ; mov           %ebp,%r14d
  DB  196,227,249,22,211,1                ; vpextrq       $0x1,%xmm2,%rbx
  DB  65,137,223                          ; mov           %ebx,%r15d
  DB  72,193,235,32                       ; shr           $0x20,%rbx
  DB  72,193,237,32                       ; shr           $0x20,%rbp
  DB  196,161,122,16,20,176               ; vmovss        (%rax,%r14,4),%xmm2
  DB  196,227,105,33,20,168,16            ; vinsertps     $0x10,(%rax,%rbp,4),%xmm2,%xmm2
  DB  196,161,122,16,28,184               ; vmovss        (%rax,%r15,4),%xmm3
  DB  196,227,105,33,211,32               ; vinsertps     $0x20,%xmm3,%xmm2,%xmm2
  DB  197,250,16,28,152                   ; vmovss        (%rax,%rbx,4),%xmm3
  DB  196,99,105,33,203,48                ; vinsertps     $0x30,%xmm3,%xmm2,%xmm9
  DB  196,161,122,16,28,144               ; vmovss        (%rax,%r10,4),%xmm3
  DB  196,163,97,33,28,128,16             ; vinsertps     $0x10,(%rax,%r8,4),%xmm3,%xmm3
  DB  196,161,122,16,20,152               ; vmovss        (%rax,%r11,4),%xmm2
  DB  196,227,97,33,210,32                ; vinsertps     $0x20,%xmm2,%xmm3,%xmm2
  DB  196,161,122,16,28,136               ; vmovss        (%rax,%r9,4),%xmm3
  DB  196,227,105,33,211,48               ; vinsertps     $0x30,%xmm3,%xmm2,%xmm2
  DB  196,195,109,24,209,1                ; vinsertf128   $0x1,%xmm9,%ymm2,%ymm2
  DB  196,193,57,114,208,24               ; vpsrld        $0x18,%xmm8,%xmm8
  DB  196,193,97,114,210,24               ; vpsrld        $0x18,%xmm10,%xmm3
  DB  196,227,61,24,219,1                 ; vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,98,125,24,66,12                 ; vbroadcastss  0xc(%rdx),%ymm8
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  91                                  ; pop           %rbx
  DB  65,92                               ; pop           %r12
  DB  65,93                               ; pop           %r13
  DB  65,94                               ; pop           %r14
  DB  65,95                               ; pop           %r15
  DB  93                                  ; pop           %rbp
  DB  255,224                             ; jmpq          *%rax
  DB  65,137,201                          ; mov           %ecx,%r9d
  DB  65,128,225,7                        ; and           $0x7,%r9b
  DB  196,65,60,87,192                    ; vxorps        %ymm8,%ymm8,%ymm8
  DB  65,254,201                          ; dec           %r9b
  DB  69,15,182,201                       ; movzbl        %r9b,%r9d
  DB  65,128,249,6                        ; cmp           $0x6,%r9b
  DB  15,135,215,253,255,255              ; ja            77a <_sk_load_tables_avx+0x1e>
  DB  76,141,21,138,0,0,0                 ; lea           0x8a(%rip),%r10        # a34 <_sk_load_tables_avx+0x2d8>
  DB  79,99,12,138                        ; movslq        (%r10,%r9,4),%r9
  DB  77,1,209                            ; add           %r10,%r9
  DB  65,255,225                          ; jmpq          *%r9
  DB  196,193,121,110,68,184,24           ; vmovd         0x18(%r8,%rdi,4),%xmm0
  DB  197,249,112,192,68                  ; vpshufd       $0x44,%xmm0,%xmm0
  DB  196,227,125,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  196,99,117,12,192,64                ; vblendps      $0x40,%ymm0,%ymm1,%ymm8
  DB  196,99,125,25,192,1                 ; vextractf128  $0x1,%ymm8,%xmm0
  DB  196,195,121,34,68,184,20,1          ; vpinsrd       $0x1,0x14(%r8,%rdi,4),%xmm0,%xmm0
  DB  196,99,61,24,192,1                  ; vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  DB  196,99,125,25,192,1                 ; vextractf128  $0x1,%ymm8,%xmm0
  DB  196,195,121,34,68,184,16,0          ; vpinsrd       $0x0,0x10(%r8,%rdi,4),%xmm0,%xmm0
  DB  196,99,61,24,192,1                  ; vinsertf128   $0x1,%xmm0,%ymm8,%ymm8
  DB  196,195,57,34,68,184,12,3           ; vpinsrd       $0x3,0xc(%r8,%rdi,4),%xmm8,%xmm0
  DB  196,99,61,12,192,15                 ; vblendps      $0xf,%ymm0,%ymm8,%ymm8
  DB  196,195,57,34,68,184,8,2            ; vpinsrd       $0x2,0x8(%r8,%rdi,4),%xmm8,%xmm0
  DB  196,99,61,12,192,15                 ; vblendps      $0xf,%ymm0,%ymm8,%ymm8
  DB  196,195,57,34,68,184,4,1            ; vpinsrd       $0x1,0x4(%r8,%rdi,4),%xmm8,%xmm0
  DB  196,99,61,12,192,15                 ; vblendps      $0xf,%ymm0,%ymm8,%ymm8
  DB  196,195,57,34,4,184,0               ; vpinsrd       $0x0,(%r8,%rdi,4),%xmm8,%xmm0
  DB  196,99,61,12,192,15                 ; vblendps      $0xf,%ymm0,%ymm8,%ymm8
  DB  233,70,253,255,255                  ; jmpq          77a <_sk_load_tables_avx+0x1e>
  DB  238                                 ; out           %al,(%dx)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,224                             ; jmpq          *%rax
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,210                             ; callq         *%rdx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,196                             ; inc           %esp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,176,255,255,255,156             ; pushq         -0x63000001(%rax)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff
  DB  128,255,255                         ; cmp           $0xff,%bh
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_a8_avx
_sk_load_a8_avx LABEL PROC
  DB  73,137,200                          ; mov           %rcx,%r8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  72,1,248                            ; add           %rdi,%rax
  DB  77,133,192                          ; test          %r8,%r8
  DB  117,59                              ; jne           a9b <_sk_load_a8_avx+0x4b>
  DB  197,251,16,0                        ; vmovsd        (%rax),%xmm0
  DB  196,226,121,49,200                  ; vpmovzxbd     %xmm0,%xmm1
  DB  196,227,121,4,192,229               ; vpermilps     $0xe5,%xmm0,%xmm0
  DB  196,226,121,49,192                  ; vpmovzxbd     %xmm0,%xmm0
  DB  196,227,117,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm1,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,74,12                ; vbroadcastss  0xc(%rdx),%ymm1
  DB  197,252,89,217                      ; vmulps        %ymm1,%ymm0,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  197,252,87,192                      ; vxorps        %ymm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  197,236,87,210                      ; vxorps        %ymm2,%ymm2,%ymm2
  DB  76,137,193                          ; mov           %r8,%rcx
  DB  255,224                             ; jmpq          *%rax
  DB  49,201                              ; xor           %ecx,%ecx
  DB  77,137,194                          ; mov           %r8,%r10
  DB  69,49,201                           ; xor           %r9d,%r9d
  DB  68,15,182,24                        ; movzbl        (%rax),%r11d
  DB  72,255,192                          ; inc           %rax
  DB  73,211,227                          ; shl           %cl,%r11
  DB  77,9,217                            ; or            %r11,%r9
  DB  72,131,193,8                        ; add           $0x8,%rcx
  DB  73,255,202                          ; dec           %r10
  DB  117,234                             ; jne           aa3 <_sk_load_a8_avx+0x53>
  DB  196,193,249,110,193                 ; vmovq         %r9,%xmm0
  DB  235,164                             ; jmp           a64 <_sk_load_a8_avx+0x14>

PUBLIC _sk_store_a8_avx
_sk_store_a8_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,8                            ; mov           (%rax),%r9
  DB  196,98,125,24,66,8                  ; vbroadcastss  0x8(%rdx),%ymm8
  DB  197,60,89,195                       ; vmulps        %ymm3,%ymm8,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,67,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm9
  DB  196,66,57,43,193                    ; vpackusdw     %xmm9,%xmm8,%xmm8
  DB  196,65,57,103,192                   ; vpackuswb     %xmm8,%xmm8,%xmm8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,10                              ; jne           af3 <_sk_store_a8_avx+0x33>
  DB  196,65,123,17,4,57                  ; vmovsd        %xmm8,(%r9,%rdi,1)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  137,200                             ; mov           %ecx,%eax
  DB  36,7                                ; and           $0x7,%al
  DB  254,200                             ; dec           %al
  DB  68,15,182,192                       ; movzbl        %al,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  119,236                             ; ja            aef <_sk_store_a8_avx+0x2f>
  DB  196,66,121,48,192                   ; vpmovzxbw     %xmm8,%xmm8
  DB  76,141,21,69,0,0,0                  ; lea           0x45(%rip),%r10        # b54 <_sk_store_a8_avx+0x94>
  DB  75,99,4,130                         ; movslq        (%r10,%r8,4),%rax
  DB  76,1,208                            ; add           %r10,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  196,67,121,20,68,57,6,12            ; vpextrb       $0xc,%xmm8,0x6(%r9,%rdi,1)
  DB  196,67,121,20,68,57,5,10            ; vpextrb       $0xa,%xmm8,0x5(%r9,%rdi,1)
  DB  196,67,121,20,68,57,4,8             ; vpextrb       $0x8,%xmm8,0x4(%r9,%rdi,1)
  DB  196,67,121,20,68,57,3,6             ; vpextrb       $0x6,%xmm8,0x3(%r9,%rdi,1)
  DB  196,67,121,20,68,57,2,4             ; vpextrb       $0x4,%xmm8,0x2(%r9,%rdi,1)
  DB  196,67,121,20,68,57,1,2             ; vpextrb       $0x2,%xmm8,0x1(%r9,%rdi,1)
  DB  196,67,121,20,4,57,0                ; vpextrb       $0x0,%xmm8,(%r9,%rdi,1)
  DB  235,158                             ; jmp           aef <_sk_store_a8_avx+0x2f>
  DB  15,31,0                             ; nopl          (%rax)
  DB  244                                 ; hlt
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  236                                 ; in            (%dx),%al
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,228                             ; jmpq          *%rsp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  220,255                             ; fdivr         %st,%st(7)
  DB  255                                 ; (bad)
  DB  255,212                             ; callq         *%rsp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,204                             ; dec           %esp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,196                             ; inc           %esp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_565_avx
_sk_load_565_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,16                           ; mov           (%rax),%r10
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,106                             ; jne           be4 <_sk_load_565_avx+0x74>
  DB  196,193,122,111,4,122               ; vmovdqu       (%r10,%rdi,2),%xmm0
  DB  197,241,239,201                     ; vpxor         %xmm1,%xmm1,%xmm1
  DB  197,249,105,201                     ; vpunpckhwd    %xmm1,%xmm0,%xmm1
  DB  196,226,121,51,192                  ; vpmovzxwd     %xmm0,%xmm0
  DB  196,227,125,24,209,1                ; vinsertf128   $0x1,%xmm1,%ymm0,%ymm2
  DB  196,226,125,24,66,104               ; vbroadcastss  0x68(%rdx),%ymm0
  DB  197,252,84,194                      ; vandps        %ymm2,%ymm0,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,226,125,24,74,116               ; vbroadcastss  0x74(%rdx),%ymm1
  DB  197,244,89,192                      ; vmulps        %ymm0,%ymm1,%ymm0
  DB  196,226,125,24,74,108               ; vbroadcastss  0x6c(%rdx),%ymm1
  DB  197,244,84,202                      ; vandps        %ymm2,%ymm1,%ymm1
  DB  197,252,91,201                      ; vcvtdq2ps     %ymm1,%ymm1
  DB  196,226,125,24,90,120               ; vbroadcastss  0x78(%rdx),%ymm3
  DB  197,228,89,201                      ; vmulps        %ymm1,%ymm3,%ymm1
  DB  196,226,125,24,90,112               ; vbroadcastss  0x70(%rdx),%ymm3
  DB  197,228,84,210                      ; vandps        %ymm2,%ymm3,%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  196,226,125,24,90,124               ; vbroadcastss  0x7c(%rdx),%ymm3
  DB  197,228,89,210                      ; vmulps        %ymm2,%ymm3,%ymm2
  DB  196,226,125,24,26                   ; vbroadcastss  (%rdx),%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  65,128,224,7                        ; and           $0x7,%r8b
  DB  197,249,239,192                     ; vpxor         %xmm0,%xmm0,%xmm0
  DB  65,254,200                          ; dec           %r8b
  DB  69,15,182,192                       ; movzbl        %r8b,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  119,132                             ; ja            b80 <_sk_load_565_avx+0x10>
  DB  76,141,13,73,0,0,0                  ; lea           0x49(%rip),%r9        # c4c <_sk_load_565_avx+0xdc>
  DB  75,99,4,129                         ; movslq        (%r9,%r8,4),%rax
  DB  76,1,200                            ; add           %r9,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  197,249,239,192                     ; vpxor         %xmm0,%xmm0,%xmm0
  DB  196,193,121,196,68,122,12,6         ; vpinsrw       $0x6,0xc(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,10,5         ; vpinsrw       $0x5,0xa(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,8,4          ; vpinsrw       $0x4,0x8(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,6,3          ; vpinsrw       $0x3,0x6(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,4,2          ; vpinsrw       $0x2,0x4(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,68,122,2,1          ; vpinsrw       $0x1,0x2(%r10,%rdi,2),%xmm0,%xmm0
  DB  196,193,121,196,4,122,0             ; vpinsrw       $0x0,(%r10,%rdi,2),%xmm0,%xmm0
  DB  233,52,255,255,255                  ; jmpq          b80 <_sk_load_565_avx+0x10>
  DB  244                                 ; hlt
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  236                                 ; in            (%dx),%al
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,228                             ; jmpq          *%rsp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  220,255                             ; fdivr         %st,%st(7)
  DB  255                                 ; (bad)
  DB  255,212                             ; callq         *%rsp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,204                             ; dec           %esp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,192                             ; inc           %eax
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_store_565_avx
_sk_store_565_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,8                            ; mov           (%rax),%r9
  DB  196,98,125,24,130,128,0,0,0         ; vbroadcastss  0x80(%rdx),%ymm8
  DB  197,60,89,200                       ; vmulps        %ymm0,%ymm8,%ymm9
  DB  196,65,125,91,201                   ; vcvtps2dq     %ymm9,%ymm9
  DB  196,193,41,114,241,11               ; vpslld        $0xb,%xmm9,%xmm10
  DB  196,67,125,25,201,1                 ; vextractf128  $0x1,%ymm9,%xmm9
  DB  196,193,49,114,241,11               ; vpslld        $0xb,%xmm9,%xmm9
  DB  196,67,45,24,201,1                  ; vinsertf128   $0x1,%xmm9,%ymm10,%ymm9
  DB  196,98,125,24,146,132,0,0,0         ; vbroadcastss  0x84(%rdx),%ymm10
  DB  197,44,89,209                       ; vmulps        %ymm1,%ymm10,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,33,114,242,5                ; vpslld        $0x5,%xmm10,%xmm11
  DB  196,67,125,25,210,1                 ; vextractf128  $0x1,%ymm10,%xmm10
  DB  196,193,41,114,242,5                ; vpslld        $0x5,%xmm10,%xmm10
  DB  196,67,37,24,210,1                  ; vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  DB  196,65,45,86,201                    ; vorpd         %ymm9,%ymm10,%ymm9
  DB  197,60,89,194                       ; vmulps        %ymm2,%ymm8,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,65,53,86,192                    ; vorpd         %ymm8,%ymm9,%ymm8
  DB  196,67,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm9
  DB  196,66,57,43,193                    ; vpackusdw     %xmm9,%xmm8,%xmm8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,10                              ; jne           cee <_sk_store_565_avx+0x86>
  DB  196,65,122,127,4,121                ; vmovdqu       %xmm8,(%r9,%rdi,2)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  137,200                             ; mov           %ecx,%eax
  DB  36,7                                ; and           $0x7,%al
  DB  254,200                             ; dec           %al
  DB  68,15,182,192                       ; movzbl        %al,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  119,236                             ; ja            cea <_sk_store_565_avx+0x82>
  DB  76,141,21,71,0,0,0                  ; lea           0x47(%rip),%r10        # d4c <_sk_store_565_avx+0xe4>
  DB  75,99,4,130                         ; movslq        (%r10,%r8,4),%rax
  DB  76,1,208                            ; add           %r10,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  196,67,121,21,68,121,12,6           ; vpextrw       $0x6,%xmm8,0xc(%r9,%rdi,2)
  DB  196,67,121,21,68,121,10,5           ; vpextrw       $0x5,%xmm8,0xa(%r9,%rdi,2)
  DB  196,67,121,21,68,121,8,4            ; vpextrw       $0x4,%xmm8,0x8(%r9,%rdi,2)
  DB  196,67,121,21,68,121,6,3            ; vpextrw       $0x3,%xmm8,0x6(%r9,%rdi,2)
  DB  196,67,121,21,68,121,4,2            ; vpextrw       $0x2,%xmm8,0x4(%r9,%rdi,2)
  DB  196,67,121,21,68,121,2,1            ; vpextrw       $0x1,%xmm8,0x2(%r9,%rdi,2)
  DB  197,121,126,192                     ; vmovd         %xmm8,%eax
  DB  102,65,137,4,121                    ; mov           %ax,(%r9,%rdi,2)
  DB  235,161                             ; jmp           cea <_sk_store_565_avx+0x82>
  DB  15,31,0                             ; nopl          (%rax)
  DB  242,255                             ; repnz         (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  234                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,226                             ; jmpq          *%rdx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  218,255                             ; (bad)
  DB  255                                 ; (bad)
  DB  255,210                             ; callq         *%rdx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,202                             ; dec           %edx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,194                             ; inc           %edx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_8888_avx
_sk_load_8888_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,16                           ; mov           (%rax),%r10
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,125                             ; jne           def <_sk_load_8888_avx+0x87>
  DB  196,65,124,16,12,186                ; vmovups       (%r10,%rdi,4),%ymm9
  DB  196,98,125,24,90,16                 ; vbroadcastss  0x10(%rdx),%ymm11
  DB  196,193,36,84,193                   ; vandps        %ymm9,%ymm11,%ymm0
  DB  197,252,91,192                      ; vcvtdq2ps     %ymm0,%ymm0
  DB  196,98,125,24,66,12                 ; vbroadcastss  0xc(%rdx),%ymm8
  DB  197,188,89,192                      ; vmulps        %ymm0,%ymm8,%ymm0
  DB  196,193,41,114,209,8                ; vpsrld        $0x8,%xmm9,%xmm10
  DB  196,99,125,25,203,1                 ; vextractf128  $0x1,%ymm9,%xmm3
  DB  197,241,114,211,8                   ; vpsrld        $0x8,%xmm3,%xmm1
  DB  196,227,45,24,201,1                 ; vinsertf128   $0x1,%xmm1,%ymm10,%ymm1
  DB  197,164,84,201                      ; vandps        %ymm1,%ymm11,%ymm1
  DB  197,252,91,201                      ; vcvtdq2ps     %ymm1,%ymm1
  DB  197,188,89,201                      ; vmulps        %ymm1,%ymm8,%ymm1
  DB  196,193,41,114,209,16               ; vpsrld        $0x10,%xmm9,%xmm10
  DB  197,233,114,211,16                  ; vpsrld        $0x10,%xmm3,%xmm2
  DB  196,227,45,24,210,1                 ; vinsertf128   $0x1,%xmm2,%ymm10,%ymm2
  DB  197,164,84,210                      ; vandps        %ymm2,%ymm11,%ymm2
  DB  197,252,91,210                      ; vcvtdq2ps     %ymm2,%ymm2
  DB  197,188,89,210                      ; vmulps        %ymm2,%ymm8,%ymm2
  DB  196,193,49,114,209,24               ; vpsrld        $0x18,%xmm9,%xmm9
  DB  197,225,114,211,24                  ; vpsrld        $0x18,%xmm3,%xmm3
  DB  196,227,53,24,219,1                 ; vinsertf128   $0x1,%xmm3,%ymm9,%ymm3
  DB  197,252,91,219                      ; vcvtdq2ps     %ymm3,%ymm3
  DB  196,193,100,89,216                  ; vmulps        %ymm8,%ymm3,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  65,128,224,7                        ; and           $0x7,%r8b
  DB  196,65,52,87,201                    ; vxorps        %ymm9,%ymm9,%ymm9
  DB  65,254,200                          ; dec           %r8b
  DB  69,15,182,192                       ; movzbl        %r8b,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  15,135,108,255,255,255              ; ja            d78 <_sk_load_8888_avx+0x10>
  DB  76,141,13,137,0,0,0                 ; lea           0x89(%rip),%r9        # e9c <_sk_load_8888_avx+0x134>
  DB  75,99,4,129                         ; movslq        (%r9,%r8,4),%rax
  DB  76,1,200                            ; add           %r9,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  196,193,121,110,68,186,24           ; vmovd         0x18(%r10,%rdi,4),%xmm0
  DB  197,249,112,192,68                  ; vpshufd       $0x44,%xmm0,%xmm0
  DB  196,227,125,24,192,1                ; vinsertf128   $0x1,%xmm0,%ymm0,%ymm0
  DB  197,244,87,201                      ; vxorps        %ymm1,%ymm1,%ymm1
  DB  196,99,117,12,200,64                ; vblendps      $0x40,%ymm0,%ymm1,%ymm9
  DB  196,99,125,25,200,1                 ; vextractf128  $0x1,%ymm9,%xmm0
  DB  196,195,121,34,68,186,20,1          ; vpinsrd       $0x1,0x14(%r10,%rdi,4),%xmm0,%xmm0
  DB  196,99,53,24,200,1                  ; vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  DB  196,99,125,25,200,1                 ; vextractf128  $0x1,%ymm9,%xmm0
  DB  196,195,121,34,68,186,16,0          ; vpinsrd       $0x0,0x10(%r10,%rdi,4),%xmm0,%xmm0
  DB  196,99,53,24,200,1                  ; vinsertf128   $0x1,%xmm0,%ymm9,%ymm9
  DB  196,195,49,34,68,186,12,3           ; vpinsrd       $0x3,0xc(%r10,%rdi,4),%xmm9,%xmm0
  DB  196,99,53,12,200,15                 ; vblendps      $0xf,%ymm0,%ymm9,%ymm9
  DB  196,195,49,34,68,186,8,2            ; vpinsrd       $0x2,0x8(%r10,%rdi,4),%xmm9,%xmm0
  DB  196,99,53,12,200,15                 ; vblendps      $0xf,%ymm0,%ymm9,%ymm9
  DB  196,195,49,34,68,186,4,1            ; vpinsrd       $0x1,0x4(%r10,%rdi,4),%xmm9,%xmm0
  DB  196,99,53,12,200,15                 ; vblendps      $0xf,%ymm0,%ymm9,%ymm9
  DB  196,195,49,34,4,186,0               ; vpinsrd       $0x0,(%r10,%rdi,4),%xmm9,%xmm0
  DB  196,99,53,12,200,15                 ; vblendps      $0xf,%ymm0,%ymm9,%ymm9
  DB  233,220,254,255,255                 ; jmpq          d78 <_sk_load_8888_avx+0x10>
  DB  238                                 ; out           %al,(%dx)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,224                             ; jmpq          *%rax
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,210                             ; callq         *%rdx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,196                             ; inc           %esp
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,176,255,255,255,156             ; pushq         -0x63000001(%rax)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff
  DB  128,255,255                         ; cmp           $0xff,%bh
  DB  255                                 ; .byte         0xff

PUBLIC _sk_store_8888_avx
_sk_store_8888_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  76,139,8                            ; mov           (%rax),%r9
  DB  196,98,125,24,66,8                  ; vbroadcastss  0x8(%rdx),%ymm8
  DB  197,60,89,200                       ; vmulps        %ymm0,%ymm8,%ymm9
  DB  196,65,125,91,201                   ; vcvtps2dq     %ymm9,%ymm9
  DB  197,60,89,209                       ; vmulps        %ymm1,%ymm8,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,33,114,242,8                ; vpslld        $0x8,%xmm10,%xmm11
  DB  196,67,125,25,210,1                 ; vextractf128  $0x1,%ymm10,%xmm10
  DB  196,193,41,114,242,8                ; vpslld        $0x8,%xmm10,%xmm10
  DB  196,67,37,24,210,1                  ; vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  DB  196,65,45,86,201                    ; vorpd         %ymm9,%ymm10,%ymm9
  DB  197,60,89,210                       ; vmulps        %ymm2,%ymm8,%ymm10
  DB  196,65,125,91,210                   ; vcvtps2dq     %ymm10,%ymm10
  DB  196,193,33,114,242,16               ; vpslld        $0x10,%xmm10,%xmm11
  DB  196,67,125,25,210,1                 ; vextractf128  $0x1,%ymm10,%xmm10
  DB  196,193,41,114,242,16               ; vpslld        $0x10,%xmm10,%xmm10
  DB  196,67,37,24,210,1                  ; vinsertf128   $0x1,%xmm10,%ymm11,%ymm10
  DB  197,60,89,195                       ; vmulps        %ymm3,%ymm8,%ymm8
  DB  196,65,125,91,192                   ; vcvtps2dq     %ymm8,%ymm8
  DB  196,193,33,114,240,24               ; vpslld        $0x18,%xmm8,%xmm11
  DB  196,67,125,25,192,1                 ; vextractf128  $0x1,%ymm8,%xmm8
  DB  196,193,57,114,240,24               ; vpslld        $0x18,%xmm8,%xmm8
  DB  196,67,37,24,192,1                  ; vinsertf128   $0x1,%xmm8,%ymm11,%ymm8
  DB  196,65,45,86,192                    ; vorpd         %ymm8,%ymm10,%ymm8
  DB  196,65,53,86,192                    ; vorpd         %ymm8,%ymm9,%ymm8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,10                              ; jne           f4d <_sk_store_8888_avx+0x95>
  DB  196,65,124,17,4,185                 ; vmovups       %ymm8,(%r9,%rdi,4)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  137,200                             ; mov           %ecx,%eax
  DB  36,7                                ; and           $0x7,%al
  DB  254,200                             ; dec           %al
  DB  68,15,182,192                       ; movzbl        %al,%r8d
  DB  65,128,248,6                        ; cmp           $0x6,%r8b
  DB  119,236                             ; ja            f49 <_sk_store_8888_avx+0x91>
  DB  76,141,21,84,0,0,0                  ; lea           0x54(%rip),%r10        # fb8 <_sk_store_8888_avx+0x100>
  DB  75,99,4,130                         ; movslq        (%r10,%r8,4),%rax
  DB  76,1,208                            ; add           %r10,%rax
  DB  255,224                             ; jmpq          *%rax
  DB  196,67,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm9
  DB  196,67,121,22,76,185,24,2           ; vpextrd       $0x2,%xmm9,0x18(%r9,%rdi,4)
  DB  196,67,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm9
  DB  196,67,121,22,76,185,20,1           ; vpextrd       $0x1,%xmm9,0x14(%r9,%rdi,4)
  DB  196,67,125,25,193,1                 ; vextractf128  $0x1,%ymm8,%xmm9
  DB  196,65,121,126,76,185,16            ; vmovd         %xmm9,0x10(%r9,%rdi,4)
  DB  196,67,121,22,68,185,12,3           ; vpextrd       $0x3,%xmm8,0xc(%r9,%rdi,4)
  DB  196,67,121,22,68,185,8,2            ; vpextrd       $0x2,%xmm8,0x8(%r9,%rdi,4)
  DB  196,67,121,22,68,185,4,1            ; vpextrd       $0x1,%xmm8,0x4(%r9,%rdi,4)
  DB  196,65,121,126,4,185                ; vmovd         %xmm8,(%r9,%rdi,4)
  DB  235,147                             ; jmp           f49 <_sk_store_8888_avx+0x91>
  DB  102,144                             ; xchg          %ax,%ax
  DB  246,255                             ; idiv          %bh
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  238                                 ; out           %al,(%dx)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,230                             ; jmpq          *%rsi
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  222,255                             ; fdivrp        %st,%st(7)
  DB  255                                 ; (bad)
  DB  255,209                             ; callq         *%rcx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255,195                             ; inc           %ebx
  DB  255                                 ; (bad)
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff
  DB  181,255                             ; mov           $0xff,%ch
  DB  255                                 ; (bad)
  DB  255                                 ; .byte         0xff

PUBLIC _sk_load_f16_avx
_sk_load_f16_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  15,133,240,0,0,0                    ; jne           10d2 <_sk_load_f16_avx+0xfe>
  DB  197,249,16,12,248                   ; vmovupd       (%rax,%rdi,8),%xmm1
  DB  197,249,16,84,248,16                ; vmovupd       0x10(%rax,%rdi,8),%xmm2
  DB  197,249,16,92,248,32                ; vmovupd       0x20(%rax,%rdi,8),%xmm3
  DB  197,121,16,68,248,48                ; vmovupd       0x30(%rax,%rdi,8),%xmm8
  DB  197,241,97,194                      ; vpunpcklwd    %xmm2,%xmm1,%xmm0
  DB  197,241,105,202                     ; vpunpckhwd    %xmm2,%xmm1,%xmm1
  DB  196,193,97,97,208                   ; vpunpcklwd    %xmm8,%xmm3,%xmm2
  DB  196,193,97,105,216                  ; vpunpckhwd    %xmm8,%xmm3,%xmm3
  DB  197,121,97,193                      ; vpunpcklwd    %xmm1,%xmm0,%xmm8
  DB  197,249,105,193                     ; vpunpckhwd    %xmm1,%xmm0,%xmm0
  DB  197,233,97,203                      ; vpunpcklwd    %xmm3,%xmm2,%xmm1
  DB  197,105,105,203                     ; vpunpckhwd    %xmm3,%xmm2,%xmm9
  DB  197,249,110,90,100                  ; vmovd         0x64(%rdx),%xmm3
  DB  197,249,112,219,0                   ; vpshufd       $0x0,%xmm3,%xmm3
  DB  196,193,97,101,208                  ; vpcmpgtw      %xmm8,%xmm3,%xmm2
  DB  196,65,105,223,192                  ; vpandn        %xmm8,%xmm2,%xmm8
  DB  197,225,101,208                     ; vpcmpgtw      %xmm0,%xmm3,%xmm2
  DB  197,233,223,192                     ; vpandn        %xmm0,%xmm2,%xmm0
  DB  197,225,101,209                     ; vpcmpgtw      %xmm1,%xmm3,%xmm2
  DB  197,233,223,201                     ; vpandn        %xmm1,%xmm2,%xmm1
  DB  196,193,97,101,209                  ; vpcmpgtw      %xmm9,%xmm3,%xmm2
  DB  196,193,105,223,209                 ; vpandn        %xmm9,%xmm2,%xmm2
  DB  196,66,121,51,208                   ; vpmovzxwd     %xmm8,%xmm10
  DB  196,98,121,51,201                   ; vpmovzxwd     %xmm1,%xmm9
  DB  197,225,239,219                     ; vpxor         %xmm3,%xmm3,%xmm3
  DB  197,57,105,195                      ; vpunpckhwd    %xmm3,%xmm8,%xmm8
  DB  197,241,105,203                     ; vpunpckhwd    %xmm3,%xmm1,%xmm1
  DB  196,98,121,51,216                   ; vpmovzxwd     %xmm0,%xmm11
  DB  196,98,121,51,226                   ; vpmovzxwd     %xmm2,%xmm12
  DB  197,121,105,235                     ; vpunpckhwd    %xmm3,%xmm0,%xmm13
  DB  197,105,105,243                     ; vpunpckhwd    %xmm3,%xmm2,%xmm14
  DB  196,193,121,114,242,13              ; vpslld        $0xd,%xmm10,%xmm0
  DB  196,193,105,114,241,13              ; vpslld        $0xd,%xmm9,%xmm2
  DB  196,227,125,24,194,1                ; vinsertf128   $0x1,%xmm2,%ymm0,%ymm0
  DB  196,98,125,24,74,92                 ; vbroadcastss  0x5c(%rdx),%ymm9
  DB  197,180,89,192                      ; vmulps        %ymm0,%ymm9,%ymm0
  DB  196,193,105,114,240,13              ; vpslld        $0xd,%xmm8,%xmm2
  DB  197,241,114,241,13                  ; vpslld        $0xd,%xmm1,%xmm1
  DB  196,227,109,24,201,1                ; vinsertf128   $0x1,%xmm1,%ymm2,%ymm1
  DB  197,180,89,201                      ; vmulps        %ymm1,%ymm9,%ymm1
  DB  196,193,105,114,243,13              ; vpslld        $0xd,%xmm11,%xmm2
  DB  196,193,97,114,244,13               ; vpslld        $0xd,%xmm12,%xmm3
  DB  196,227,109,24,211,1                ; vinsertf128   $0x1,%xmm3,%ymm2,%ymm2
  DB  197,180,89,210                      ; vmulps        %ymm2,%ymm9,%ymm2
  DB  196,193,57,114,245,13               ; vpslld        $0xd,%xmm13,%xmm8
  DB  196,193,97,114,246,13               ; vpslld        $0xd,%xmm14,%xmm3
  DB  196,227,61,24,219,1                 ; vinsertf128   $0x1,%xmm3,%ymm8,%ymm3
  DB  197,180,89,219                      ; vmulps        %ymm3,%ymm9,%ymm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  197,251,16,12,248                   ; vmovsd        (%rax,%rdi,8),%xmm1
  DB  196,65,57,87,192                    ; vxorpd        %xmm8,%xmm8,%xmm8
  DB  72,131,249,1                        ; cmp           $0x1,%rcx
  DB  117,6                               ; jne           10e8 <_sk_load_f16_avx+0x114>
  DB  197,250,126,201                     ; vmovq         %xmm1,%xmm1
  DB  235,30                              ; jmp           1106 <_sk_load_f16_avx+0x132>
  DB  197,241,22,76,248,8                 ; vmovhpd       0x8(%rax,%rdi,8),%xmm1,%xmm1
  DB  72,131,249,3                        ; cmp           $0x3,%rcx
  DB  114,18                              ; jb            1106 <_sk_load_f16_avx+0x132>
  DB  197,251,16,84,248,16                ; vmovsd        0x10(%rax,%rdi,8),%xmm2
  DB  72,131,249,3                        ; cmp           $0x3,%rcx
  DB  117,19                              ; jne           1113 <_sk_load_f16_avx+0x13f>
  DB  197,250,126,210                     ; vmovq         %xmm2,%xmm2
  DB  235,46                              ; jmp           1134 <_sk_load_f16_avx+0x160>
  DB  197,225,87,219                      ; vxorpd        %xmm3,%xmm3,%xmm3
  DB  197,233,87,210                      ; vxorpd        %xmm2,%xmm2,%xmm2
  DB  233,230,254,255,255                 ; jmpq          ff9 <_sk_load_f16_avx+0x25>
  DB  197,233,22,84,248,24                ; vmovhpd       0x18(%rax,%rdi,8),%xmm2,%xmm2
  DB  72,131,249,5                        ; cmp           $0x5,%rcx
  DB  114,21                              ; jb            1134 <_sk_load_f16_avx+0x160>
  DB  197,251,16,92,248,32                ; vmovsd        0x20(%rax,%rdi,8),%xmm3
  DB  72,131,249,5                        ; cmp           $0x5,%rcx
  DB  117,18                              ; jne           113d <_sk_load_f16_avx+0x169>
  DB  197,250,126,219                     ; vmovq         %xmm3,%xmm3
  DB  233,197,254,255,255                 ; jmpq          ff9 <_sk_load_f16_avx+0x25>
  DB  197,225,87,219                      ; vxorpd        %xmm3,%xmm3,%xmm3
  DB  233,188,254,255,255                 ; jmpq          ff9 <_sk_load_f16_avx+0x25>
  DB  197,225,22,92,248,40                ; vmovhpd       0x28(%rax,%rdi,8),%xmm3,%xmm3
  DB  72,131,249,7                        ; cmp           $0x7,%rcx
  DB  15,130,172,254,255,255              ; jb            ff9 <_sk_load_f16_avx+0x25>
  DB  197,123,16,68,248,48                ; vmovsd        0x30(%rax,%rdi,8),%xmm8
  DB  233,161,254,255,255                 ; jmpq          ff9 <_sk_load_f16_avx+0x25>

PUBLIC _sk_store_f16_avx
_sk_store_f16_avx LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  196,98,125,24,66,96                 ; vbroadcastss  0x60(%rdx),%ymm8
  DB  197,60,89,200                       ; vmulps        %ymm0,%ymm8,%ymm9
  DB  196,67,125,25,202,1                 ; vextractf128  $0x1,%ymm9,%xmm10
  DB  196,193,41,114,210,13               ; vpsrld        $0xd,%xmm10,%xmm10
  DB  196,193,49,114,209,13               ; vpsrld        $0xd,%xmm9,%xmm9
  DB  197,60,89,217                       ; vmulps        %ymm1,%ymm8,%ymm11
  DB  196,67,125,25,220,1                 ; vextractf128  $0x1,%ymm11,%xmm12
  DB  196,193,25,114,212,13               ; vpsrld        $0xd,%xmm12,%xmm12
  DB  196,193,33,114,211,13               ; vpsrld        $0xd,%xmm11,%xmm11
  DB  197,60,89,234                       ; vmulps        %ymm2,%ymm8,%ymm13
  DB  196,67,125,25,238,1                 ; vextractf128  $0x1,%ymm13,%xmm14
  DB  196,193,9,114,214,13                ; vpsrld        $0xd,%xmm14,%xmm14
  DB  196,193,17,114,213,13               ; vpsrld        $0xd,%xmm13,%xmm13
  DB  197,60,89,195                       ; vmulps        %ymm3,%ymm8,%ymm8
  DB  196,67,125,25,199,1                 ; vextractf128  $0x1,%ymm8,%xmm15
  DB  196,193,1,114,215,13                ; vpsrld        $0xd,%xmm15,%xmm15
  DB  196,193,57,114,208,13               ; vpsrld        $0xd,%xmm8,%xmm8
  DB  196,193,33,115,251,2                ; vpslldq       $0x2,%xmm11,%xmm11
  DB  196,65,33,235,201                   ; vpor          %xmm9,%xmm11,%xmm9
  DB  196,193,33,115,252,2                ; vpslldq       $0x2,%xmm12,%xmm11
  DB  196,65,33,235,226                   ; vpor          %xmm10,%xmm11,%xmm12
  DB  196,193,57,115,248,2                ; vpslldq       $0x2,%xmm8,%xmm8
  DB  196,65,57,235,197                   ; vpor          %xmm13,%xmm8,%xmm8
  DB  196,193,41,115,255,2                ; vpslldq       $0x2,%xmm15,%xmm10
  DB  196,65,41,235,238                   ; vpor          %xmm14,%xmm10,%xmm13
  DB  196,65,49,98,216                    ; vpunpckldq    %xmm8,%xmm9,%xmm11
  DB  196,65,49,106,208                   ; vpunpckhdq    %xmm8,%xmm9,%xmm10
  DB  196,65,25,98,205                    ; vpunpckldq    %xmm13,%xmm12,%xmm9
  DB  196,65,25,106,197                   ; vpunpckhdq    %xmm13,%xmm12,%xmm8
  DB  72,133,201                          ; test          %rcx,%rcx
  DB  117,27                              ; jne           121b <_sk_store_f16_avx+0xc3>
  DB  197,120,17,28,248                   ; vmovups       %xmm11,(%rax,%rdi,8)
  DB  197,120,17,84,248,16                ; vmovups       %xmm10,0x10(%rax,%rdi,8)
  DB  197,120,17,76,248,32                ; vmovups       %xmm9,0x20(%rax,%rdi,8)
  DB  197,122,127,68,248,48               ; vmovdqu       %xmm8,0x30(%rax,%rdi,8)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax
  DB  197,121,214,28,248                  ; vmovq         %xmm11,(%rax,%rdi,8)
  DB  72,131,249,1                        ; cmp           $0x1,%rcx
  DB  116,241                             ; je            1217 <_sk_store_f16_avx+0xbf>
  DB  197,121,23,92,248,8                 ; vmovhpd       %xmm11,0x8(%rax,%rdi,8)
  DB  72,131,249,3                        ; cmp           $0x3,%rcx
  DB  114,229                             ; jb            1217 <_sk_store_f16_avx+0xbf>
  DB  197,121,214,84,248,16               ; vmovq         %xmm10,0x10(%rax,%rdi,8)
  DB  116,221                             ; je            1217 <_sk_store_f16_avx+0xbf>
  DB  197,121,23,84,248,24                ; vmovhpd       %xmm10,0x18(%rax,%rdi,8)
  DB  72,131,249,5                        ; cmp           $0x5,%rcx
  DB  114,209                             ; jb            1217 <_sk_store_f16_avx+0xbf>
  DB  197,121,214,76,248,32               ; vmovq         %xmm9,0x20(%rax,%rdi,8)
  DB  116,201                             ; je            1217 <_sk_store_f16_avx+0xbf>
  DB  197,121,23,76,248,40                ; vmovhpd       %xmm9,0x28(%rax,%rdi,8)
  DB  72,131,249,7                        ; cmp           $0x7,%rcx
  DB  114,189                             ; jb            1217 <_sk_store_f16_avx+0xbf>
  DB  197,121,214,68,248,48               ; vmovq         %xmm8,0x30(%rax,%rdi,8)
  DB  235,181                             ; jmp           1217 <_sk_store_f16_avx+0xbf>

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
  DB  15,91,200                           ; cvtdq2ps      %xmm0,%xmm1
  DB  243,15,16,18                        ; movss         (%rdx),%xmm2
  DB  243,15,16,90,4                      ; movss         0x4(%rdx),%xmm3
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  15,88,203                           ; addps         %xmm3,%xmm1
  DB  15,16,66,20                         ; movups        0x14(%rdx),%xmm0
  DB  15,88,193                           ; addps         %xmm1,%xmm0
  DB  102,15,110,8                        ; movd          (%rax),%xmm1
  DB  102,15,112,201,0                    ; pshufd        $0x0,%xmm1,%xmm1
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  15,88,203                           ; addps         %xmm3,%xmm1
  DB  15,198,210,0                        ; shufps        $0x0,%xmm2,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
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
  DB  243,68,15,16,2                      ; movss         (%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
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
  DB  243,68,15,16,2                      ; movss         (%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
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
  DB  243,68,15,16,2                      ; movss         (%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,93,192                        ; minps         %xmm8,%xmm0
  DB  65,15,93,200                        ; minps         %xmm8,%xmm1
  DB  65,15,93,208                        ; minps         %xmm8,%xmm2
  DB  65,15,93,216                        ; minps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_a_sse41
_sk_clamp_a_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,2                      ; movss         (%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,93,216                        ; minps         %xmm8,%xmm3
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
  DB  243,68,15,16,18                     ; movss         (%rdx),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
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
  DB  243,68,15,16,90,64                  ; movss         0x40(%rdx),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,40,211                        ; movaps        %xmm11,%xmm10
  DB  68,15,89,208                        ; mulps         %xmm0,%xmm10
  DB  68,15,40,240                        ; movaps        %xmm0,%xmm14
  DB  69,15,89,246                        ; mulps         %xmm14,%xmm14
  DB  243,68,15,16,66,60                  ; movss         0x3c(%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  243,68,15,16,98,52                  ; movss         0x34(%rdx),%xmm12
  DB  243,68,15,16,106,56                 ; movss         0x38(%rdx),%xmm13
  DB  69,15,198,237,0                     ; shufps        $0x0,%xmm13,%xmm13
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,200                        ; mulps         %xmm0,%xmm9
  DB  69,15,88,205                        ; addps         %xmm13,%xmm9
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  69,15,89,206                        ; mulps         %xmm14,%xmm9
  DB  69,15,88,204                        ; addps         %xmm12,%xmm9
  DB  243,68,15,16,114,68                 ; movss         0x44(%rdx),%xmm14
  DB  69,15,198,246,0                     ; shufps        $0x0,%xmm14,%xmm14
  DB  65,15,194,198,1                     ; cmpltps       %xmm14,%xmm0
  DB  102,69,15,56,20,202                 ; blendvps      %xmm0,%xmm10,%xmm9
  DB  69,15,40,251                        ; movaps        %xmm11,%xmm15
  DB  68,15,89,249                        ; mulps         %xmm1,%xmm15
  DB  15,40,193                           ; movaps        %xmm1,%xmm0
  DB  15,89,192                           ; mulps         %xmm0,%xmm0
  DB  69,15,40,208                        ; movaps        %xmm8,%xmm10
  DB  68,15,89,209                        ; mulps         %xmm1,%xmm10
  DB  69,15,88,213                        ; addps         %xmm13,%xmm10
  DB  68,15,89,208                        ; mulps         %xmm0,%xmm10
  DB  69,15,88,212                        ; addps         %xmm12,%xmm10
  DB  65,15,194,206,1                     ; cmpltps       %xmm14,%xmm1
  DB  15,40,193                           ; movaps        %xmm1,%xmm0
  DB  102,69,15,56,20,215                 ; blendvps      %xmm0,%xmm15,%xmm10
  DB  68,15,89,218                        ; mulps         %xmm2,%xmm11
  DB  15,40,194                           ; movaps        %xmm2,%xmm0
  DB  15,89,192                           ; mulps         %xmm0,%xmm0
  DB  68,15,89,194                        ; mulps         %xmm2,%xmm8
  DB  69,15,88,197                        ; addps         %xmm13,%xmm8
  DB  68,15,89,192                        ; mulps         %xmm0,%xmm8
  DB  69,15,88,196                        ; addps         %xmm12,%xmm8
  DB  65,15,194,214,1                     ; cmpltps       %xmm14,%xmm2
  DB  15,40,194                           ; movaps        %xmm2,%xmm0
  DB  102,69,15,56,20,195                 ; blendvps      %xmm0,%xmm11,%xmm8
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,193                        ; movaps        %xmm9,%xmm0
  DB  65,15,40,202                        ; movaps        %xmm10,%xmm1
  DB  65,15,40,208                        ; movaps        %xmm8,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_to_srgb_sse41
_sk_to_srgb_sse41 LABEL PROC
  DB  72,131,236,24                       ; sub           $0x18,%rsp
  DB  15,41,60,36                         ; movaps        %xmm7,(%rsp)
  DB  15,40,254                           ; movaps        %xmm6,%xmm7
  DB  15,40,245                           ; movaps        %xmm5,%xmm6
  DB  15,40,236                           ; movaps        %xmm4,%xmm5
  DB  15,40,227                           ; movaps        %xmm3,%xmm4
  DB  68,15,40,194                        ; movaps        %xmm2,%xmm8
  DB  15,40,217                           ; movaps        %xmm1,%xmm3
  DB  15,82,208                           ; rsqrtps       %xmm0,%xmm2
  DB  68,15,83,202                        ; rcpps         %xmm2,%xmm9
  DB  68,15,82,210                        ; rsqrtps       %xmm2,%xmm10
  DB  243,15,16,18                        ; movss         (%rdx),%xmm2
  DB  243,68,15,16,90,72                  ; movss         0x48(%rdx),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  65,15,40,203                        ; movaps        %xmm11,%xmm1
  DB  15,89,200                           ; mulps         %xmm0,%xmm1
  DB  15,198,210,0                        ; shufps        $0x0,%xmm2,%xmm2
  DB  243,68,15,16,98,76                  ; movss         0x4c(%rdx),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  243,68,15,16,106,80                 ; movss         0x50(%rdx),%xmm13
  DB  69,15,198,237,0                     ; shufps        $0x0,%xmm13,%xmm13
  DB  243,68,15,16,114,84                 ; movss         0x54(%rdx),%xmm14
  DB  69,15,198,246,0                     ; shufps        $0x0,%xmm14,%xmm14
  DB  69,15,89,205                        ; mulps         %xmm13,%xmm9
  DB  69,15,88,206                        ; addps         %xmm14,%xmm9
  DB  69,15,89,212                        ; mulps         %xmm12,%xmm10
  DB  69,15,88,209                        ; addps         %xmm9,%xmm10
  DB  68,15,40,202                        ; movaps        %xmm2,%xmm9
  DB  69,15,93,202                        ; minps         %xmm10,%xmm9
  DB  243,68,15,16,122,88                 ; movss         0x58(%rdx),%xmm15
  DB  69,15,198,255,0                     ; shufps        $0x0,%xmm15,%xmm15
  DB  65,15,194,199,1                     ; cmpltps       %xmm15,%xmm0
  DB  102,68,15,56,20,201                 ; blendvps      %xmm0,%xmm1,%xmm9
  DB  15,82,195                           ; rsqrtps       %xmm3,%xmm0
  DB  15,83,200                           ; rcpps         %xmm0,%xmm1
  DB  15,82,192                           ; rsqrtps       %xmm0,%xmm0
  DB  65,15,89,205                        ; mulps         %xmm13,%xmm1
  DB  65,15,88,206                        ; addps         %xmm14,%xmm1
  DB  65,15,89,196                        ; mulps         %xmm12,%xmm0
  DB  15,88,193                           ; addps         %xmm1,%xmm0
  DB  68,15,40,210                        ; movaps        %xmm2,%xmm10
  DB  68,15,93,208                        ; minps         %xmm0,%xmm10
  DB  65,15,40,203                        ; movaps        %xmm11,%xmm1
  DB  15,89,203                           ; mulps         %xmm3,%xmm1
  DB  65,15,194,223,1                     ; cmpltps       %xmm15,%xmm3
  DB  15,40,195                           ; movaps        %xmm3,%xmm0
  DB  102,68,15,56,20,209                 ; blendvps      %xmm0,%xmm1,%xmm10
  DB  65,15,82,192                        ; rsqrtps       %xmm8,%xmm0
  DB  15,83,200                           ; rcpps         %xmm0,%xmm1
  DB  65,15,89,205                        ; mulps         %xmm13,%xmm1
  DB  65,15,88,206                        ; addps         %xmm14,%xmm1
  DB  15,82,192                           ; rsqrtps       %xmm0,%xmm0
  DB  65,15,89,196                        ; mulps         %xmm12,%xmm0
  DB  15,88,193                           ; addps         %xmm1,%xmm0
  DB  15,93,208                           ; minps         %xmm0,%xmm2
  DB  69,15,89,216                        ; mulps         %xmm8,%xmm11
  DB  69,15,194,199,1                     ; cmpltps       %xmm15,%xmm8
  DB  65,15,40,192                        ; movaps        %xmm8,%xmm0
  DB  102,65,15,56,20,211                 ; blendvps      %xmm0,%xmm11,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  65,15,40,193                        ; movaps        %xmm9,%xmm0
  DB  65,15,40,202                        ; movaps        %xmm10,%xmm1
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
  DB  243,68,15,16,74,12                  ; movss         0xc(%rdx),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  65,15,89,193                        ; mulps         %xmm9,%xmm0
  DB  65,15,89,201                        ; mulps         %xmm9,%xmm1
  DB  65,15,89,209                        ; mulps         %xmm9,%xmm2
  DB  65,15,89,217                        ; mulps         %xmm9,%xmm3
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
  DB  243,68,15,16,74,12                  ; movss         0xc(%rdx),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,193                        ; mulps         %xmm9,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  65,15,89,201                        ; mulps         %xmm9,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,209                        ; mulps         %xmm9,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,92,223                           ; subps         %xmm7,%xmm3
  DB  65,15,89,217                        ; mulps         %xmm9,%xmm3
  DB  15,88,223                           ; addps         %xmm7,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_lerp_565_sse41
_sk_lerp_565_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,68,15,56,51,4,120               ; pmovzxwd      (%rax,%rdi,2),%xmm8
  DB  102,15,110,90,104                   ; movd          0x68(%rdx),%xmm3
  DB  102,15,112,219,0                    ; pshufd        $0x0,%xmm3,%xmm3
  DB  102,65,15,219,216                   ; pand          %xmm8,%xmm3
  DB  68,15,91,203                        ; cvtdq2ps      %xmm3,%xmm9
  DB  243,15,16,26                        ; movss         (%rdx),%xmm3
  DB  243,68,15,16,82,116                 ; movss         0x74(%rdx),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  102,68,15,110,74,108                ; movd          0x6c(%rdx),%xmm9
  DB  102,69,15,112,201,0                 ; pshufd        $0x0,%xmm9,%xmm9
  DB  102,69,15,219,200                   ; pand          %xmm8,%xmm9
  DB  69,15,91,201                        ; cvtdq2ps      %xmm9,%xmm9
  DB  243,68,15,16,90,120                 ; movss         0x78(%rdx),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,89,217                        ; mulps         %xmm9,%xmm11
  DB  102,68,15,110,74,112                ; movd          0x70(%rdx),%xmm9
  DB  102,69,15,112,201,0                 ; pshufd        $0x0,%xmm9,%xmm9
  DB  102,69,15,219,200                   ; pand          %xmm8,%xmm9
  DB  69,15,91,193                        ; cvtdq2ps      %xmm9,%xmm8
  DB  243,68,15,16,74,124                 ; movss         0x7c(%rdx),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,194                        ; mulps         %xmm10,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  65,15,89,203                        ; mulps         %xmm11,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,209                        ; mulps         %xmm9,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_tables_sse41
_sk_load_tables_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,8                            ; mov           (%rax),%rcx
  DB  76,139,64,8                         ; mov           0x8(%rax),%r8
  DB  243,68,15,111,4,185                 ; movdqu        (%rcx,%rdi,4),%xmm8
  DB  102,15,110,66,16                    ; movd          0x10(%rdx),%xmm0
  DB  102,15,112,192,0                    ; pshufd        $0x0,%xmm0,%xmm0
  DB  102,65,15,111,200                   ; movdqa        %xmm8,%xmm1
  DB  102,15,114,209,8                    ; psrld         $0x8,%xmm1
  DB  102,15,219,200                      ; pand          %xmm0,%xmm1
  DB  102,65,15,111,208                   ; movdqa        %xmm8,%xmm2
  DB  102,15,114,210,16                   ; psrld         $0x10,%xmm2
  DB  102,15,219,208                      ; pand          %xmm0,%xmm2
  DB  102,65,15,219,192                   ; pand          %xmm8,%xmm0
  DB  102,72,15,58,22,193,1               ; pextrq        $0x1,%xmm0,%rcx
  DB  65,137,201                          ; mov           %ecx,%r9d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  102,73,15,126,194                   ; movq          %xmm0,%r10
  DB  69,137,211                          ; mov           %r10d,%r11d
  DB  73,193,234,32                       ; shr           $0x20,%r10
  DB  243,67,15,16,4,152                  ; movss         (%r8,%r11,4),%xmm0
  DB  102,67,15,58,33,4,144,16            ; insertps      $0x10,(%r8,%r10,4),%xmm0
  DB  102,67,15,58,33,4,136,32            ; insertps      $0x20,(%r8,%r9,4),%xmm0
  DB  102,65,15,58,33,4,136,48            ; insertps      $0x30,(%r8,%rcx,4),%xmm0
  DB  72,139,72,16                        ; mov           0x10(%rax),%rcx
  DB  102,73,15,58,22,200,1               ; pextrq        $0x1,%xmm1,%r8
  DB  69,137,193                          ; mov           %r8d,%r9d
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  102,73,15,126,202                   ; movq          %xmm1,%r10
  DB  69,137,211                          ; mov           %r10d,%r11d
  DB  73,193,234,32                       ; shr           $0x20,%r10
  DB  243,66,15,16,12,153                 ; movss         (%rcx,%r11,4),%xmm1
  DB  102,66,15,58,33,12,145,16           ; insertps      $0x10,(%rcx,%r10,4),%xmm1
  DB  243,66,15,16,28,137                 ; movss         (%rcx,%r9,4),%xmm3
  DB  102,15,58,33,203,32                 ; insertps      $0x20,%xmm3,%xmm1
  DB  243,66,15,16,28,129                 ; movss         (%rcx,%r8,4),%xmm3
  DB  102,15,58,33,203,48                 ; insertps      $0x30,%xmm3,%xmm1
  DB  72,139,64,24                        ; mov           0x18(%rax),%rax
  DB  102,72,15,58,22,209,1               ; pextrq        $0x1,%xmm2,%rcx
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  102,73,15,126,209                   ; movq          %xmm2,%r9
  DB  69,137,202                          ; mov           %r9d,%r10d
  DB  73,193,233,32                       ; shr           $0x20,%r9
  DB  243,66,15,16,20,144                 ; movss         (%rax,%r10,4),%xmm2
  DB  102,66,15,58,33,20,136,16           ; insertps      $0x10,(%rax,%r9,4),%xmm2
  DB  243,66,15,16,28,128                 ; movss         (%rax,%r8,4),%xmm3
  DB  102,15,58,33,211,32                 ; insertps      $0x20,%xmm3,%xmm2
  DB  243,15,16,28,136                    ; movss         (%rax,%rcx,4),%xmm3
  DB  102,15,58,33,211,48                 ; insertps      $0x30,%xmm3,%xmm2
  DB  102,65,15,114,208,24                ; psrld         $0x18,%xmm8
  DB  69,15,91,192                        ; cvtdq2ps      %xmm8,%xmm8
  DB  243,15,16,90,12                     ; movss         0xc(%rdx),%xmm3
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_a8_sse41
_sk_load_a8_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  102,15,56,49,4,56                   ; pmovzxbd      (%rax,%rdi,1),%xmm0
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  243,15,16,90,12                     ; movss         0xc(%rdx),%xmm3
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  15,89,216                           ; mulps         %xmm0,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,87,192                           ; xorps         %xmm0,%xmm0
  DB  15,87,201                           ; xorps         %xmm1,%xmm1
  DB  15,87,210                           ; xorps         %xmm2,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_a8_sse41
_sk_store_a8_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,68,15,16,66,8                   ; movss         0x8(%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
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
  DB  102,68,15,56,51,12,120              ; pmovzxwd      (%rax,%rdi,2),%xmm9
  DB  102,15,110,66,104                   ; movd          0x68(%rdx),%xmm0
  DB  102,15,112,192,0                    ; pshufd        $0x0,%xmm0,%xmm0
  DB  102,65,15,219,193                   ; pand          %xmm9,%xmm0
  DB  15,91,200                           ; cvtdq2ps      %xmm0,%xmm1
  DB  243,15,16,26                        ; movss         (%rdx),%xmm3
  DB  243,15,16,66,116                    ; movss         0x74(%rdx),%xmm0
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  15,89,193                           ; mulps         %xmm1,%xmm0
  DB  102,15,110,74,108                   ; movd          0x6c(%rdx),%xmm1
  DB  102,15,112,201,0                    ; pshufd        $0x0,%xmm1,%xmm1
  DB  102,65,15,219,201                   ; pand          %xmm9,%xmm1
  DB  68,15,91,193                        ; cvtdq2ps      %xmm1,%xmm8
  DB  243,15,16,74,120                    ; movss         0x78(%rdx),%xmm1
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  102,15,110,82,112                   ; movd          0x70(%rdx),%xmm2
  DB  102,15,112,210,0                    ; pshufd        $0x0,%xmm2,%xmm2
  DB  102,65,15,219,209                   ; pand          %xmm9,%xmm2
  DB  68,15,91,194                        ; cvtdq2ps      %xmm2,%xmm8
  DB  243,15,16,82,124                    ; movss         0x7c(%rdx),%xmm2
  DB  15,198,210,0                        ; shufps        $0x0,%xmm2,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_565_sse41
_sk_store_565_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,68,15,16,130,128,0,0,0          ; movss         0x80(%rdx),%xmm8
  DB  243,68,15,16,138,132,0,0,0          ; movss         0x84(%rdx),%xmm9
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  69,15,40,208                        ; movaps        %xmm8,%xmm10
  DB  68,15,89,208                        ; mulps         %xmm0,%xmm10
  DB  102,69,15,91,210                    ; cvtps2dq      %xmm10,%xmm10
  DB  102,65,15,114,242,11                ; pslld         $0xb,%xmm10
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  68,15,89,201                        ; mulps         %xmm1,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  102,65,15,114,241,5                 ; pslld         $0x5,%xmm9
  DB  102,69,15,235,202                   ; por           %xmm10,%xmm9
  DB  68,15,89,194                        ; mulps         %xmm2,%xmm8
  DB  102,69,15,91,192                    ; cvtps2dq      %xmm8,%xmm8
  DB  102,69,15,86,193                    ; orpd          %xmm9,%xmm8
  DB  102,69,15,56,43,192                 ; packusdw      %xmm8,%xmm8
  DB  102,68,15,214,4,120                 ; movq          %xmm8,(%rax,%rdi,2)
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_8888_sse41
_sk_load_8888_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,15,111,28,184                   ; movdqu        (%rax,%rdi,4),%xmm3
  DB  102,15,110,66,16                    ; movd          0x10(%rdx),%xmm0
  DB  102,15,112,192,0                    ; pshufd        $0x0,%xmm0,%xmm0
  DB  102,15,111,203                      ; movdqa        %xmm3,%xmm1
  DB  102,15,114,209,8                    ; psrld         $0x8,%xmm1
  DB  102,15,219,200                      ; pand          %xmm0,%xmm1
  DB  102,15,111,211                      ; movdqa        %xmm3,%xmm2
  DB  102,15,114,210,16                   ; psrld         $0x10,%xmm2
  DB  102,15,219,208                      ; pand          %xmm0,%xmm2
  DB  102,15,219,195                      ; pand          %xmm3,%xmm0
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  243,68,15,16,66,12                  ; movss         0xc(%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  15,91,210                           ; cvtdq2ps      %xmm2,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  102,15,114,211,24                   ; psrld         $0x18,%xmm3
  DB  15,91,219                           ; cvtdq2ps      %xmm3,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_8888_sse41
_sk_store_8888_sse41 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,68,15,16,66,8                   ; movss         0x8(%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,200                        ; mulps         %xmm0,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  69,15,40,208                        ; movaps        %xmm8,%xmm10
  DB  68,15,89,209                        ; mulps         %xmm1,%xmm10
  DB  102,69,15,91,210                    ; cvtps2dq      %xmm10,%xmm10
  DB  102,65,15,114,242,8                 ; pslld         $0x8,%xmm10
  DB  102,69,15,235,209                   ; por           %xmm9,%xmm10
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,202                        ; mulps         %xmm2,%xmm9
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
  DB  102,15,110,66,100                   ; movd          0x64(%rdx),%xmm0
  DB  102,15,112,216,0                    ; pshufd        $0x0,%xmm0,%xmm3
  DB  102,15,111,203                      ; movdqa        %xmm3,%xmm1
  DB  102,65,15,101,200                   ; pcmpgtw       %xmm8,%xmm1
  DB  102,65,15,223,200                   ; pandn         %xmm8,%xmm1
  DB  102,15,101,218                      ; pcmpgtw       %xmm2,%xmm3
  DB  102,15,223,218                      ; pandn         %xmm2,%xmm3
  DB  102,15,56,51,193                    ; pmovzxwd      %xmm1,%xmm0
  DB  102,15,114,240,13                   ; pslld         $0xd,%xmm0
  DB  102,15,110,82,92                    ; movd          0x5c(%rdx),%xmm2
  DB  102,68,15,112,194,0                 ; pshufd        $0x0,%xmm2,%xmm8
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
  DB  102,68,15,110,66,96                 ; movd          0x60(%rdx),%xmm8
  DB  102,69,15,112,192,0                 ; pshufd        $0x0,%xmm8,%xmm8
  DB  102,69,15,111,200                   ; movdqa        %xmm8,%xmm9
  DB  68,15,89,200                        ; mulps         %xmm0,%xmm9
  DB  102,65,15,114,209,13                ; psrld         $0xd,%xmm9
  DB  102,69,15,111,208                   ; movdqa        %xmm8,%xmm10
  DB  68,15,89,209                        ; mulps         %xmm1,%xmm10
  DB  102,65,15,114,210,13                ; psrld         $0xd,%xmm10
  DB  102,69,15,111,216                   ; movdqa        %xmm8,%xmm11
  DB  68,15,89,218                        ; mulps         %xmm2,%xmm11
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
  DB  15,91,200                           ; cvtdq2ps      %xmm0,%xmm1
  DB  243,15,16,18                        ; movss         (%rdx),%xmm2
  DB  243,15,16,90,4                      ; movss         0x4(%rdx),%xmm3
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  15,88,203                           ; addps         %xmm3,%xmm1
  DB  15,16,66,20                         ; movups        0x14(%rdx),%xmm0
  DB  15,88,193                           ; addps         %xmm1,%xmm0
  DB  102,15,110,8                        ; movd          (%rax),%xmm1
  DB  102,15,112,201,0                    ; pshufd        $0x0,%xmm1,%xmm1
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  15,88,203                           ; addps         %xmm3,%xmm1
  DB  15,198,210,0                        ; shufps        $0x0,%xmm2,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
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
  DB  243,68,15,16,2                      ; movss         (%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
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
  DB  243,68,15,16,2                      ; movss         (%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
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
  DB  243,68,15,16,2                      ; movss         (%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,93,192                        ; minps         %xmm8,%xmm0
  DB  65,15,93,200                        ; minps         %xmm8,%xmm1
  DB  65,15,93,208                        ; minps         %xmm8,%xmm2
  DB  65,15,93,216                        ; minps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_clamp_a_sse2
_sk_clamp_a_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,2                      ; movss         (%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,93,216                        ; minps         %xmm8,%xmm3
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
  DB  243,68,15,16,10                     ; movss         (%rdx),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
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
  DB  243,68,15,16,66,64                  ; movss         0x40(%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  69,15,40,232                        ; movaps        %xmm8,%xmm13
  DB  68,15,89,232                        ; mulps         %xmm0,%xmm13
  DB  68,15,40,224                        ; movaps        %xmm0,%xmm12
  DB  69,15,89,228                        ; mulps         %xmm12,%xmm12
  DB  243,68,15,16,74,60                  ; movss         0x3c(%rdx),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  243,68,15,16,82,52                  ; movss         0x34(%rdx),%xmm10
  DB  243,68,15,16,90,56                  ; movss         0x38(%rdx),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,40,241                        ; movaps        %xmm9,%xmm14
  DB  68,15,89,240                        ; mulps         %xmm0,%xmm14
  DB  69,15,88,243                        ; addps         %xmm11,%xmm14
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  69,15,89,244                        ; mulps         %xmm12,%xmm14
  DB  69,15,88,242                        ; addps         %xmm10,%xmm14
  DB  243,68,15,16,98,68                  ; movss         0x44(%rdx),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  65,15,194,196,1                     ; cmpltps       %xmm12,%xmm0
  DB  68,15,84,232                        ; andps         %xmm0,%xmm13
  DB  65,15,85,198                        ; andnps        %xmm14,%xmm0
  DB  65,15,86,197                        ; orps          %xmm13,%xmm0
  DB  69,15,40,232                        ; movaps        %xmm8,%xmm13
  DB  68,15,89,233                        ; mulps         %xmm1,%xmm13
  DB  68,15,40,241                        ; movaps        %xmm1,%xmm14
  DB  69,15,89,246                        ; mulps         %xmm14,%xmm14
  DB  69,15,40,249                        ; movaps        %xmm9,%xmm15
  DB  68,15,89,249                        ; mulps         %xmm1,%xmm15
  DB  69,15,88,251                        ; addps         %xmm11,%xmm15
  DB  69,15,89,254                        ; mulps         %xmm14,%xmm15
  DB  69,15,88,250                        ; addps         %xmm10,%xmm15
  DB  65,15,194,204,1                     ; cmpltps       %xmm12,%xmm1
  DB  68,15,84,233                        ; andps         %xmm1,%xmm13
  DB  65,15,85,207                        ; andnps        %xmm15,%xmm1
  DB  65,15,86,205                        ; orps          %xmm13,%xmm1
  DB  68,15,89,194                        ; mulps         %xmm2,%xmm8
  DB  68,15,40,234                        ; movaps        %xmm2,%xmm13
  DB  69,15,89,237                        ; mulps         %xmm13,%xmm13
  DB  68,15,89,202                        ; mulps         %xmm2,%xmm9
  DB  69,15,88,203                        ; addps         %xmm11,%xmm9
  DB  69,15,89,205                        ; mulps         %xmm13,%xmm9
  DB  69,15,88,202                        ; addps         %xmm10,%xmm9
  DB  65,15,194,212,1                     ; cmpltps       %xmm12,%xmm2
  DB  68,15,84,194                        ; andps         %xmm2,%xmm8
  DB  65,15,85,209                        ; andnps        %xmm9,%xmm2
  DB  65,15,86,208                        ; orps          %xmm8,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_to_srgb_sse2
_sk_to_srgb_sse2 LABEL PROC
  DB  72,131,236,40                       ; sub           $0x28,%rsp
  DB  15,41,124,36,16                     ; movaps        %xmm7,0x10(%rsp)
  DB  15,41,52,36                         ; movaps        %xmm6,(%rsp)
  DB  15,40,245                           ; movaps        %xmm5,%xmm6
  DB  15,40,236                           ; movaps        %xmm4,%xmm5
  DB  15,40,227                           ; movaps        %xmm3,%xmm4
  DB  68,15,82,192                        ; rsqrtps       %xmm0,%xmm8
  DB  69,15,83,232                        ; rcpps         %xmm8,%xmm13
  DB  69,15,82,248                        ; rsqrtps       %xmm8,%xmm15
  DB  243,15,16,26                        ; movss         (%rdx),%xmm3
  DB  243,68,15,16,66,72                  ; movss         0x48(%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  69,15,40,240                        ; movaps        %xmm8,%xmm14
  DB  68,15,89,240                        ; mulps         %xmm0,%xmm14
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  243,68,15,16,82,76                  ; movss         0x4c(%rdx),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  243,68,15,16,90,80                  ; movss         0x50(%rdx),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  243,68,15,16,98,84                  ; movss         0x54(%rdx),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  69,15,89,235                        ; mulps         %xmm11,%xmm13
  DB  69,15,88,236                        ; addps         %xmm12,%xmm13
  DB  69,15,89,250                        ; mulps         %xmm10,%xmm15
  DB  69,15,88,253                        ; addps         %xmm13,%xmm15
  DB  68,15,40,203                        ; movaps        %xmm3,%xmm9
  DB  69,15,93,207                        ; minps         %xmm15,%xmm9
  DB  243,68,15,16,106,88                 ; movss         0x58(%rdx),%xmm13
  DB  69,15,198,237,0                     ; shufps        $0x0,%xmm13,%xmm13
  DB  65,15,194,197,1                     ; cmpltps       %xmm13,%xmm0
  DB  68,15,84,240                        ; andps         %xmm0,%xmm14
  DB  65,15,85,193                        ; andnps        %xmm9,%xmm0
  DB  65,15,86,198                        ; orps          %xmm14,%xmm0
  DB  68,15,82,201                        ; rsqrtps       %xmm1,%xmm9
  DB  69,15,83,241                        ; rcpps         %xmm9,%xmm14
  DB  69,15,82,201                        ; rsqrtps       %xmm9,%xmm9
  DB  69,15,89,243                        ; mulps         %xmm11,%xmm14
  DB  69,15,88,244                        ; addps         %xmm12,%xmm14
  DB  69,15,89,202                        ; mulps         %xmm10,%xmm9
  DB  69,15,88,206                        ; addps         %xmm14,%xmm9
  DB  68,15,40,243                        ; movaps        %xmm3,%xmm14
  DB  69,15,93,241                        ; minps         %xmm9,%xmm14
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,201                        ; mulps         %xmm1,%xmm9
  DB  65,15,194,205,1                     ; cmpltps       %xmm13,%xmm1
  DB  68,15,84,201                        ; andps         %xmm1,%xmm9
  DB  65,15,85,206                        ; andnps        %xmm14,%xmm1
  DB  65,15,86,201                        ; orps          %xmm9,%xmm1
  DB  68,15,82,202                        ; rsqrtps       %xmm2,%xmm9
  DB  69,15,83,241                        ; rcpps         %xmm9,%xmm14
  DB  69,15,89,243                        ; mulps         %xmm11,%xmm14
  DB  69,15,88,244                        ; addps         %xmm12,%xmm14
  DB  65,15,82,249                        ; rsqrtps       %xmm9,%xmm7
  DB  65,15,89,250                        ; mulps         %xmm10,%xmm7
  DB  65,15,88,254                        ; addps         %xmm14,%xmm7
  DB  15,93,223                           ; minps         %xmm7,%xmm3
  DB  68,15,89,194                        ; mulps         %xmm2,%xmm8
  DB  65,15,194,213,1                     ; cmpltps       %xmm13,%xmm2
  DB  68,15,84,194                        ; andps         %xmm2,%xmm8
  DB  15,85,211                           ; andnps        %xmm3,%xmm2
  DB  65,15,86,208                        ; orps          %xmm8,%xmm2
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,40,220                           ; movaps        %xmm4,%xmm3
  DB  15,40,229                           ; movaps        %xmm5,%xmm4
  DB  15,40,238                           ; movaps        %xmm6,%xmm5
  DB  15,40,52,36                         ; movaps        (%rsp),%xmm6
  DB  15,40,124,36,16                     ; movaps        0x10(%rsp),%xmm7
  DB  72,131,196,40                       ; add           $0x28,%rsp
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
  DB  243,68,15,16,74,12                  ; movss         0xc(%rdx),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  65,15,89,193                        ; mulps         %xmm9,%xmm0
  DB  65,15,89,201                        ; mulps         %xmm9,%xmm1
  DB  65,15,89,209                        ; mulps         %xmm9,%xmm2
  DB  65,15,89,217                        ; mulps         %xmm9,%xmm3
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
  DB  243,68,15,16,74,12                  ; movss         0xc(%rdx),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,193                        ; mulps         %xmm9,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  65,15,89,201                        ; mulps         %xmm9,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,209                        ; mulps         %xmm9,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,92,223                           ; subps         %xmm7,%xmm3
  DB  65,15,89,217                        ; mulps         %xmm9,%xmm3
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
  DB  102,15,110,90,104                   ; movd          0x68(%rdx),%xmm3
  DB  102,15,112,219,0                    ; pshufd        $0x0,%xmm3,%xmm3
  DB  102,65,15,219,216                   ; pand          %xmm8,%xmm3
  DB  68,15,91,203                        ; cvtdq2ps      %xmm3,%xmm9
  DB  243,15,16,26                        ; movss         (%rdx),%xmm3
  DB  243,68,15,16,82,116                 ; movss         0x74(%rdx),%xmm10
  DB  69,15,198,210,0                     ; shufps        $0x0,%xmm10,%xmm10
  DB  69,15,89,209                        ; mulps         %xmm9,%xmm10
  DB  102,68,15,110,74,108                ; movd          0x6c(%rdx),%xmm9
  DB  102,69,15,112,201,0                 ; pshufd        $0x0,%xmm9,%xmm9
  DB  102,69,15,219,200                   ; pand          %xmm8,%xmm9
  DB  69,15,91,201                        ; cvtdq2ps      %xmm9,%xmm9
  DB  243,68,15,16,90,120                 ; movss         0x78(%rdx),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,89,217                        ; mulps         %xmm9,%xmm11
  DB  102,68,15,110,74,112                ; movd          0x70(%rdx),%xmm9
  DB  102,69,15,112,201,0                 ; pshufd        $0x0,%xmm9,%xmm9
  DB  102,69,15,219,200                   ; pand          %xmm8,%xmm9
  DB  69,15,91,193                        ; cvtdq2ps      %xmm9,%xmm8
  DB  243,68,15,16,74,124                 ; movss         0x7c(%rdx),%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  69,15,89,200                        ; mulps         %xmm8,%xmm9
  DB  15,92,196                           ; subps         %xmm4,%xmm0
  DB  65,15,89,194                        ; mulps         %xmm10,%xmm0
  DB  15,88,196                           ; addps         %xmm4,%xmm0
  DB  15,92,205                           ; subps         %xmm5,%xmm1
  DB  65,15,89,203                        ; mulps         %xmm11,%xmm1
  DB  15,88,205                           ; addps         %xmm5,%xmm1
  DB  15,92,214                           ; subps         %xmm6,%xmm2
  DB  65,15,89,209                        ; mulps         %xmm9,%xmm2
  DB  15,88,214                           ; addps         %xmm6,%xmm2
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_load_tables_sse2
_sk_load_tables_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,8                            ; mov           (%rax),%rcx
  DB  76,139,64,8                         ; mov           0x8(%rax),%r8
  DB  243,68,15,111,4,185                 ; movdqu        (%rcx,%rdi,4),%xmm8
  DB  102,15,110,66,16                    ; movd          0x10(%rdx),%xmm0
  DB  102,15,112,192,0                    ; pshufd        $0x0,%xmm0,%xmm0
  DB  102,69,15,111,200                   ; movdqa        %xmm8,%xmm9
  DB  102,65,15,114,209,8                 ; psrld         $0x8,%xmm9
  DB  102,68,15,219,200                   ; pand          %xmm0,%xmm9
  DB  102,69,15,111,208                   ; movdqa        %xmm8,%xmm10
  DB  102,65,15,114,210,16                ; psrld         $0x10,%xmm10
  DB  102,68,15,219,208                   ; pand          %xmm0,%xmm10
  DB  102,65,15,219,192                   ; pand          %xmm8,%xmm0
  DB  102,15,112,216,78                   ; pshufd        $0x4e,%xmm0,%xmm3
  DB  102,72,15,126,217                   ; movq          %xmm3,%rcx
  DB  65,137,201                          ; mov           %ecx,%r9d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  102,73,15,126,194                   ; movq          %xmm0,%r10
  DB  69,137,211                          ; mov           %r10d,%r11d
  DB  73,193,234,32                       ; shr           $0x20,%r10
  DB  243,67,15,16,28,144                 ; movss         (%r8,%r10,4),%xmm3
  DB  243,65,15,16,4,136                  ; movss         (%r8,%rcx,4),%xmm0
  DB  15,20,216                           ; unpcklps      %xmm0,%xmm3
  DB  243,67,15,16,4,152                  ; movss         (%r8,%r11,4),%xmm0
  DB  243,67,15,16,12,136                 ; movss         (%r8,%r9,4),%xmm1
  DB  15,20,193                           ; unpcklps      %xmm1,%xmm0
  DB  15,20,195                           ; unpcklps      %xmm3,%xmm0
  DB  72,139,72,16                        ; mov           0x10(%rax),%rcx
  DB  102,65,15,112,201,78                ; pshufd        $0x4e,%xmm9,%xmm1
  DB  102,73,15,126,200                   ; movq          %xmm1,%r8
  DB  69,137,193                          ; mov           %r8d,%r9d
  DB  73,193,232,32                       ; shr           $0x20,%r8
  DB  102,77,15,126,202                   ; movq          %xmm9,%r10
  DB  69,137,211                          ; mov           %r10d,%r11d
  DB  73,193,234,32                       ; shr           $0x20,%r10
  DB  243,66,15,16,28,145                 ; movss         (%rcx,%r10,4),%xmm3
  DB  243,66,15,16,12,129                 ; movss         (%rcx,%r8,4),%xmm1
  DB  15,20,217                           ; unpcklps      %xmm1,%xmm3
  DB  243,66,15,16,12,153                 ; movss         (%rcx,%r11,4),%xmm1
  DB  243,66,15,16,20,137                 ; movss         (%rcx,%r9,4),%xmm2
  DB  15,20,202                           ; unpcklps      %xmm2,%xmm1
  DB  15,20,203                           ; unpcklps      %xmm3,%xmm1
  DB  72,139,64,24                        ; mov           0x18(%rax),%rax
  DB  102,65,15,112,210,78                ; pshufd        $0x4e,%xmm10,%xmm2
  DB  102,72,15,126,209                   ; movq          %xmm2,%rcx
  DB  65,137,200                          ; mov           %ecx,%r8d
  DB  72,193,233,32                       ; shr           $0x20,%rcx
  DB  102,77,15,126,209                   ; movq          %xmm10,%r9
  DB  69,137,202                          ; mov           %r9d,%r10d
  DB  73,193,233,32                       ; shr           $0x20,%r9
  DB  243,70,15,16,12,136                 ; movss         (%rax,%r9,4),%xmm9
  DB  243,15,16,20,136                    ; movss         (%rax,%rcx,4),%xmm2
  DB  68,15,20,202                        ; unpcklps      %xmm2,%xmm9
  DB  243,66,15,16,20,144                 ; movss         (%rax,%r10,4),%xmm2
  DB  243,66,15,16,28,128                 ; movss         (%rax,%r8,4),%xmm3
  DB  15,20,211                           ; unpcklps      %xmm3,%xmm2
  DB  65,15,20,209                        ; unpcklps      %xmm9,%xmm2
  DB  102,65,15,114,208,24                ; psrld         $0x18,%xmm8
  DB  69,15,91,192                        ; cvtdq2ps      %xmm8,%xmm8
  DB  243,15,16,90,12                     ; movss         0xc(%rdx),%xmm3
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
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
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  243,15,16,90,12                     ; movss         0xc(%rdx),%xmm3
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  15,89,216                           ; mulps         %xmm0,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  15,87,192                           ; xorps         %xmm0,%xmm0
  DB  102,15,239,201                      ; pxor          %xmm1,%xmm1
  DB  15,87,210                           ; xorps         %xmm2,%xmm2
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_a8_sse2
_sk_store_a8_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,68,15,16,66,8                   ; movss         0x8(%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
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
  DB  243,68,15,126,12,120                ; movq          (%rax,%rdi,2),%xmm9
  DB  102,15,239,192                      ; pxor          %xmm0,%xmm0
  DB  102,68,15,97,200                    ; punpcklwd     %xmm0,%xmm9
  DB  102,15,110,66,104                   ; movd          0x68(%rdx),%xmm0
  DB  102,15,112,192,0                    ; pshufd        $0x0,%xmm0,%xmm0
  DB  102,65,15,219,193                   ; pand          %xmm9,%xmm0
  DB  15,91,200                           ; cvtdq2ps      %xmm0,%xmm1
  DB  243,15,16,26                        ; movss         (%rdx),%xmm3
  DB  243,15,16,66,116                    ; movss         0x74(%rdx),%xmm0
  DB  15,198,192,0                        ; shufps        $0x0,%xmm0,%xmm0
  DB  15,89,193                           ; mulps         %xmm1,%xmm0
  DB  102,15,110,74,108                   ; movd          0x6c(%rdx),%xmm1
  DB  102,15,112,201,0                    ; pshufd        $0x0,%xmm1,%xmm1
  DB  102,65,15,219,201                   ; pand          %xmm9,%xmm1
  DB  68,15,91,193                        ; cvtdq2ps      %xmm1,%xmm8
  DB  243,15,16,74,120                    ; movss         0x78(%rdx),%xmm1
  DB  15,198,201,0                        ; shufps        $0x0,%xmm1,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  102,15,110,82,112                   ; movd          0x70(%rdx),%xmm2
  DB  102,15,112,210,0                    ; pshufd        $0x0,%xmm2,%xmm2
  DB  102,65,15,219,209                   ; pand          %xmm9,%xmm2
  DB  68,15,91,194                        ; cvtdq2ps      %xmm2,%xmm8
  DB  243,15,16,82,124                    ; movss         0x7c(%rdx),%xmm2
  DB  15,198,210,0                        ; shufps        $0x0,%xmm2,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  15,198,219,0                        ; shufps        $0x0,%xmm3,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_565_sse2
_sk_store_565_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,68,15,16,130,128,0,0,0          ; movss         0x80(%rdx),%xmm8
  DB  243,68,15,16,138,132,0,0,0          ; movss         0x84(%rdx),%xmm9
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  69,15,40,208                        ; movaps        %xmm8,%xmm10
  DB  68,15,89,208                        ; mulps         %xmm0,%xmm10
  DB  102,69,15,91,210                    ; cvtps2dq      %xmm10,%xmm10
  DB  102,65,15,114,242,11                ; pslld         $0xb,%xmm10
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  68,15,89,201                        ; mulps         %xmm1,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  102,65,15,114,241,5                 ; pslld         $0x5,%xmm9
  DB  102,69,15,235,202                   ; por           %xmm10,%xmm9
  DB  68,15,89,194                        ; mulps         %xmm2,%xmm8
  DB  102,69,15,91,192                    ; cvtps2dq      %xmm8,%xmm8
  DB  102,69,15,86,193                    ; orpd          %xmm9,%xmm8
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
  DB  243,15,111,28,184                   ; movdqu        (%rax,%rdi,4),%xmm3
  DB  102,15,110,66,16                    ; movd          0x10(%rdx),%xmm0
  DB  102,15,112,192,0                    ; pshufd        $0x0,%xmm0,%xmm0
  DB  102,15,111,203                      ; movdqa        %xmm3,%xmm1
  DB  102,15,114,209,8                    ; psrld         $0x8,%xmm1
  DB  102,15,219,200                      ; pand          %xmm0,%xmm1
  DB  102,15,111,211                      ; movdqa        %xmm3,%xmm2
  DB  102,15,114,210,16                   ; psrld         $0x10,%xmm2
  DB  102,15,219,208                      ; pand          %xmm0,%xmm2
  DB  102,15,219,195                      ; pand          %xmm3,%xmm0
  DB  15,91,192                           ; cvtdq2ps      %xmm0,%xmm0
  DB  243,68,15,16,66,12                  ; movss         0xc(%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,89,192                        ; mulps         %xmm8,%xmm0
  DB  15,91,201                           ; cvtdq2ps      %xmm1,%xmm1
  DB  65,15,89,200                        ; mulps         %xmm8,%xmm1
  DB  15,91,210                           ; cvtdq2ps      %xmm2,%xmm2
  DB  65,15,89,208                        ; mulps         %xmm8,%xmm2
  DB  102,15,114,211,24                   ; psrld         $0x18,%xmm3
  DB  15,91,219                           ; cvtdq2ps      %xmm3,%xmm3
  DB  65,15,89,216                        ; mulps         %xmm8,%xmm3
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_store_8888_sse2
_sk_store_8888_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  72,139,0                            ; mov           (%rax),%rax
  DB  243,68,15,16,66,8                   ; movss         0x8(%rdx),%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,200                        ; mulps         %xmm0,%xmm9
  DB  102,69,15,91,201                    ; cvtps2dq      %xmm9,%xmm9
  DB  69,15,40,208                        ; movaps        %xmm8,%xmm10
  DB  68,15,89,209                        ; mulps         %xmm1,%xmm10
  DB  102,69,15,91,210                    ; cvtps2dq      %xmm10,%xmm10
  DB  102,65,15,114,242,8                 ; pslld         $0x8,%xmm10
  DB  102,69,15,235,209                   ; por           %xmm9,%xmm10
  DB  69,15,40,200                        ; movaps        %xmm8,%xmm9
  DB  68,15,89,202                        ; mulps         %xmm2,%xmm9
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
  DB  102,15,110,66,100                   ; movd          0x64(%rdx),%xmm0
  DB  102,15,112,216,0                    ; pshufd        $0x0,%xmm0,%xmm3
  DB  102,15,111,203                      ; movdqa        %xmm3,%xmm1
  DB  102,65,15,101,200                   ; pcmpgtw       %xmm8,%xmm1
  DB  102,65,15,223,200                   ; pandn         %xmm8,%xmm1
  DB  102,15,101,218                      ; pcmpgtw       %xmm2,%xmm3
  DB  102,15,223,218                      ; pandn         %xmm2,%xmm3
  DB  102,69,15,239,192                   ; pxor          %xmm8,%xmm8
  DB  102,15,111,193                      ; movdqa        %xmm1,%xmm0
  DB  102,65,15,97,192                    ; punpcklwd     %xmm8,%xmm0
  DB  102,15,114,240,13                   ; pslld         $0xd,%xmm0
  DB  102,15,110,82,92                    ; movd          0x5c(%rdx),%xmm2
  DB  102,68,15,112,202,0                 ; pshufd        $0x0,%xmm2,%xmm9
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
  DB  102,68,15,110,66,96                 ; movd          0x60(%rdx),%xmm8
  DB  102,69,15,112,192,0                 ; pshufd        $0x0,%xmm8,%xmm8
  DB  102,69,15,111,200                   ; movdqa        %xmm8,%xmm9
  DB  68,15,89,200                        ; mulps         %xmm0,%xmm9
  DB  102,65,15,114,209,13                ; psrld         $0xd,%xmm9
  DB  102,69,15,111,208                   ; movdqa        %xmm8,%xmm10
  DB  68,15,89,209                        ; mulps         %xmm1,%xmm10
  DB  102,65,15,114,210,13                ; psrld         $0xd,%xmm10
  DB  102,69,15,111,216                   ; movdqa        %xmm8,%xmm11
  DB  68,15,89,218                        ; mulps         %xmm2,%xmm11
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
  DB  243,68,15,16,26                     ; movss         (%rdx),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,84,217                        ; andps         %xmm9,%xmm11
  DB  69,15,92,211                        ; subps         %xmm11,%xmm10
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
  DB  243,68,15,16,26                     ; movss         (%rdx),%xmm11
  DB  69,15,198,219,0                     ; shufps        $0x0,%xmm11,%xmm11
  DB  69,15,84,217                        ; andps         %xmm9,%xmm11
  DB  69,15,92,211                        ; subps         %xmm11,%xmm10
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
  DB  243,68,15,16,8                      ; movss         (%rax),%xmm9
  DB  69,15,40,193                        ; movaps        %xmm9,%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,92,192                        ; subps         %xmm8,%xmm0
  DB  243,69,15,88,201                    ; addss         %xmm9,%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  68,15,40,208                        ; movaps        %xmm0,%xmm10
  DB  69,15,94,209                        ; divps         %xmm9,%xmm10
  DB  243,69,15,91,218                    ; cvttps2dq     %xmm10,%xmm11
  DB  69,15,91,219                        ; cvtdq2ps      %xmm11,%xmm11
  DB  69,15,194,211,1                     ; cmpltps       %xmm11,%xmm10
  DB  243,68,15,16,34                     ; movss         (%rdx),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  69,15,84,226                        ; andps         %xmm10,%xmm12
  DB  69,15,87,210                        ; xorps         %xmm10,%xmm10
  DB  69,15,92,220                        ; subps         %xmm12,%xmm11
  DB  69,15,89,217                        ; mulps         %xmm9,%xmm11
  DB  65,15,92,195                        ; subps         %xmm11,%xmm0
  DB  65,15,92,192                        ; subps         %xmm8,%xmm0
  DB  68,15,92,208                        ; subps         %xmm0,%xmm10
  DB  65,15,84,194                        ; andps         %xmm10,%xmm0
  DB  102,69,15,118,201                   ; pcmpeqd       %xmm9,%xmm9
  DB  102,69,15,254,200                   ; paddd         %xmm8,%xmm9
  DB  65,15,93,193                        ; minps         %xmm9,%xmm0
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  255,224                             ; jmpq          *%rax

PUBLIC _sk_mirror_y_sse2
_sk_mirror_y_sse2 LABEL PROC
  DB  72,173                              ; lods          %ds:(%rsi),%rax
  DB  243,68,15,16,8                      ; movss         (%rax),%xmm9
  DB  69,15,40,193                        ; movaps        %xmm9,%xmm8
  DB  69,15,198,192,0                     ; shufps        $0x0,%xmm8,%xmm8
  DB  65,15,92,200                        ; subps         %xmm8,%xmm1
  DB  243,69,15,88,201                    ; addss         %xmm9,%xmm9
  DB  69,15,198,201,0                     ; shufps        $0x0,%xmm9,%xmm9
  DB  68,15,40,209                        ; movaps        %xmm1,%xmm10
  DB  69,15,94,209                        ; divps         %xmm9,%xmm10
  DB  243,69,15,91,218                    ; cvttps2dq     %xmm10,%xmm11
  DB  69,15,91,219                        ; cvtdq2ps      %xmm11,%xmm11
  DB  69,15,194,211,1                     ; cmpltps       %xmm11,%xmm10
  DB  243,68,15,16,34                     ; movss         (%rdx),%xmm12
  DB  69,15,198,228,0                     ; shufps        $0x0,%xmm12,%xmm12
  DB  69,15,84,226                        ; andps         %xmm10,%xmm12
  DB  69,15,87,210                        ; xorps         %xmm10,%xmm10
  DB  69,15,92,220                        ; subps         %xmm12,%xmm11
  DB  69,15,89,217                        ; mulps         %xmm9,%xmm11
  DB  65,15,92,203                        ; subps         %xmm11,%xmm1
  DB  65,15,92,200                        ; subps         %xmm8,%xmm1
  DB  68,15,92,209                        ; subps         %xmm1,%xmm10
  DB  65,15,84,202                        ; andps         %xmm10,%xmm1
  DB  102,69,15,118,201                   ; pcmpeqd       %xmm9,%xmm9
  DB  102,69,15,254,200                   ; paddd         %xmm8,%xmm9
  DB  65,15,93,201                        ; minps         %xmm9,%xmm1
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
END
