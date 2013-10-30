/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <SkColor.h>

/**
 * All morphology procs have the same signature: src is the source buffer, dst the
 * destination buffer, radius is the morphology radius, width and height are the bounds
 * of the destination buffer (in pixels), and srcStride and dstStride are the
 * number of pixels per row in each buffer. All buffers are 8888.
 */

typedef void (*SkMorphologyProc)(const SkPMColor* src, SkPMColor* dst, int radius,
                                 int width, int height, int srcStride, int dstStride);

enum SkMorphologyProcType {
    kDilateX_SkMorphologyProcType,
    kDilateY_SkMorphologyProcType,
    kErodeX_SkMorphologyProcType,
    kErodeY_SkMorphologyProcType
};

SkMorphologyProc SkMorphologyGetPlatformProc(SkMorphologyProcType type);
