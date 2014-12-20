
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawGroup_DEFINED
#define SkDrawGroup_DEFINED

#include "SkADrawable.h"
#include "SkIntArray.h"
#include "SkMemberInfo.h"

class SkGroup : public SkADrawable { //interface for schema element <g>
public:
    DECLARE_MEMBER_INFO(Group);
    SkGroup();
    virtual ~SkGroup();
    virtual bool addChild(SkAnimateMaker& , SkDisplayable* child) SK_OVERRIDE;
    virtual bool contains(SkDisplayable* ) SK_OVERRIDE;
    SkGroup* copy();
    SkBool copySet(int index);
    virtual SkDisplayable* deepCopy(SkAnimateMaker* ) SK_OVERRIDE;
    virtual bool doEvent(SkDisplayEvent::Kind , SkEventState* state ) SK_OVERRIDE;
    virtual bool draw(SkAnimateMaker& ) SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* ) SK_OVERRIDE;
    virtual void dumpDrawables(SkAnimateMaker* );
    virtual void dumpEvents() SK_OVERRIDE;
#endif
    int findGroup(SkADrawable* drawable,  SkTDDrawableArray** list,
        SkGroup** parent, SkGroup** found, SkTDDrawableArray** grandList);
    virtual bool enable(SkAnimateMaker& ) SK_OVERRIDE;
    SkTDDrawableArray* getChildren() { return &fChildren; }
    SkGroup* getOriginal() { return fOriginal; }
    virtual bool hasEnable() const SK_OVERRIDE;
    virtual void initialize() SK_OVERRIDE;
    SkBool isACopy() { return fOriginal != NULL; }
    void markCopyClear(int index);
    void markCopySet(int index);
    void markCopySize(int index);
    bool markedForDelete(int index) const { return (fCopies[index >> 5] & 1 << (index & 0x1f)) == 0; }
    void reset();
    bool resolveIDs(SkAnimateMaker& maker, SkDisplayable* original, SkApply* ) SK_OVERRIDE;
    virtual void setSteps(int steps) SK_OVERRIDE;
#ifdef SK_DEBUG
    virtual void validate() SK_OVERRIDE;
#endif
protected:
    bool ifCondition(SkAnimateMaker& maker, SkADrawable* drawable,
        SkString& conditionString);
    SkString condition;
    SkString enableCondition;
    SkTDDrawableArray fChildren;
    SkTDDrawableArray* fParentList;
    SkTDIntArray fCopies;
    SkGroup* fOriginal;
private:
    typedef SkADrawable INHERITED;
};

class SkSave: public SkGroup {
    DECLARE_MEMBER_INFO(Save);
    virtual bool draw(SkAnimateMaker& ) SK_OVERRIDE;
private:
    typedef SkGroup INHERITED;
};

#endif // SkDrawGroup_DEFINED
