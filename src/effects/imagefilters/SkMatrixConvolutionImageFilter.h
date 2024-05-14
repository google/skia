/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixConvolutionImageFilter_DEFINED
#define SkMatrixConvolutionImageFilter_DEFINED

namespace MatrixConvolutionImageFilter {

// The matrix convolution image filter applies the convolution naively, it does not use any DFT to
// convert the input images into the frequency domain. As such, kernels can quickly become too
// slow to run in a reasonable amount of time (and anyone using a giant kernel should not be
// relying on Skia to perform the calculations). 256 as a limit on the kernel size is somewhat
// arbitrary but should, hopefully, not cause existing clients/websites to fail when historically
// there was no upper limit.
// Note: SkSL balks (w/ a "program is too large" error) whenever the number of kernel values
// is >= 2048 (e.g., 8x256, 16x128, ...) so that should be a pretty good upper limit for what
// is being seen in the wild.
static constexpr int kLargeKernelSize = 256;

// The texture-based implementation has two levels: a small size version and one at the
// maximum kernel size. In either case, the texture is a 1D array that can hold any
// width/height combination that fits within it.
static constexpr int kSmallKernelSize = 64;

// The uniform-based kernel shader can store 28 values in any order layout (28x1, 1x25, 5x5, and
// smaller orders like 3x3 or 5x4, etc.), but must be a multiple of 4 for better packing in std140.
static constexpr int kMaxUniformKernelSize = 28;

} // namespace MatrixConvolutionImageFilter

#endif // SkMatrixConvolutionImageFilter_DEFINED
