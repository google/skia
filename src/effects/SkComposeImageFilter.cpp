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

    SkBitmap tmp;
    return inner->filterImage(proxy, src, ctx, &tmp, offset) &&
           outer->filterImage(proxy, tmp, ctx, result, offset);
}

bool SkComposeImageFilter::onFilterBounds(const SkIRect& src,
                                          const SkMatrix& ctm,
                                          SkIRect* dst) const {
    SkImageFilter* outer = getInput(0);
    SkImageFilter* inner = getInput(1);

    SkIRect tmp;
    return inner->filterBounds(src, ctm, &tmp) && outer->filterBounds(tmp, ctm, dst);
}

SkFlattenable* SkComposeImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    return SkComposeImageFilter::Create(common.getInput(0), common.getInput(1));
}

#ifndef SK_IGNORE_TO_STRING
void SkComposeImageFilter::toString(SkString* str) const {
    SkImageFilter* outer = getInput(0);
    SkImageFilter* inner = getInput(1);

    str->appendf("SkComposeImageFilter: (");

    str->appendf("outer: ");
    outer->toString(str);

    str->appendf("inner: ");
    inner->toString(str);

    str->appendf(")");
}
#endif
