
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawRectangle_DEFINED
#define SkDrawRectangle_DEFINED

#include "SkBoundable.h"
#include "SkMemberInfo.h"
#include "SkRect.h"

class SkRectToRect;

class SkDrawRect : public SkBoundable {
    DECLARE_DRAW_MEMBER_INFO(Rect);
    SkDrawRect();
    virtual void dirty();
    virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif
    virtual SkDisplayable* getParent() const;
    virtual bool getProperty(int index, SkScriptValue* value) const;
    virtual bool setParent(SkDisplayable* parent);
    virtual bool setProperty(int index, SkScriptValue& );
protected:
    SkRect fRect;
    SkDisplayable* fParent;
private:
    friend class SkDrawClip;
    friend class SkRectToRect;
    friend class SkSaveLayer;
    typedef SkBoundable INHERITED;
};

class SkRoundRect : public SkDrawRect {
    DECLARE_MEMBER_INFO(RoundRect);
    SkRoundRect();
    virtual bool draw(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif    
protected:
    SkScalar rx;
    SkScalar ry;
private:
    typedef SkDrawRect INHERITED;
};

#endif // SkDrawRectangle_DEFINED

