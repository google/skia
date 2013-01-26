
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayAdd.h"
#include "SkAnimateMaker.h"
#include "SkDisplayApply.h"
#include "SkDisplayList.h"
#include "SkDrawable.h"
#include "SkDrawGroup.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkAdd::fInfo[] = {
    SK_MEMBER(mode, AddMode),
    SK_MEMBER(offset, Int),
    SK_MEMBER(use, Drawable),
    SK_MEMBER(where, Drawable)
};

#endif

// start here;
// add onEndElement to turn where string into f_Where
// probably need new SkAnimateMaker::resolve flavor that takes
// where="id", where="event-target" or not-specified
// offset="#" (implements before, after, and index if no 'where')

DEFINE_GET_MEMBER(SkAdd);

SkAdd::SkAdd() : mode(kMode_indirect),
    offset(SK_MaxS32), use(NULL), where(NULL) {
}

SkDisplayable* SkAdd::deepCopy(SkAnimateMaker* maker) {
    SkDrawable* saveUse = use;
    SkDrawable* saveWhere = where;
    use = NULL;
    where = NULL;
    SkAdd* copy = (SkAdd*) INHERITED::deepCopy(maker);
    copy->use = use = saveUse;
    copy->where = where = saveWhere;
    return copy;
}

bool SkAdd::draw(SkAnimateMaker& maker) {
    SkASSERT(use);
    SkASSERT(use->isDrawable());
    if (mode == kMode_indirect)
        use->draw(maker);
    return false;
}

#ifdef SK_DUMP_ENABLED
void SkAdd::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    dumpAttrs(maker);
    if (where)
        SkDebugf("where=\"%s\" ", where->id);
    if (mode == kMode_immediate)
        SkDebugf("mode=\"immediate\" ");
    SkDebugf(">\n");
    SkDisplayList::fIndent += 4;
    int save = SkDisplayList::fDumpIndex;
    if (use)    //just in case
        use->dump(maker);
    SkDisplayList::fIndent -= 4;
    SkDisplayList::fDumpIndex = save;
    dumpEnd(maker);
}
#endif

bool SkAdd::enable(SkAnimateMaker& maker ) {
    SkDisplayTypes type = getType();
    SkDisplayList& displayList = maker.fDisplayList;
    SkTDDrawableArray* parentList = displayList.getDrawList();
    if (type == SkType_Add) {
        if (use == NULL) // not set in apply yet
            return true;
    }
    bool skipAddToParent = true;
    SkASSERT(type != SkType_Replace || where);
    SkTDDrawableArray* grandList SK_INIT_TO_AVOID_WARNING;
    SkGroup* parentGroup = NULL;
    SkGroup* thisGroup = NULL;
    int index = where ? displayList.findGroup(where, &parentList, &parentGroup,
        &thisGroup, &grandList) : 0;
    if (index < 0)
        return true;
    int max = parentList->count();
    if (where == NULL && type == SkType_Move)
        index = max;
    if (offset != SK_MaxS32) {
        index += offset;
        if (index > max) {
            maker.setErrorCode(SkDisplayXMLParserError::kIndexOutOfRange);
            return true;    // caller should not add
        }
    }
    if (offset < 0 && where == NULL)
        index += max + 1;
    switch (type) {
        case SkType_Add:
            if (offset == SK_MaxS32 && where == NULL) {
                if (use->isDrawable()) {
                    skipAddToParent = mode == kMode_immediate;
                    if (skipAddToParent) {
                        if (where == NULL) {
                            SkTDDrawableArray* useParentList;
                            index = displayList.findGroup(this, &useParentList, &parentGroup,
                                &thisGroup, &grandList);
                            if (index >= 0) {
                                parentGroup->markCopySize(index);
                                parentGroup->markCopySet(index);
                                useParentList->begin()[index] = use;
                                break;
                            }
                        }
                        *parentList->append() = use;
                    }
                }
                break;
            } else {
                if (thisGroup)
                    thisGroup->markCopySize(index);
                *parentList->insert(index) = use;
                if (thisGroup)
                    thisGroup->markCopySet(index);
                if (use->isApply())
                    ((SkApply*) use)->setEmbedded();
            }
            break;
        case SkType_Move: {
            int priorLocation = parentList->find(use);
            if (priorLocation < 0)
                break;
            *parentList->insert(index) = use;
            if (index < priorLocation)
                priorLocation++;
            parentList->remove(priorLocation);
            } break;
        case SkType_Remove: {
            SkDisplayable* old = (*parentList)[index];
            if (((SkRemove*)(this))->fDelete) {
                delete old;
                goto noHelperNeeded;
            }
            for (int inner = 0; inner < maker.fChildren.count(); inner++) {
                SkDisplayable* child = maker.fChildren[inner];
                if (child == old || child->contains(old))
                    goto noHelperNeeded;
            }
            if (maker.fHelpers.find(old) < 0)
                maker.helperAdd(old);
noHelperNeeded:
            parentList->remove(index);
            } break;
        case SkType_Replace:
            if (thisGroup) {
                thisGroup->markCopySize(index);
                if (thisGroup->markedForDelete(index)) {
                    SkDisplayable* old = (*parentList)[index];
                    if (maker.fHelpers.find(old) < 0)
                        maker.helperAdd(old);
                }
            }
            (*parentList)[index] = use;
            if (thisGroup)
                thisGroup->markCopySet(index);
            break;
        default:
            SkASSERT(0);
    }
    if (type == SkType_Remove)
        return true;
    if (use->hasEnable())
        use->enable(maker);
    return skipAddToParent; // append if indirect: *parentList->append() = this;
}

bool SkAdd::hasEnable() const {
    return true;
}

void SkAdd::initialize() {
    if (use)
        use->initialize();
}

bool SkAdd::isDrawable() const {
    return getType() == SkType_Add && mode == kMode_indirect && offset == SK_MaxS32 &&
        where == NULL && use != NULL && use->isDrawable();
}

//SkDisplayable* SkAdd::resolveTarget(SkAnimateMaker& maker) {
//  return use;
//}


bool SkClear::enable(SkAnimateMaker& maker ) {
    SkDisplayList& displayList = maker.fDisplayList;
    displayList.clear();
    return true;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkMove::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkMove);

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkRemove::fInfo[] = {
    SK_MEMBER_ALIAS(delete, fDelete, Boolean),  // !!! experimental
    SK_MEMBER(offset, Int),
    SK_MEMBER(where, Drawable)
};

#endif

DEFINE_GET_MEMBER(SkRemove);

SkRemove::SkRemove() : fDelete(false) {
}

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkReplace::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkReplace);
