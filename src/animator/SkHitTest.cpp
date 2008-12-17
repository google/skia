/* libs/graphics/animator/SkHitTest.cpp
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

#include "SkHitTest.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkHitTest::fInfo[] = {
    SK_MEMBER_ARRAY(bullets, Displayable),
    SK_MEMBER_ARRAY(hits, Int),
    SK_MEMBER_ARRAY(targets, Displayable),
    SK_MEMBER(value, Boolean)
};

#endif

DEFINE_GET_MEMBER(SkHitTest);

SkHitTest::SkHitTest() : value(false) {
}

bool SkHitTest::draw(SkAnimateMaker& maker) {
    hits.setCount(bullets.count());
    value = false;
    int bulletCount = bullets.count();
    int targetCount = targets.count();
    for (int bIndex = 0; bIndex < bulletCount; bIndex++) {
        SkDisplayable* bullet = bullets[bIndex];
        SkRect bBounds;
        bullet->getBounds(&bBounds);
        hits[bIndex] = -1;
        if (bBounds.fLeft == (int16_t)0x8000U)
            continue;
        for (int tIndex = 0; tIndex < targetCount; tIndex++) {
            SkDisplayable* target = targets[tIndex];
            SkRect tBounds;
            target->getBounds(&tBounds);
            if (bBounds.intersect(tBounds)) {
                hits[bIndex] = tIndex;
                value = true;
                break;
            }
        }
    }
    return false;
}

bool SkHitTest::enable(SkAnimateMaker& maker) {
    for (int bIndex = 0; bIndex < bullets.count(); bIndex++) {
        SkDisplayable* bullet = bullets[bIndex];
        bullet->enableBounder();
    }
    for (int tIndex = 0; tIndex < targets.count(); tIndex++) {
        SkDisplayable* target = targets[tIndex];
        target->enableBounder();
    }
    return false;
}

bool SkHitTest::hasEnable() const {
    return true;
}

const SkMemberInfo* SkHitTest::preferredChild(SkDisplayTypes type) {
    if (bullets.count() == 0)
        return getMember("bullets");
    return getMember("targets"); // !!! cwap! need to refer to member through enum like kScope instead
}

