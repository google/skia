/* libs/graphics/animator/SkHitClear.cpp
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

#include "SkHitClear.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkHitClear::fInfo[] = {
    SK_MEMBER_ARRAY(targets, Displayable)
};

#endif

DEFINE_GET_MEMBER(SkHitClear);

bool SkHitClear::enable(SkAnimateMaker& maker) {
    for (int tIndex = 0; tIndex < targets.count(); tIndex++) {
        SkDisplayable* target = targets[tIndex];
        target->clearBounder();
    }
    return true;
}

bool SkHitClear::hasEnable() const {
    return true;
}

