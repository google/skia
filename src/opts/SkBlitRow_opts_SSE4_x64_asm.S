/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef CRBUG_399842_FIXED

#if defined(__clang__) || (defined(__GNUC__) && !defined(SK_BUILD_FOR_MAC))

#define EXTRACT_ALPHA(var1, var2) \
    movdqa      %var1, %var2;           /* Clone source pixels to extract alpha */\
    psrlw       $8, %var2;              /* Discard red and blue, leaving alpha and green */\
    pshufhw     $0xF5, %var2, %var2;    /* Repeat alpha for scaling (high) */\
    movdqa      %xmm6, %xmm4;           \
    pshuflw     $0xF5, %var2, %var2;    /* Repeat alpha for scaling (low) */\
    movdqa      %xmm5, %xmm3;           \
    psubw       %var2, %xmm4            /* Finalize alpha calculations */

#define SCALE_PIXELS \
    psllw       $8, %xmm5;              /* Filter out red and blue components */\
    pmulhuw     %xmm4, %xmm5;           /* Scale red and blue */\
    psrlw       $8, %xmm3;              /* Filter out alpha and green components */\
    pmullw      %xmm4, %xmm3            /* Scale alpha and green */


/*
 * void S32A_Opaque_BlitRow32_SSE4(SkPMColor* SK_RESTRICT dst,
 *                                 const SkPMColor* SK_RESTRICT src,
 *                                 int count, U8CPU alpha)
 *
 * This function is divided into six blocks: initialization, blit 4-15 pixels,
 * blit 0-3 pixels, align destination for 16+ pixel blits,
 * blit 16+ pixels with source unaligned, blit 16+ pixels with source aligned.
 * There are some code reuse between the blocks.
 *
 * The primary optimization comes from checking the source pixels' alpha value.
 * If the alpha is zero, the pixel can be skipped entirely.
 * If the alpha is fully opaque, the pixel can be copied directly to the destination.
 * According to collected statistics, these two cases are the most common.
 * The main loop(s) uses pre-loading and unrolling in an attempt to reduce the
 * memory latency worse-case.
 */

#ifdef __clang__
    .text
#else
    .section .text.sse4.2,"ax",@progbits
    .type S32A_Opaque_BlitRow32_SSE4_asm, @function
#endif
    .p2align 4
#if defined(SK_BUILD_FOR_MAC)
    .global _S32A_Opaque_BlitRow32_SSE4_asm
    .private_extern _S32A_Opaque_BlitRow32_SSE4_asm
_S32A_Opaque_BlitRow32_SSE4_asm:
#else
    .global S32A_Opaque_BlitRow32_SSE4_asm
    .hidden S32A_Opaque_BlitRow32_SSE4_asm
S32A_Opaque_BlitRow32_SSE4_asm:
#endif
    .cfi_startproc
    prefetcht0  (%rsi)
    movl        %edx, %ecx              // Pixel count
    movq        %rdi, %rdx              // Destination pointer
    movq        %rsi, %rax              // Source pointer

    // Setup SSE constants
    movdqa      .LAlphaCheckMask(%rip), %xmm7  // 0xFF000000 mask to check alpha
    movdqa      .LInverseAlphaCalc(%rip), %xmm6// 16-bit 256 to calculate inv. alpha
    movdqa      .LResultMergeMask(%rip), %xmm0 // 0x00FF00FF mask (Must be in xmm0 because of pblendvb)

    subl        $4, %ecx                // Check if we have only 0-3 pixels
    js          .LReallySmall
    cmpl        $11, %ecx               // Do we have enough pixels to run the main loop?
    ja          .LBigBlit

    // Handle small blits (4-15 pixels)
    ////////////////////////////////////////////////////////////////////////////////
    xorq        %rdi, %rdi              // Reset offset to zero

.LSmallLoop:
    lddqu       (%rax, %rdi), %xmm1     // Load four source pixels
    ptest       %xmm7, %xmm1            // Check if all alphas are zero or opaque
    ja          .LSmallAlphaNotOpaqueOrZero
    jz          .LSmallAlphaZero
    movdqu      %xmm1, (%rdx, %rdi)     // Store four destination pixels
.LSmallAlphaZero:
    addq        $16, %rdi
    subl        $4, %ecx                // Check if there are four additional pixels, at least
    jns         .LSmallLoop
    jmp         .LSmallRemaining

    // Handle mixed alphas (calculate and scale)
    .p2align 4
