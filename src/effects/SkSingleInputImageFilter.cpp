/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSingleInputImageFilter.h"
#include "SkBitmap.h"
#include "SkFlattenableBuffers.h"
#include "SkMatrix.h"

SkSingleInputImageFilter::SkSingleInputImageFilter(SkImageFilter* input) : INHERITED(input) {
}

SkSingleInputImageFilter::~SkSingleInputImageFilter() {
}

SkSingleInputImageFilter::SkSingleInputImageFilter(SkFlattenableReadBuffer& rb)
    : INHERITED(rb) {
}

void SkSingleInputImageFilter::flatten(SkFlattenableWriteBuffer& wb) const {
    this->INHERITED::flatten(wb);
}

SkBitmap SkSingleInputImageFilter::getInputResult(Proxy* proxy,
                                                  const SkBitmap& src,
                                                  const SkMatrix& ctm,
                                                  SkIPoint* offset) {
    return this->INHERITED::getInputResult(0, proxy, src, ctm, offset);
}
