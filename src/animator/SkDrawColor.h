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
    bool add() override;
    void dirty() override;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
    SkColor getColor();
    SkDisplayable* deepCopy(SkAnimateMaker* ) override;
    SkDisplayable* getParent() const override;
    bool getProperty(int index, SkScriptValue* value) const override;
    void onEndElement(SkAnimateMaker& ) override;
    bool setParent(SkDisplayable* parent) override;
    bool setProperty(int index, SkScriptValue&) override;
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
