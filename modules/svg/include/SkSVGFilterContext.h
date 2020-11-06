/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFilterContext_DEFINED
#define SkSVGFilterContext_DEFINED

#include <vector>
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"

class SkImageFilter;
class SkString;
class SkSVGFeInputType;
class SkSVGRenderContext;

class SkSVGFilterContext {
public:
    SkSVGFilterContext(const SkRect& filterEffectsRegion)
            : fFilterEffectsRegion(filterEffectsRegion) {}

    const SkRect& filterEffectsRegion() const { return fFilterEffectsRegion; }

    void registerResult(const SkString&, const sk_sp<SkImageFilter>&);

    sk_sp<SkImageFilter> resolveInput(const SkSVGRenderContext&, const SkSVGFeInputType&) const;

private:
    struct Result {
        SkString fId;
        sk_sp<SkImageFilter> fResult;
    };

    sk_sp<SkImageFilter> findResultById(const SkString&) const;

    SkRect fFilterEffectsRegion;
    std::vector<Result> fResults;
};

#endif  // SkSVGFilterContext_DEFINED
