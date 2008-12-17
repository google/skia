/* libs/graphics/animator/SkDrawDiscrete.cpp
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

#include "SkDrawDiscrete.h"
#include "SkAnimateMaker.h"
#include "SkPaint.h"
#include "SkDiscretePathEffect.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDiscrete::fInfo[] = {
    SK_MEMBER(deviation, Float),
    SK_MEMBER(segLength, Float)
};

#endif

DEFINE_GET_MEMBER(SkDiscrete);

SkDiscrete::SkDiscrete() : deviation(0), segLength(0) {
}

SkPathEffect* SkDiscrete::getPathEffect() {
    if (deviation <= 0 || segLength <= 0)
        return NULL;
    else
        return new SkDiscretePathEffect(segLength, deviation);
}

