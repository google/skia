
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawGroup.h"
#include "SkAnimateMaker.h"
#include "SkAnimatorScript.h"
#include "SkCanvas.h"
#include "SkDisplayApply.h"
#include "SkPaint.h"
#ifdef SK_DEBUG
#include "SkDisplayList.h"
#endif

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkGroup::fInfo[] = {
    SK_MEMBER(condition, String),
    SK_MEMBER(enableCondition, String)
};

#endif

DEFINE_GET_MEMBER(SkGroup);

SkGroup::SkGroup() : fParentList(NULL), fOriginal(NULL) {
}

SkGroup::~SkGroup() {
    if (fOriginal)  // has been copied
        return;
    int index = 0;
    int max = fCopies.count() << 5;
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        if (index >= max || markedForDelete(index))
            delete *ptr;
//      else {
//          SkApply* apply = (SkApply*) *ptr;
//          SkASSERT(apply->isApply());
//          SkASSERT(apply->getScope());
//          delete apply->getScope();
//      }
        index++;
    }
}

bool SkGroup::add(SkAnimateMaker& , SkDisplayable* child) {
    SkASSERT(child); 
//  SkASSERT(child->isDrawable());
    *fChildren.append() = (SkDrawable*) child;
    if (child->isGroup()) {
        SkGroup* groupie = (SkGroup*) child;
        SkASSERT(groupie->fParentList == NULL);
        groupie->fParentList = &fChildren;
    }
    return true;
}

bool SkGroup::contains(SkDisplayable* match) {
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDrawable* drawable = *ptr;
        if (drawable == match || drawable->contains(match))
            return true;
    }
    return false;
}

SkGroup* SkGroup::copy() {
    SkGroup* result = new SkGroup();
    result->fOriginal = this;
    result->fChildren = fChildren;
    return result;
}

SkBool SkGroup::copySet(int index) {
    return (fCopies[index >> 5] & 1 << (index & 0x1f)) != 0;
}

SkDisplayable* SkGroup::deepCopy(SkAnimateMaker* maker) {
    SkDisplayable* copy = INHERITED::deepCopy(maker);
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDisplayable* displayable = (SkDisplayable*)*ptr;
        SkDisplayable* deeperCopy = displayable->deepCopy(maker);
        ((SkGroup*)copy)->add(*maker, deeperCopy);
    }
    return copy;
}

bool SkGroup::doEvent(SkDisplayEvent::Kind kind, SkEventState* state) {
    bool handled = false;
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDrawable* drawable = *ptr;
        if (drawable->isDrawable() == false)
            continue;
        handled |= drawable->doEvent(kind, state);
    }
    return handled;
}

bool SkGroup::draw(SkAnimateMaker& maker) {
    bool conditionTrue = ifCondition(maker, this, condition);
    bool result = false;
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDrawable* drawable = *ptr;
        if (drawable->isDrawable() == false)
            continue;
        if (conditionTrue == false) {
            if (drawable->isApply())
                ((SkApply*) drawable)->disable();
            continue;
        }
        maker.validate();
        result |= drawable->draw(maker);
        maker.validate();
    }
    return result;
}

#ifdef SK_DUMP_ENABLED
void SkGroup::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    if (condition.size() > 0)
        SkDebugf("condition=\"%s\" ", condition.c_str());
    if (enableCondition.size() > 0)
        SkDebugf("enableCondition=\"%s\" ", enableCondition.c_str());
    dumpDrawables(maker);
}

void SkGroup::dumpDrawables(SkAnimateMaker* maker) {
    SkDisplayList::fIndent += 4;
    int save = SkDisplayList::fDumpIndex;
    SkDisplayList::fDumpIndex = 0;
    bool closedYet = false;
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        if (closedYet == false) {
            closedYet = true;
            SkDebugf(">\n");
        }
        SkDrawable* drawable = *ptr;
        drawable->dump(maker);
        SkDisplayList::fDumpIndex++;
    }
    SkDisplayList::fIndent -= 4;
    SkDisplayList::fDumpIndex = save;
    if (closedYet) //we had children, now it's time to close the group
        dumpEnd(maker);
    else    //no children
        SkDebugf("/>\n");
}

void SkGroup::dumpEvents() {
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDrawable* drawable = *ptr;
        drawable->dumpEvents();
    }
}
#endif

