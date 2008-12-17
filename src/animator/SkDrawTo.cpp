/* libs/graphics/animator/SkDrawTo.cpp
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

