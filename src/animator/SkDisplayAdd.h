
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayAdd_DEFINED
#define SkDisplayAdd_DEFINED

#include "SkADrawable.h"
#include "SkMemberInfo.h"

class SkAdd : public SkADrawable {
    DECLARE_MEMBER_INFO(Add);
    SkAdd();

    enum Mode {
        kMode_indirect,
        kMode_immediate
    };

    virtual SkDisplayable* deepCopy(SkAnimateMaker* ) SK_OVERRIDE;
    virtual bool draw(SkAnimateMaker& ) SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    virtual bool enable(SkAnimateMaker& ) SK_OVERRIDE;
    virtual bool hasEnable() const SK_OVERRIDE;
    virtual void initialize() SK_OVERRIDE;
    virtual bool isDrawable() const SK_OVERRIDE;
protected:
//  struct _A {
        Mode mode;
        int32_t offset;
        SkADrawable* use;
        SkADrawable* where;  // if NULL, offset becomes index
//  } A;
private:
    typedef SkADrawable INHERITED;
};

class SkClear : public SkDisplayable {
    virtual bool enable(SkAnimateMaker& );
};

class SkMove : public SkAdd {
    DECLARE_MEMBER_INFO(Move);
private:
    typedef SkAdd INHERITED;
};

class SkRemove : public SkAdd {
    DECLARE_MEMBER_INFO(Remove);
    SkRemove();
protected:
    SkBool fDelete;
private:
    friend class SkAdd;
    typedef SkAdd INHERITED;
};

class SkReplace : public SkAdd {
    DECLARE_MEMBER_INFO(Replace);
private:
    typedef SkAdd INHERITED;
};

#endif // SkDisplayAdd_DEFINED