bool SkGroup::enable(SkAnimateMaker& maker ) {
    reset();
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDrawable* drawable = *ptr;
        if (ifCondition(maker, drawable, enableCondition) == false)
            continue;
        drawable->enable(maker);
    }
    return true;    // skip add; already added so that scope is findable by children
}

int SkGroup::findGroup(SkDrawable* match,  SkTDDrawableArray** list,
                 SkGroup** parent, SkGroup** found, SkTDDrawableArray** grandList) {
    *list = &fChildren;
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDrawable* drawable = *ptr;
        if (drawable->isGroup()) {
            SkGroup* childGroup = (SkGroup*) drawable;
            if (childGroup->fOriginal == match)
                goto foundMatch;
        }
        if (drawable == match) {
foundMatch:
            *parent = this;
            return (int) (ptr - fChildren.begin());
        }
    }
    *grandList = &fChildren;
    return SkDisplayList::SearchForMatch(match, list, parent, found, grandList);
}

bool SkGroup::hasEnable() const {
    return true;
}

bool SkGroup::ifCondition(SkAnimateMaker& maker, SkDrawable* drawable,
        SkString& conditionString) {
    if (conditionString.size() == 0)
        return true;
    int32_t result;
    bool success = SkAnimatorScript::EvaluateInt(maker, this, conditionString.c_str(), &result);
#ifdef SK_DUMP_ENABLED
    if (maker.fDumpGConditions) {
        SkDebugf("group: ");
        dumpBase(&maker);
        SkDebugf("condition=%s ", conditionString.c_str());
        if (success == false)
            SkDebugf("(script failed)\n");
        else
            SkDebugf("success=%s\n", result != 0 ? "true" : "false");
    }
#endif
    return success && result != 0;
}

void SkGroup::initialize() {
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDrawable* drawable = *ptr;
        if (drawable->isDrawable() == false)
            continue;
        drawable->initialize();
    }
}

void SkGroup::markCopyClear(int index) {
    if (index < 0)
        index = fChildren.count();
    fCopies[index >> 5] &= ~(1 << (index & 0x1f));
}

void SkGroup::markCopySet(int index) {
    if (index < 0)
        index = fChildren.count();
    fCopies[index >> 5] |= 1 << (index & 0x1f);
}

void SkGroup::markCopySize(int index) {
    if (index < 0)
        index = fChildren.count() + 1;
    int oldLongs = fCopies.count();
    int newLongs = (index >> 5) + 1;
    if (oldLongs < newLongs) {
        fCopies.setCount(newLongs);
        memset(&fCopies[oldLongs], 0, (newLongs - oldLongs) << 2);
    }
}

void SkGroup::reset() {
    if (fOriginal)  // has been copied
        return;
    int index = 0;
    int max = fCopies.count() << 5;
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        if (index >= max || copySet(index) == false)
            continue;
        SkApply* apply = (SkApply*) *ptr;
        SkASSERT(apply->isApply());
        SkASSERT(apply->getScope());
        *ptr = apply->getScope();
        markCopyClear(index);
        index++;
    }
}

bool SkGroup::resolveIDs(SkAnimateMaker& maker, SkDisplayable* orig, SkApply* apply) {
    SkGroup* original = (SkGroup*) orig;
    SkTDDrawableArray& originalChildren = original->fChildren;
    SkDrawable** originalPtr = originalChildren.begin();
    SkDrawable** ptr = fChildren.begin();
    SkDrawable** end = fChildren.end();
    SkDrawable** origChild = ((SkGroup*) orig)->fChildren.begin();
    while (ptr < end) {
        SkDrawable* drawable = *ptr++;
        maker.resolveID(drawable, *origChild++);
        if (drawable->resolveIDs(maker, *originalPtr++, apply) == true)
            return true; // failed
    }
    return false;
}

void SkGroup::setSteps(int steps) {
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDrawable* drawable = *ptr;
        if (drawable->isDrawable() == false)
            continue;
        drawable->setSteps(steps);
    }
}

#ifdef SK_DEBUG
void SkGroup::validate() {
    for (SkDrawable** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkDrawable* drawable = *ptr;
        drawable->validate();
    }
}
#endif

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkSave::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkSave);

bool SkSave::draw(SkAnimateMaker& maker) {
    maker.fCanvas->save();
    SkPaint* save = maker.fPaint;
    SkPaint local = SkPaint(*maker.fPaint);
    maker.fPaint = &local;
    bool result = INHERITED::draw(maker);
    maker.fPaint = save;
    maker.fCanvas->restore();
    return result;
}


