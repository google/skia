/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkComposeImageFilter.h"
#include "SkFlattenableBuffers.h"

SkComposeImageFilter::~SkComposeImageFilter() {
}

bool SkComposeImageFilter::onFilterImage(Proxy* proxy,
                                         const SkBitmap& src,
                                         const SkMatrix& ctm,
                                         SkBitmap* result,
                                         SkIPoint* loc) {
    SkImageFilter* outer = getInput(0);
    SkImageFilter* inner = getInput(1);

    if (!outer && !inner) {
        return false;
    }

    if (!outer || !inner) {
        return (outer ? outer : inner)->filterImage(proxy, src, ctm, result, loc);
    }

    SkBitmap tmp;
    return inner->filterImage(proxy, src, ctm, &tmp, loc) &&
           outer->filterImage(proxy, tmp, ctm, result, loc);
}

bool SkComposeImageFilter::onFilterBounds(const SkIRect& src,
                                          const SkMatrix& ctm,
                                          SkIRect* dst) {
    SkImageFilter* outer = getInput(0);
    SkImageFilter* inner = getInput(1);

    if (!outer && !inner) {
        return false;
    }

    if (!outer || !inner) {
        return (outer ? outer : inner)->filterBounds(src, ctm, dst);
    }

    SkIRect tmp;
    return inner->filterBounds(src, ctm, &tmp) &&
           outer->filterBounds(tmp, ctm, dst);
}

SkComposeImageFilter::SkComposeImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
}
