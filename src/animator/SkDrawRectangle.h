
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
    void dirty() SK_OVERRIDE;
    bool draw(SkAnimateMaker& ) SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    SkDisplayable* getParent() const SK_OVERRIDE;
    bool getProperty(int index, SkScriptValue* value) const SK_OVERRIDE;
    bool setParent(SkDisplayable* parent) SK_OVERRIDE;
    bool setProperty(int index, SkScriptValue& ) SK_OVERRIDE;
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
    bool draw(SkAnimateMaker& ) SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
protected:
    SkScalar rx;
    SkScalar ry;
private:
    typedef SkDrawRect INHERITED;
};

#endif // SkDrawRectangle_DEFINED
