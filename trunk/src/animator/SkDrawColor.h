
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawColor_DEFINED
#define SkDrawColor_DEFINED

#include "SkPaintParts.h"
#include "SkColor.h"

class SkDrawColor : public SkPaintPart {
    DECLARE_DRAW_MEMBER_INFO(Color);
    SkDrawColor();
    virtual bool add();
    virtual void dirty();
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif
    SkColor getColor();
    virtual SkDisplayable* deepCopy(SkAnimateMaker* );
    virtual SkDisplayable* getParent() const;
    virtual bool getProperty(int index, SkScriptValue* value) const;
    virtual void onEndElement(SkAnimateMaker& );
    virtual bool setParent(SkDisplayable* parent);
    virtual bool setProperty(int index, SkScriptValue&);
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
