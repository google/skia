/* libs/graphics/animator/SkDrawSaveLayer.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
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


