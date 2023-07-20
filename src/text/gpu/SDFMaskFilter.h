/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_SDFMaskFilter_DEFINED
#define sktext_gpu_SDFMaskFilter_DEFINED

#include "include/core/SkTypes.h"

#if !defined(SK_DISABLE_SDF_TEXT)

#include "include/core/SkMaskFilter.h"
#include "include/core/SkRefCnt.h"

namespace sktext::gpu {

/** \class SDFMaskFilter

    This mask filter converts an alpha mask to a signed distance field representation
*/
class SDFMaskFilter : public SkMaskFilter {
public:
    static sk_sp<SkMaskFilter> Make();
};

}  // namespace sktext::gpu

#endif // !defined(SK_DISABLE_SDF_TEXT)

#endif
