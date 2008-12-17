/* libs/graphics/animator/SkDrawEmboss.cpp
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

#include "SkDrawEmboss.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawEmboss::fInfo[] = {
    SK_MEMBER(ambient, Float),
    SK_MEMBER_ARRAY(direction, Float),
    SK_MEMBER(radius, Float),
    SK_MEMBER(specular, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawEmboss);

SkDrawEmboss::SkDrawEmboss() : radius(-1) { 
    direction.setCount(3);
}

SkMaskFilter* SkDrawEmboss::getMaskFilter() {
    if (radius < 0 || direction.count() !=3)
        return NULL;
    return SkBlurMaskFilter::CreateEmboss(direction.begin(), ambient, specular, radius);
}

