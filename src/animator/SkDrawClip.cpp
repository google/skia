/* libs/graphics/animator/SkDrawClip.cpp
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

#include "SkDrawClip.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkDrawRectangle.h"
#include "SkDrawPath.h"


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawClip::fInfo[] = {
    SK_MEMBER(path, Path),
    SK_MEMBER(rect, Rect)
};

#endif

DEFINE_GET_MEMBER(SkDrawClip);

SkDrawClip::SkDrawClip() : rect(NULL), path(NULL) {
}

bool SkDrawClip::draw(SkAnimateMaker& maker ) {
    if (rect != NULL)
        maker.fCanvas->clipRect(rect->fRect);
    else {
        SkASSERT(path != NULL);
        maker.fCanvas->clipPath(path->fPath);
    }
    return false;
}

