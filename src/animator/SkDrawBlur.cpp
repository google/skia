/* libs/graphics/animator/SkDrawBlur.cpp
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

#include "SkDrawBlur.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawBlur::fInfo[] = {
    SK_MEMBER(blurStyle, MaskFilterBlurStyle),
    SK_MEMBER(radius, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawBlur);

SkDrawBlur::SkDrawBlur() : radius(-1), 
    blurStyle(SkBlurMaskFilter::kNormal_BlurStyle) {
}

SkMaskFilter* SkDrawBlur::getMaskFilter() {
    if (radius < 0)
        return NULL;
    return SkBlurMaskFilter::Create(radius, (SkBlurMaskFilter::BlurStyle) blurStyle);
}

