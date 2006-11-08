/* libs/graphics/animator/SkDrawPoint.cpp
**
** Copyright 2006, Google Inc.
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

#include "SkDrawPoint.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo Sk_Point::fInfo[] = {
    SK_MEMBER_ALIAS(x, fPoint.fX, Float),
    SK_MEMBER_ALIAS(y, fPoint.fY, Float)
};

#endif

DEFINE_NO_VIRTUALS_GET_MEMBER(Sk_Point);

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawPoint::fInfo[] = {
    SK_MEMBER_ALIAS(x, fPoint.fX, Float),
    SK_MEMBER_ALIAS(y, fPoint.fY, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawPoint);

SkDrawPoint::SkDrawPoint() { 
    fPoint.set(0, 0);   
}

void SkDrawPoint::getBounds(SkRect* rect ) {
    rect->fLeft = rect->fRight = fPoint.fX;
    rect->fTop = rect->fBottom = fPoint.fY;
}


