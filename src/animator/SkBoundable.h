
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBoundable_DEFINED
#define SkBoundable_DEFINED

#include "SkADrawable.h"
#include "SkRect.h"

class SkBoundable : public SkADrawable {
public:
    SkBoundable();
    virtual void clearBounder();
    virtual void enableBounder();
    virtual void getBounds(SkRect* );
    bool hasBounds() { return fBounds.fLeft != (int16_t)0x8000U; }
    void setBounds(SkIRect& bounds) { fBounds = bounds; }
protected:
    void clearBounds() { fBounds.fLeft = (int16_t) SkToU16(0x8000); }; // mark bounds as unset
    SkIRect fBounds;
private:
    typedef SkADrawable INHERITED;
};

class SkBoundableAuto {
public:
    SkBoundableAuto(SkBoundable* boundable, SkAnimateMaker& maker);
    ~SkBoundableAuto();
private:
    SkBoundable* fBoundable;
    SkAnimateMaker& fMaker;
    SkBoundableAuto& operator= (const SkBoundableAuto& );
};

#endif // SkBoundable_DEFINED
