/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGFe.h"
#include "modules/svg/include/SkSVGFilterContext.h"

sk_sp<SkImageFilter> SkSVGFe::makeImageFilter(const SkSVGRenderContext& ctx,
                                              SkSVGFilterContext* fctx) const {
    return this->onMakeImageFilter(ctx, *fctx);
}
