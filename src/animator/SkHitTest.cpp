
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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

bool SkHitTest::draw(SkAnimateMaker&) {
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

bool SkHitTest::enable(SkAnimateMaker&) {
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

const SkMemberInfo* SkHitTest::preferredChild(SkDisplayTypes) {
    if (bullets.count() == 0)
        return getMember("bullets");
    return getMember("targets"); // !!! cwap! need to refer to member through enum like kScope instead
}
