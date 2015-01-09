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
    bool add() SK_OVERRIDE;
    void dirty() SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    SkColor getColor();
    SkDisplayable* deepCopy(SkAnimateMaker* ) SK_OVERRIDE;
    SkDisplayable* getParent() const SK_OVERRIDE;
    bool getProperty(int index, SkScriptValue* value) const SK_OVERRIDE;
    void onEndElement(SkAnimateMaker& ) SK_OVERRIDE;
    bool setParent(SkDisplayable* parent) SK_OVERRIDE;
    bool setProperty(int index, SkScriptValue&) SK_OVERRIDE;
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
