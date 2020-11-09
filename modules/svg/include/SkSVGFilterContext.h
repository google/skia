/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFilterContext_DEFINED
#define SkSVGFilterContext_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/SkTHash.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkImageFilter;
class SkSVGFeInputType;
class SkSVGRenderContext;

class SkSVGFilterContext {
public:
    SkSVGFilterContext(const SkRect& filterEffectsRegion)
            : fFilterEffectsRegion(filterEffectsRegion) {}

    const SkRect& filterEffectsRegion() const { return fFilterEffectsRegion; }

    void registerResult(const SkSVGStringType&, const sk_sp<SkImageFilter>&);

    sk_sp<SkImageFilter> resolveInput(const SkSVGRenderContext&, const SkSVGFeInputType&) const;

private:
    sk_sp<SkImageFilter> findResultById(const SkSVGStringType&) const;

    SkRect fFilterEffectsRegion;

    SkTHashMap<SkSVGStringType, sk_sp<SkImageFilter>> fResults;
};

#endif  // SkSVGFilterContext_DEFINED