.LSmallAlphaNotOpaqueOrZero:
    lddqu       (%rdx, %rdi), %xmm5     // Load four destination pixels
    EXTRACT_ALPHA(xmm1, xmm2)           // Extract and clone alpha value
    SCALE_PIXELS                        // Scale pixels using alpha

    addq        $16, %rdi
    subl        $4, %ecx                // Check if there are four additional pixels, at least
    pblendvb    %xmm5, %xmm3            // Mask in %xmm0, implicitly
    paddb       %xmm3, %xmm1            // Add source and destination pixels together
    movdqu      %xmm1, -16(%rdx, %rdi)  // Store four destination pixels
    jns         .LSmallLoop

    // Handle the last 0-3 pixels (also used by the main loops)
.LSmallRemaining:
    cmpl        $-4, %ecx               // Check if we are done
    je          .LSmallExit
    sall        $2, %ecx                // Calculate offset for last pixels
    movslq      %ecx, %rcx
    addq        %rcx, %rdi

    lddqu       (%rax, %rdi), %xmm1     // Load last four source pixels (overlapping)
    ptest       %xmm7, %xmm1            // Check if all alphas are zero or opaque
    jc          .LSmallRemainingStoreAll// If all alphas are opaque, just store (overlapping)
    jz          .LSmallExit             // If all alphas are zero, skip the pixels completely

    // Handle mixed alphas (calculate and scale)
    lddqu       (%rdx, %rdi), %xmm5     // Load last four destination pixels (overlapping)
    EXTRACT_ALPHA(xmm1, xmm2)           // Extract and clone alpha value

    psllw       $8, %xmm3               // Filter out red and blue components
    pmulhuw     %xmm4, %xmm3            // Scale red and blue
    movdqa      %xmm5, %xmm2
    psrlw       $8, %xmm2               // Filter out alpha and green components
    pmullw      %xmm4, %xmm2            // Scale alpha and green

    cmpl        $-8, %ecx               // Check how many pixels should be written
    pblendvb    %xmm3, %xmm2            // Combine results (mask in %xmm0, implicitly)
    paddb       %xmm2, %xmm1            // Add source and destination pixels together
    jb          .LSmallPixelsLeft1
    ja          .LSmallPixelsLeft3      // To avoid double-blending the overlapping pixels...
    pblendw     $0xF0, %xmm1, %xmm5     // Merge only the final two pixels to the destination
    movdqu      %xmm5, (%rdx, %rdi)     // Store last two destination pixels
.LSmallExit:
    ret

.LSmallPixelsLeft1:
    pblendw     $0xC0, %xmm1, %xmm5     // Merge only the final pixel to the destination
    movdqu      %xmm5, (%rdx, %rdi)     // Store last destination pixel
    ret

.LSmallPixelsLeft3:
    pblendw     $0xFC, %xmm1, %xmm5     // Merge only the final three pixels to the destination
    movdqu      %xmm5, (%rdx, %rdi)     // Store last three destination pixels
    ret

.LSmallRemainingStoreAll:
    movdqu      %xmm1, (%rdx, %rdi)     // Store last destination pixels (overwrite)
    ret

    // Handle really small blits (0-3 pixels)
    ////////////////////////////////////////////////////////////////////////////////
.LReallySmall:
    addl        $4, %ecx
    jle         .LReallySmallExit
    pcmpeqd     %xmm1, %xmm1
    cmpl        $2, %ecx                // Check how many pixels should be read
    pinsrd      $0x0, (%rax), %xmm1     // Load one source pixel
    pinsrd      $0x0, (%rdx), %xmm5     // Load one destination pixel
    jb          .LReallySmallCalc
    pinsrd      $0x1, 4(%rax), %xmm1    // Load second source pixel
    pinsrd      $0x1, 4(%rdx), %xmm5    // Load second destination pixel
    je          .LReallySmallCalc
    pinsrd      $0x2, 8(%rax), %xmm1    // Load third source pixel
    pinsrd      $0x2, 8(%rdx), %xmm5    // Load third destination pixel

.LReallySmallCalc:
    ptest       %xmm7, %xmm1            // Check if all alphas are opaque
    jc          .LReallySmallStore      // If all alphas are opaque, just store

    // Handle mixed alphas (calculate and scale)
    EXTRACT_ALPHA(xmm1, xmm2)           // Extract and clone alpha value

    pand        %xmm0, %xmm5            // Filter out red and blue components
    pmullw      %xmm4, %xmm5            // Scale red and blue
    psrlw       $8, %xmm3               // Filter out alpha and green components
    pmullw      %xmm4, %xmm3            // Scale alpha and green

    psrlw       $8, %xmm5               // Combine results
    pblendvb    %xmm5, %xmm3            // Mask in %xmm0, implicitly
    paddb       %xmm3, %xmm1            // Add source and destination pixels together

