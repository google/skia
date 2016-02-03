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

void SkComposeImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    SkImageFilter* outer = getInput(0);
    SkImageFilter* inner = getInput(1);

    SkRect tmp;
    inner->computeFastBounds(src, &tmp);
    outer->computeFastBounds(tmp, dst);
}

bool SkComposeImageFilter::onFilterImage(Proxy* proxy,
                                         const SkBitmap& src,
                                         const Context& ctx,
                                         SkBitmap* result,
                                         SkIPoint* offset) const {
    SkBitmap tmp;
    SkIPoint innerOffset = SkIPoint::Make(0, 0);
    SkIPoint outerOffset = SkIPoint::Make(0, 0);
    if (!this->filterInput(1, proxy, src, ctx, &tmp, &innerOffset))
        return false;

    SkMatrix outerMatrix(ctx.ctm());
    outerMatrix.postTranslate(SkIntToScalar(-innerOffset.x()), SkIntToScalar(-innerOffset.y()));
    SkIRect clipBounds = ctx.clipBounds();
    clipBounds.offset(-innerOffset.x(), -innerOffset.y());
    Context outerContext(outerMatrix, clipBounds, ctx.cache());
    if (!this->filterInput(0, proxy, tmp, outerContext, result, &outerOffset)) {
        return false;
    }

    *offset = innerOffset + outerOffset;
    return true;
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
