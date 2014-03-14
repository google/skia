/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkComposeImageFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkComposeImageFilter::~SkComposeImageFilter() {
}

bool SkComposeImageFilter::onFilterImage(Proxy* proxy,
                                         const SkBitmap& src,
                                         const Context& ctx,
                                         SkBitmap* result,
                                         SkIPoint* offset) const {
    SkImageFilter* outer = getInput(0);
    SkImageFilter* inner = getInput(1);

    if (!outer && !inner) {
        return false;
    }

    if (!outer || !inner) {
        return (outer ? outer : inner)->filterImage(proxy, src, ctx, result, offset);
    }

    SkBitmap tmp;
    return inner->filterImage(proxy, src, ctx, &tmp, offset) &&
           outer->filterImage(proxy, tmp, ctx, result, offset);
}

bool SkComposeImageFilter::onFilterBounds(const SkIRect& src,
                                          const SkMatrix& ctm,
                                          SkIRect* dst) const {
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

SkComposeImageFilter::SkComposeImageFilter(SkReadBuffer& buffer)
  : INHERITED(2, buffer) {
}
