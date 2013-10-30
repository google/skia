/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

void SkDilateX_SSE2(const SkPMColor* src, SkPMColor* dst, int radius,
                    int width, int height, int srcStride, int dstStride);
void SkDilateY_SSE2(const SkPMColor* src, SkPMColor* dst, int radius,
                    int width, int height, int srcStride, int dstStride);
void SkErodeX_SSE2(const SkPMColor* src, SkPMColor* dst, int radius,
                   int width, int height, int srcStride, int dstStride);
void SkErodeY_SSE2(const SkPMColor* src, SkPMColor* dst, int radius,
                   int width, int height, int srcStride, int dstStride);
