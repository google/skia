/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSDFMaskFilter_DEFINED
#define GrSDFMaskFilter_DEFINED

#include "include/core/SkMaskFilter.h"

/** \class GrSDFMaskFilter

    This mask filter converts an alpha mask to a signed distance field representation
*/
class GrSDFMaskFilter : public SkMaskFilter {
public:
    static sk_sp<SkMaskFilter> Make();
};

extern void gr_register_sdf_maskfilter_createproc();

#endif
