/* libs/graphics/animator/SkDisplayBounds.cpp
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

#include "SkDisplayBounds.h"
#include "SkAnimateMaker.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayBounds::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(inval, Boolean)
};

#endif

DEFINE_GET_MEMBER(SkDisplayBounds);

SkDisplayBounds::SkDisplayBounds() : inval(false) {
}

bool SkDisplayBounds::draw(SkAnimateMaker& maker) {
    maker.fDisplayList.fUnionBounds = SkToBool(inval);
    maker.fDisplayList.fDrawBounds = false;
    fBounds.setEmpty();
    bool result = INHERITED::draw(maker);
    maker.fDisplayList.fUnionBounds = false;
    maker.fDisplayList.fDrawBounds = true;
    if (inval && fBounds.isEmpty() == false) {
        SkIRect& rect = maker.fDisplayList.fInvalBounds;
        maker.fDisplayList.fHasUnion = true;
        if (rect.isEmpty())
            rect = fBounds;
        else 
            rect.join(fBounds);
    }
    return result;
}



