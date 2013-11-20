/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"
#include "SkTDStackNester.h"
#include "SkPdfGraphicsState.h"

class SkPdfAllocator;
class SkPdfNativeDoc;
class SkPdfNativeObject;

/** \class SkPdfContext
 *   The context of the drawing. The document we draw from, the current stack of objects, ...
 */
class SkPdfContext {
public:
    SkTDStackNester<SkPdfNativeObject*>  fObjectStack;
    SkTDStackNester<SkPdfGraphicsState>  fStateStack;
    SkPdfGraphicsState              fGraphicsState;
    SkPdfNativeDoc*                 fPdfDoc;
    SkPdfAllocator*                 fTmpPageAllocator;
    SkMatrix                        fOriginalMatrix;

    SkPdfContext(SkPdfNativeDoc* doc);
    ~SkPdfContext();
};
