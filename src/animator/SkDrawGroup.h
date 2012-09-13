
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawGroup_DEFINED
#define SkDrawGroup_DEFINED

#include "SkDrawable.h"
#include "SkIntArray.h"
#include "SkMemberInfo.h"

class SkGroup : public SkDrawable { //interface for schema element <g>
public:
    DECLARE_MEMBER_INFO(Group);
    SkGroup();
    virtual ~SkGroup();
    virtual bool add(SkAnimateMaker& , SkDisplayable* child) SK_OVERRIDE;
    virtual bool contains(SkDisplayable* );
    SkGroup* copy();
    SkBool copySet(int index);
    virtual SkDisplayable* deepCopy(SkAnimateMaker* );
    virtual bool doEvent(SkDisplayEvent::Kind , SkEventState* state );
    virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
    virtual void dumpDrawables(SkAnimateMaker* );
    virtual void dumpEvents();
#endif
    int findGroup(SkDrawable* drawable,  SkTDDrawableArray** list,
        SkGroup** parent, SkGroup** found, SkTDDrawableArray** grandList);
    virtual bool enable(SkAnimateMaker& );
    SkTDDrawableArray* getChildren() { return &fChildren; }
    SkGroup* getOriginal() { return fOriginal; }
    virtual bool hasEnable() const;
    virtual void initialize();
    SkBool isACopy() { return fOriginal != NULL; }
    void markCopyClear(int index);
    void markCopySet(int index);
    void markCopySize(int index);
    bool markedForDelete(int index) const { return (fCopies[index >> 5] & 1 << (index & 0x1f)) == 0; }
    void reset();
    bool resolveIDs(SkAnimateMaker& maker, SkDisplayable* original, SkApply* );
    virtual void setSteps(int steps);
#ifdef SK_DEBUG
    virtual void validate();
#endif
protected:
    bool ifCondition(SkAnimateMaker& maker, SkDrawable* drawable,
        SkString& conditionString);
    SkString condition;
    SkString enableCondition;
    SkTDDrawableArray fChildren;
    SkTDDrawableArray* fParentList;
    SkTDIntArray fCopies;
    SkGroup* fOriginal;
private:
    typedef SkDrawable INHERITED;
};

class SkSave: public SkGroup {
    DECLARE_MEMBER_INFO(Save);
    virtual bool draw(SkAnimateMaker& );
private:
    typedef SkGroup INHERITED;
};

#endif // SkDrawGroup_DEFINED
