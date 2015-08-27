
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
    bool addChild(SkAnimateMaker& , SkDisplayable* child) override;
    bool contains(SkDisplayable* ) override;
    SkGroup* copy();
    SkBool copySet(int index);
    SkDisplayable* deepCopy(SkAnimateMaker* ) override;
    bool doEvent(SkDisplayEvent::Kind , SkEventState* state ) override;
    bool draw(SkAnimateMaker& ) override;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
    virtual void dumpDrawables(SkAnimateMaker* );
    void dumpEvents() override;
#endif
    int findGroup(SkADrawable* drawable,  SkTDDrawableArray** list,
        SkGroup** parent, SkGroup** found, SkTDDrawableArray** grandList);
    bool enable(SkAnimateMaker& ) override;
    SkTDDrawableArray* getChildren() { return &fChildren; }
    SkGroup* getOriginal() { return fOriginal; }
    bool hasEnable() const override;
    void initialize() override;
    SkBool isACopy() { return fOriginal != nullptr; }
    void markCopyClear(int index);
    void markCopySet(int index);
    void markCopySize(int index);
    bool markedForDelete(int index) const { return (fCopies[index >> 5] & 1 << (index & 0x1f)) == 0; }
    void reset();
    bool resolveIDs(SkAnimateMaker& maker, SkDisplayable* original, SkApply* ) override;
    void setSteps(int steps) override;
#ifdef SK_DEBUG
    void validate() override;
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
    bool draw(SkAnimateMaker& ) override;
private:
    typedef SkGroup INHERITED;
};

#endif // SkDrawGroup_DEFINED
