
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayList.h"
#include "SkAnimateActive.h"
#include "SkAnimateBase.h"
#include "SkAnimateMaker.h"
#include "SkDisplayApply.h"
#include "SkADrawable.h"
#include "SkDrawGroup.h"
#include "SkDrawMatrix.h"
#include "SkInterpolator.h"
#include "SkTime.h"

SkDisplayList::SkDisplayList() : fDrawBounds(true), fUnionBounds(false), fInTime(0) {
}

SkDisplayList::~SkDisplayList() {
}

void SkDisplayList::append(SkActive* active) {
    *fActiveList.append() = active;
}

bool SkDisplayList::draw(SkAnimateMaker& maker, SkMSec inTime) {
    validate();
    fInTime = inTime;
    bool result = false;
    fInvalBounds.setEmpty();
    if (fDrawList.count()) {
        for (SkActive** activePtr = fActiveList.begin(); activePtr < fActiveList.end(); activePtr++) {
            SkActive* active = *activePtr;
            active->reset();
        }
        for (int index = 0; index < fDrawList.count(); index++) {
            SkADrawable* draw = fDrawList[index];
            draw->initialize(); // allow matrices to reset themselves
            SkASSERT(draw->isDrawable());
            validate();
            result |= draw->draw(maker);
        }
    }
    validate();
    return result;
}

int SkDisplayList::findGroup(SkADrawable* match, SkTDDrawableArray** list,
        SkGroup** parent, SkGroup** found, SkTDDrawableArray**grandList) {
    *parent = NULL;
    *list = &fDrawList;
    *grandList = &fDrawList;
    return SearchForMatch(match, list, parent, found, grandList);
}

void SkDisplayList::hardReset() {
    fDrawList.reset();
    fActiveList.reset();
}

bool SkDisplayList::onIRect(const SkIRect& r) {
    fBounds = r;
    return fDrawBounds;
}

int SkDisplayList::SearchForMatch(SkADrawable* match, SkTDDrawableArray** list,
        SkGroup** parent, SkGroup** found, SkTDDrawableArray**grandList) {
    *found = NULL;
    for (int index = 0; index < (*list)->count(); index++) {
        SkADrawable* draw = (**list)[index];
        if (draw == match)
            return index;
        if (draw->isApply()) {
            SkApply* apply = (SkApply*) draw;
            if (apply->scope == match)
                return index;
            if (apply->scope->isGroup() && SearchGroupForMatch(apply->scope, match, list, parent, found, grandList, index))
                return index;
            if (apply->mode == SkApply::kMode_create) {
                for (SkADrawable** ptr = apply->fScopes.begin(); ptr < apply->fScopes.end(); ptr++) {
                    SkADrawable* scope = *ptr;
                    if (scope == match)
                        return index;
                    //perhaps should call SearchGroupForMatch here as well (on scope)
                }
            }
        }
        if (draw->isGroup() && SearchGroupForMatch(draw, match, list, parent, found, grandList, index))
            return index;

    }
    return -1;
}

bool SkDisplayList::SearchGroupForMatch(SkADrawable* draw, SkADrawable* match, SkTDDrawableArray** list,
        SkGroup** parent, SkGroup** found, SkTDDrawableArray** grandList, int &index) {
            SkGroup* group = (SkGroup*) draw;
            if (group->getOriginal() == match)
                return true;
            SkTDDrawableArray* saveList = *list;
            int groupIndex = group->findGroup(match, list, parent, found, grandList);
            if (groupIndex >= 0) {
                *found = group;
                index = groupIndex;
                return true;
            }
            *list = saveList;
            return false;
        }

void SkDisplayList::reset() {
    for (int index = 0; index < fDrawList.count(); index++) {
        SkADrawable* draw = fDrawList[index];
        if (draw->isApply() == false)
            continue;
        SkApply* apply = (SkApply*) draw;
        apply->reset();
    }
}

void SkDisplayList::remove(SkActive* active) {
    int index = fActiveList.find(active);
    SkASSERT(index >= 0);
    fActiveList.remove(index);  // !!! could use shuffle instead
    SkASSERT(fActiveList.find(active) < 0);
}

#ifdef SK_DUMP_ENABLED
int SkDisplayList::fDumpIndex;
int SkDisplayList::fIndent;

void SkDisplayList::dump(SkAnimateMaker* maker) {
    fIndent = 0;
    dumpInner(maker);
}

void SkDisplayList::dumpInner(SkAnimateMaker* maker) {
    for (int index = 0; index < fDrawList.count(); index++) {
        fDumpIndex = index;
        fDrawList[fDumpIndex]->dump(maker);
    }
}

#endif

#ifdef SK_DEBUG
void SkDisplayList::validate() {
    for (int index = 0; index < fDrawList.count(); index++) {
        SkADrawable* draw = fDrawList[index];
        draw->validate();
    }
}
#endif
