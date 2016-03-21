/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkComposeImageFilter.h"

#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkWriteBuffer.h"


void SkComposeImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    SkImageFilter* outer = getInput(0);
    SkImageFilter* inner = getInput(1);

    SkRect tmp;
    inner->computeFastBounds(src, &tmp);
    outer->computeFastBounds(tmp, dst);
}

SkSpecialImage* SkComposeImageFilter::onFilterImage(SkSpecialImage* source, const Context& ctx,
                                                    SkIPoint* offset) const {
    // The bounds passed to the inner filter must be filtered by the outer
    // filter, so that the inner filter produces the pixels that the outer
    // filter requires as input. This matters if the outer filter moves pixels.
    SkIRect innerClipBounds;
    getInput(0)->filterBounds(ctx.clipBounds(), ctx.ctm(), &innerClipBounds);
    Context innerContext(ctx.ctm(), innerClipBounds, ctx.cache());
    SkIPoint innerOffset = SkIPoint::Make(0, 0);
    SkAutoTUnref<SkSpecialImage> inner(this->filterInput(1, source, innerContext, &innerOffset));
    if (!inner) {
        return nullptr;
    }

    SkMatrix outerMatrix(ctx.ctm());
    outerMatrix.postTranslate(SkIntToScalar(-innerOffset.x()), SkIntToScalar(-innerOffset.y()));
    SkIRect clipBounds = ctx.clipBounds();
    clipBounds.offset(-innerOffset.x(), -innerOffset.y());
    Context outerContext(outerMatrix, clipBounds, ctx.cache());

    SkIPoint outerOffset = SkIPoint::Make(0, 0);
    SkAutoTUnref<SkSpecialImage> outer(this->filterInput(0, inner, outerContext, &outerOffset));
    if (!outer) {
        return nullptr;
    }

    *offset = innerOffset + outerOffset;
    return outer.release();
}

bool SkComposeImageFilter::onFilterBounds(const SkIRect& src,
                                          const SkMatrix& ctm,
                                          SkIRect* dst,
                                          MapDirection direction) const {
    SkImageFilter* outer = this->getInput(0);
    SkImageFilter* inner = this->getInput(1);

    SkIRect tmp;
    return inner->filterBounds(src, ctm, &tmp, direction) &&
           outer->filterBounds(tmp, ctm, dst, direction);
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
