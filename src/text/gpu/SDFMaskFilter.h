/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_SDFMaskFilter_DEFINED
#define sktext_gpu_SDFMaskFilter_DEFINED

#include "include/core/SkMaskFilter.h"

namespace sktext::gpu {

/** \class SDFMaskFilter

    This mask filter converts an alpha mask to a signed distance field representation
*/
class SDFMaskFilter : public SkMaskFilter {
public:
    static sk_sp<SkMaskFilter> Make();
};

extern void register_sdf_maskfilter_createproc();

}  // namespace sktext::gpu

#endif
