
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawSaveLayer.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkDrawPaint.h"
#include "SkDrawRectangle.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkSaveLayer::fInfo[] = {
    SK_MEMBER(bounds, Rect),
    SK_MEMBER(paint, Paint)
};

#endif

DEFINE_GET_MEMBER(SkSaveLayer);

SkSaveLayer::SkSaveLayer() : paint(NULL), bounds(NULL) {
}

SkSaveLayer::~SkSaveLayer(){
}

bool SkSaveLayer::draw(SkAnimateMaker& maker)
{
    if (!bounds) {
        return false;
    }
    SkPaint* save = maker.fPaint;   
    //paint is an SkDrawPaint
    if (paint)
    {
        SkPaint realPaint;
        paint->setupPaint(&realPaint);
        maker.fCanvas->saveLayer(&bounds->fRect, &realPaint, SkCanvas::kHasAlphaLayer_SaveFlag);
    }
    else
        maker.fCanvas->saveLayer(&bounds->fRect, save, SkCanvas::kHasAlphaLayer_SaveFlag);
    SkPaint local = SkPaint(*maker.fPaint);
    maker.fPaint = &local;
    bool result = INHERITED::draw(maker);
    maker.fPaint = save;
    maker.fCanvas->restore();
    return result;
}

#ifdef SK_DUMP_ENABLED
void SkSaveLayer::dump(SkAnimateMaker* maker)
{
    dumpBase(maker);
    //would dump enabled be defined but not debug?
#ifdef SK_DEBUG
    if (paint)
        SkDebugf("paint=\"%s\" ", paint->id);
    if (bounds)
        SkDebugf("bounds=\"%s\" ", bounds->id);
#endif
    dumpDrawables(maker);
}
#endif

void SkSaveLayer::onEndElement(SkAnimateMaker& maker)
{
    if (!bounds)
        maker.setErrorCode(SkDisplayXMLParserError::kSaveLayerNeedsBounds);
    INHERITED::onEndElement(maker);
}


