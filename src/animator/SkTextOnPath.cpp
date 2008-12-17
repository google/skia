/* libs/graphics/animator/SkTextOnPath.cpp
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

#include "SkTextOnPath.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkDrawPath.h"
#include "SkDrawText.h"
#include "SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkTextOnPath::fInfo[] = {
    SK_MEMBER(offset, Float),
    SK_MEMBER(path, Path),
    SK_MEMBER(text, Text)
};

#endif

DEFINE_GET_MEMBER(SkTextOnPath);

SkTextOnPath::SkTextOnPath() : offset(0), path(NULL), text(NULL) {
}

bool SkTextOnPath::draw(SkAnimateMaker& maker) {
    SkASSERT(text);
    SkASSERT(path);
    SkBoundableAuto boundable(this, maker);
    maker.fCanvas->drawTextOnPathHV(text->getText(), text->getSize(), 
                                    path->getPath(), offset, 0, *maker.fPaint);
    return false;
}
