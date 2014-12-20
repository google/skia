/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawColor_DEFINED
#define SkDrawColor_DEFINED

#include "SkPaintPart.h"
#include "SkColor.h"

class SkDrawColor : public SkPaintPart {
    DECLARE_DRAW_MEMBER_INFO(Color);
    SkDrawColor();
    virtual bool add() SK_OVERRIDE;
    virtual void dirty() SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    SkColor getColor();
    virtual SkDisplayable* deepCopy(SkAnimateMaker* ) SK_OVERRIDE;
    virtual SkDisplayable* getParent() const SK_OVERRIDE;
    virtual bool getProperty(int index, SkScriptValue* value) const SK_OVERRIDE;
    virtual void onEndElement(SkAnimateMaker& ) SK_OVERRIDE;
    virtual bool setParent(SkDisplayable* parent) SK_OVERRIDE;
    virtual bool setProperty(int index, SkScriptValue&) SK_OVERRIDE;
protected:
    SkColor color;
    SkScalar fHue;
    SkScalar fSaturation;
    SkScalar fValue;
    SkBool fDirty;
private:
    friend class SkDrawGradient;
    typedef SkPaintPart INHERITED;
};

#endif // SkDrawColor_DEFINED
