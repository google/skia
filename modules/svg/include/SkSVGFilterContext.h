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
#include "include/private/SkTHash.h"

class SkImageFilter;
class SkPicture;
class SkString;

class SkSVGFilterContext {
public:
    SkSVGFilterContext(const SkRect& filterEffectsRegion)
            : fFilterEffectsRegion(filterEffectsRegion) {}

    sk_sp<SkImageFilter> findResultById(const SkString& id) const;

    const SkRect& filterEffectsRegion() const { return fFilterEffectsRegion; }

private:
    SkRect fFilterEffectsRegion;
};

#endif  // SkSVGFilterContext_DEFINED
