/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfContext.h"
#include "SkPdfNativeTokenizer.h"

SkPdfContext::SkPdfContext(SkPdfNativeDoc* doc)
    : fPdfDoc(doc)
    , fTmpPageAllocator(new SkPdfAllocator()) {
}

SkPdfContext::~SkPdfContext() {
    delete fTmpPageAllocator;
}
