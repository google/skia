/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitMask_DEFINED
#define SkBlitMask_DEFINED

#include "include/core/SkColor.h"

namespace SkOpts {
    // Optimized mask-blit routine
    extern void (*blit_mask_d32_a8)(SkPMColor* dst, size_t dstRB,
                                    const SkAlpha* mask, size_t maskRB,
                                    SkColor color, int w, int h);

    void Init_BlitMask();
}  // namespace SkOpts

#endif // SkBlitMask_DEFINED
