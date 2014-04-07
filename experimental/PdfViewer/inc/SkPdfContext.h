/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfContext_DEFINED
#define SkPdfContext_DEFINED

#include "SkMatrix.h"
#include "SkPdfGraphicsState.h"
#include "SkPdfNativeTokenizer.h"
#include "SkTDStackNester.h"
#include "SkTypes.h"

class SkCanvas;
class SkPdfNativeDoc;
class SkPdfNativeObject;

/**
 *   The context of the drawing. The document we draw from, the current stack of
 *   objects, ...
 */
class SkPdfContext : SkNoncopyable {
public:
    // FIXME (scroggo): Add functions for accessing these.
    SkTDStackNester<SkPdfNativeObject*>  fObjectStack;
    SkTDStackNester<SkPdfGraphicsState>  fStateStack;
    SkPdfGraphicsState              fGraphicsState;
    SkPdfNativeDoc*                 fPdfDoc;
    SkMatrix                        fOriginalMatrix;

    // Does not take ownership of the doc.
    explicit SkPdfContext(SkPdfNativeDoc* doc);

    /**
     *  Parse the stream and draw its commands to the canvas.
     *  FIXME (scroggo): May not be the best place for this, but leaving here
     *  for now, since it uses SkPdfContext's members.
     */
    void parseStream(SkPdfNativeObject* stream, SkCanvas* canvas);

private:
    // FIXME (scroggo): Is this the right place for the allocator?
    SkPdfAllocator fTmpPageAllocator;
};
#endif // SkPdfContext_DEFINED