.LReallySmallStore:
    cmpl        $2, %ecx                // Check how many pixels should be written
    pextrd      $0x0, %xmm1, (%rdx)     // Store one destination pixel
    jb          .LReallySmallExit
    pextrd      $0x1, %xmm1, 4(%rdx)    // Store second destination pixel
    je          .LReallySmallExit
    pextrd      $0x2, %xmm1, 8(%rdx)    // Store third destination pixel
.LReallySmallExit:
    ret

    // Handle bigger blit operations (16+ pixels)
    ////////////////////////////////////////////////////////////////////////////////
    .p2align 4
.LBigBlit:
    // Align destination?
    testl       $0xF, %edx
    lddqu       (%rax), %xmm1           // Pre-load four source pixels
    jz          .LAligned

    movq        %rdx, %rdi              // Calculate alignment of destination pointer
    negq        %rdi
    andl        $0xF, %edi

    // Handle 1-3 pixels to align destination
    ptest       %xmm7, %xmm1            // Check if all alphas are zero or opaque
    jz          .LAlignDone             // If all alphas are zero, just skip
    lddqu       (%rdx), %xmm5           // Load four destination pixels
    jc          .LAlignStore            // If all alphas are opaque, just store

    // Handle mixed alphas (calculate and scale)
    EXTRACT_ALPHA(xmm1, xmm2)           // Extract and clone alpha value

    psllw       $8, %xmm3               // Filter out red and blue components
    pmulhuw     %xmm4, %xmm3            // Scale red and blue
    movdqa      %xmm5, %xmm2
    psrlw       $8, %xmm2               // Filter out alpha and green components
    pmullw      %xmm4, %xmm2            // Scale alpha and green

    pblendvb    %xmm3, %xmm2            // Combine results (mask in %xmm0, implicitly)
    paddb       %xmm2, %xmm1            // Add source and destination pixels together

.LAlignStore:
    cmpl        $8, %edi                // Check how many pixels should be written
    jb          .LAlignPixelsLeft1
    ja          .LAlignPixelsLeft3
    pblendw     $0x0F, %xmm1, %xmm5     // Blend two pixels
    jmp .LAlignStorePixels

.LAlignPixelsLeft1:
    pblendw     $0x03, %xmm1, %xmm5     // Blend one pixel
    jmp .LAlignStorePixels

.LAlignPixelsLeft3:
    pblendw     $0x3F, %xmm1, %xmm5     // Blend three pixels

.LAlignStorePixels:
    movdqu      %xmm5, (%rdx)           // Store destination pixels

.LAlignDone:
    addq        %rdi, %rax              // Adjust pointers and pixel count
    addq        %rdi, %rdx
    shrq        $2, %rdi
    lddqu       (%rax), %xmm1           // Pre-load new source pixels (after alignment)
    subl        %edi, %ecx

.LAligned:                              // Destination is guaranteed to be 16 byte aligned
    xorq        %rdi, %rdi              // Reset offset to zero
    subl        $8, %ecx                // Decrease counter (Reserve four pixels for the cleanup)
    testl       $0xF, %eax              // Check alignment of source pointer
    jz          .LAlignedLoop

    // Source not aligned to destination
    ////////////////////////////////////////////////////////////////////////////////
    .p2align 4
.LUnalignedLoop:                        // Main loop for unaligned, handles eight pixels per iteration
    ptest       %xmm7, %xmm1            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero00
    lddqu       16(%rax, %rdi), %xmm2   // Pre-load four source pixels
    jz          .LAlphaZero00
    movdqa      %xmm1, (%rdx, %rdi)     // Store four destination pixels

.LAlphaZero00:
    ptest       %xmm7, %xmm2            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero01
    lddqu       32(%rax, %rdi), %xmm1   // Pre-load four source pixels
    jz          .LAlphaZero01
    movdqa      %xmm2, 16(%rdx, %rdi)   // Store four destination pixels

.LAlphaZero01:
    addq        $32, %rdi               // Adjust offset and pixel count
    subl        $8, %ecx
    jae         .LUnalignedLoop
    addl        $8, %ecx                // Adjust pixel count
    jmp         .LLoopCleanup0

    .p2align 4
