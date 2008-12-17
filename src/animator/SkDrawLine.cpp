/* libs/graphics/animator/SkDrawLine.cpp
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

#include "SkDrawLine.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkLine::fInfo[] = {
    SK_MEMBER(x1, Float),
    SK_MEMBER(x2, Float),
    SK_MEMBER(y1, Float),
    SK_MEMBER(y2, Float)
};

#endif

DEFINE_GET_MEMBER(SkLine);

SkLine::SkLine() : x1(0), x2(0), y1(0), y2(0) { 
}

bool SkLine::draw(SkAnimateMaker& maker) {
    SkBoundableAuto boundable(this, maker);
    maker.fCanvas->drawLine(x1, y1, x2, y2, *maker.fPaint);
    return false;
}
