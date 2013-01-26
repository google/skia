
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawTo.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkDrawBitmap.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawTo::fInfo[] = {
    SK_MEMBER(drawOnce, Boolean),
    SK_MEMBER(use, Bitmap)
};

#endif

DEFINE_GET_MEMBER(SkDrawTo);

SkDrawTo::SkDrawTo() : drawOnce(false), use(NULL), fDrawnOnce(false) {
}

#if 0
SkDrawTo::~SkDrawTo() {
    SkASSERT(0);
}
#endif

bool SkDrawTo::draw(SkAnimateMaker& maker) {
    if (fDrawnOnce)
        return false;
    SkCanvas canvas(use->fBitmap);
    SkCanvas* save = maker.fCanvas;
    maker.fCanvas = &canvas;
    INHERITED::draw(maker);
    maker.fCanvas = save;
    fDrawnOnce = drawOnce;
    return false;
}

#ifdef SK_DUMP_ENABLED
void SkDrawTo::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    dumpAttrs(maker);
    if (use)
        SkDebugf("use=\"%s\" ", use->id);
    dumpDrawables(maker);
}
#endif