.LAlphaNotOpaqueOrZero00:
    movdqa      (%rdx, %rdi), %xmm5     // Load four destination pixels
    EXTRACT_ALPHA(xmm1, xmm2)           // Extract and clone alpha value
    SCALE_PIXELS                        // Scale pixels using alpha

    lddqu       16(%rax, %rdi), %xmm2   // Pre-load four source pixels
    pblendvb    %xmm5, %xmm3            // Combine results (mask in %xmm0, implicitly)
    paddb       %xmm3, %xmm1            // Add source and destination pixels together
    movdqa      %xmm1, (%rdx, %rdi)     // Store four destination pixels

    // Handle next four pixels
    ptest       %xmm7, %xmm2            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero01
    lddqu       32(%rax, %rdi), %xmm1   // Pre-load four source pixels
    jz          .LAlphaZero02
    movdqa      %xmm2, 16(%rdx, %rdi)   // Store four destination pixels
.LAlphaZero02:
    addq        $32, %rdi               // Adjust offset and pixel count
    subl        $8, %ecx
    jae         .LUnalignedLoop
    addl        $8, %ecx                // Adjust pixel count
    jmp         .LLoopCleanup0

    .p2align 4
.LAlphaNotOpaqueOrZero01:
    movdqa      16(%rdx, %rdi), %xmm5   // Load four destination pixels
    EXTRACT_ALPHA(xmm2, xmm1)           // Extract and clone alpha value
    SCALE_PIXELS                        // Scale pixels using alpha

    lddqu       32(%rax, %rdi), %xmm1   // Pre-load four source pixels
    addq        $32, %rdi
    pblendvb    %xmm5, %xmm3            // Combine results (mask in %xmm0, implicitly)
    paddb       %xmm3, %xmm2            // Add source and destination pixels together
    subl        $8, %ecx
    movdqa      %xmm2, -16(%rdx, %rdi)  // Store four destination pixels
    jae         .LUnalignedLoop
    addl        $8, %ecx                // Adjust pixel count

    // Cleanup - handle pending pixels from loop
.LLoopCleanup0:
    ptest       %xmm7, %xmm1            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero02
    jz          .LAlphaZero03
    movdqa      %xmm1, (%rdx, %rdi)     // Store four destination pixels
.LAlphaZero03:
    addq        $16, %rdi
    subl        $4, %ecx
    js          .LSmallRemaining        // Reuse code from small loop

.LRemain0:
    lddqu       (%rax, %rdi), %xmm1     // Load four source pixels
    ptest       %xmm7, %xmm1            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero02
    jz          .LAlphaZero04
    movdqa      %xmm1, (%rdx, %rdi)     // Store four destination pixels
.LAlphaZero04:
    addq        $16, %rdi
    subl        $4, %ecx
    jmp         .LSmallRemaining        // Reuse code from small loop

.LAlphaNotOpaqueOrZero02:
    movdqa      (%rdx, %rdi), %xmm5     // Load four destination pixels
    EXTRACT_ALPHA(xmm1, xmm2)           // Extract and clone alpha value
    SCALE_PIXELS                        // Scale pixels using alpha

    addq        $16, %rdi
    subl        $4, %ecx
    pblendvb    %xmm5, %xmm3            // Combine results (mask in %xmm0, implicitly)
    paddb       %xmm3, %xmm1            // Add source and destination pixels together
    movdqa      %xmm1, -16(%rdx, %rdi)  // Store four destination pixels
    js          .LSmallRemaining        // Reuse code from small loop
    jmp         .LRemain0

    // Source aligned to destination
    ////////////////////////////////////////////////////////////////////////////////
    .p2align 4
.LAlignedLoop:                          // Main loop for aligned, handles eight pixels per iteration
    ptest       %xmm7, %xmm1            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero10
    movdqa      16(%rax, %rdi), %xmm2   // Pre-load four source pixels
    jz          .LAlphaZero10
    movdqa      %xmm1, (%rdx, %rdi)     // Store four destination pixels

.LAlphaZero10:
    ptest       %xmm7, %xmm2            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero11
    movdqa      32(%rax, %rdi), %xmm1   // Pre-load four source pixels
    jz          .LAlphaZero11
    movdqa      %xmm2, 16(%rdx, %rdi)   // Store four destination pixels

