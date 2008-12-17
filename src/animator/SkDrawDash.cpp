/* libs/graphics/animator/SkDrawDash.cpp
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

#include "SkDrawDash.h"
#include "SkDashPathEffect.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDash::fInfo[] = {
    SK_MEMBER_ARRAY(intervals, Float),
    SK_MEMBER(phase, Float)
};

#endif

DEFINE_GET_MEMBER(SkDash);

SkDash::SkDash() : phase(0) {
}

SkDash::~SkDash() {
}

SkPathEffect* SkDash::getPathEffect() {
    int count = intervals.count();
    if (count == 0)
        return NULL;
    return new SkDashPathEffect(intervals.begin(), count, phase);
}

