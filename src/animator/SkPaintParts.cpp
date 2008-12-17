/* libs/graphics/animator/SkPaintParts.cpp
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

#include "SkPaintParts.h"
#include "SkDrawPaint.h"
#ifdef SK_DUMP_ENABLED
#include "SkDisplayList.h"
#include "SkDump.h"
#endif

SkPaintPart::SkPaintPart() : fPaint(NULL) {
}

SkDisplayable* SkPaintPart::getParent() const {
    return fPaint;
}

bool SkPaintPart::setParent(SkDisplayable* parent) {
    SkASSERT(parent != NULL);
    if (parent->isPaint() == false)
        return true;
    fPaint = (SkDrawPaint*) parent;
    return false;
}


// SkDrawMaskFilter
bool SkDrawMaskFilter::add() {
    if (fPaint->maskFilter != (SkDrawMaskFilter*) -1)
        return true;
    fPaint->maskFilter = this;
    fPaint->fOwnsMaskFilter = true;
    return false;
}

SkMaskFilter* SkDrawMaskFilter::getMaskFilter() {
    return NULL;
}


// SkDrawPathEffect
bool SkDrawPathEffect::add() {
    if (fPaint->isPaint()) {
        if (fPaint->pathEffect != (SkDrawPathEffect*) -1)
            return true;
        fPaint->pathEffect = this;
        fPaint->fOwnsPathEffect = true;
        return false;
    }
    fPaint->add(*(SkAnimateMaker*) NULL, this);
    return false;
}

SkPathEffect* SkDrawPathEffect::getPathEffect() {
    return NULL;
}


// SkDrawShader
SkShader* SkDrawShader::getShader() {
    return NULL;
}


// Typeface
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawTypeface::fInfo[] = {
    SK_MEMBER(fontName, String),
    SK_MEMBER(style, FontStyle)
};

#endif

DEFINE_GET_MEMBER(SkDrawTypeface);

SkDrawTypeface::SkDrawTypeface() : style (SkTypeface::kNormal){
}

bool SkDrawTypeface::add() {
    if (fPaint->typeface != (SkDrawTypeface*) -1)
        return true;
    fPaint->typeface = this;
    fPaint->fOwnsTypeface = true;
    return false;
}

#ifdef SK_DUMP_ENABLED
void SkDrawTypeface::dump(SkAnimateMaker* maker) {
    SkDebugf("%*s<typeface fontName=\"%s\" ", SkDisplayList::fIndent, "", fontName.c_str());
    SkString string;
    SkDump::GetEnumString(SkType_FontStyle, style, &string);
    SkDebugf("style=\"%s\" />\n", string.c_str());
}
#endif