.LAlphaZero11:
    addq        $32, %rdi               // Adjust offset and pixel count
    subl        $8, %ecx
    jae         .LAlignedLoop
    addl        $8, %ecx                // Adjust pixel count
    jmp         .LLoopCleanup1

    .p2align 4
.LAlphaNotOpaqueOrZero10:
    movdqa      (%rdx, %rdi), %xmm5     // Load four destination pixels
    EXTRACT_ALPHA(xmm1, xmm2)           // Extract and clone alpha value
    SCALE_PIXELS                        // Scale pixels using alpha

    movdqa      16(%rax, %rdi), %xmm2   // Pre-load four source pixels
    pblendvb    %xmm5, %xmm3            // Combine results (mask in %xmm0, implicitly)
    paddb       %xmm3, %xmm1            // Add source and destination pixels together
    movdqa      %xmm1, (%rdx, %rdi)     // Store four destination pixels

    // Handle next four pixels
    ptest       %xmm7, %xmm2            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero11
    movdqa      32(%rax, %rdi), %xmm1   // Pre-load four source pixels
    jz          .LAlphaZero12
    movdqa      %xmm2, 16(%rdx, %rdi)   // Store four destination pixels
.LAlphaZero12:
    addq        $32, %rdi               // Adjust offset and pixel count
    subl        $8, %ecx
    jae         .LAlignedLoop
    addl        $8, %ecx                // Adjust pixel count
    jmp         .LLoopCleanup1

    .p2align 4
.LAlphaNotOpaqueOrZero11:
    movdqa      16(%rdx, %rdi), %xmm5   // Load four destination pixels
    EXTRACT_ALPHA(xmm2, xmm1)           // Extract and clone alpha value
    SCALE_PIXELS                        // Scale pixels using alpha
    movdqa      32(%rax, %rdi), %xmm1   // Pre-load four source pixels

    addq        $32, %rdi
    pblendvb    %xmm5, %xmm3            // Combine results (mask in %xmm0, implicitly)
    paddb       %xmm3, %xmm2            // Add source and destination pixels together
    subl        $8, %ecx
    movdqa      %xmm2, -16(%rdx, %rdi)  // Store four destination pixels
    jae         .LAlignedLoop
    addl        $8, %ecx                // Adjust pixel count

    // Cleanup - handle four pending pixels from loop
.LLoopCleanup1:
    ptest       %xmm7, %xmm1            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero12
    jz          .LAlphaZero13
    movdqa      %xmm1, (%rdx, %rdi)     // Store four destination pixels
.LAlphaZero13:
    addq        $16, %rdi
    subl        $4, %ecx
    js          .LSmallRemaining        // Reuse code from small loop

.LRemain1:
    movdqa      (%rax, %rdi), %xmm1     // Pre-load four source pixels
    ptest       %xmm7, %xmm1            // Check if all alphas are zero or opaque
    ja          .LAlphaNotOpaqueOrZero12
    jz          .LAlphaZero14
    movdqa      %xmm1, (%rdx, %rdi)     // Store four destination pixels
.LAlphaZero14:
    addq        $16, %rdi
    subl        $4, %ecx
    jmp         .LSmallRemaining        // Reuse code from small loop

.LAlphaNotOpaqueOrZero12:
    movdqa      (%rdx, %rdi), %xmm5     // Load four destination pixels
    EXTRACT_ALPHA(xmm1, xmm2)           // Extract and clone alpha value
    SCALE_PIXELS                        // Scale pixels using alpha

    addq        $16, %rdi
    subl        $4, %ecx
    pblendvb    %xmm5, %xmm3            // Combine results (mask in %xmm0, implicitly)
    paddb       %xmm3, %xmm1            // Add source and destination pixels together
    movdqa      %xmm1, -16(%rdx, %rdi)  // Store four destination pixels
    js          .LSmallRemaining        // Reuse code from small loop
    jmp         .LRemain1

    .cfi_endproc
#ifndef __clang__
    .size S32A_Opaque_BlitRow32_SSE4_asm, .-S32A_Opaque_BlitRow32_SSE4_asm
#endif

    // Constants for SSE code
#ifndef __clang__
    .section .rodata
#endif
    .p2align 4
.LAlphaCheckMask:
    .long   0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000
.LInverseAlphaCalc:
    .word   256, 256, 256, 256, 256, 256, 256, 256
.LResultMergeMask:
    .long   0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF
#endif

#endif // CRBUG_399842_FIXED
