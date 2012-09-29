
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBGViewArtist_DEFINED
#define SkBGViewArtist_DEFINED

#include "SkView.h"
#include "SkPaint.h"

class SkBGViewArtist : public SkView::Artist {
public:
            SkBGViewArtist(SkColor c = SK_ColorWHITE);
    virtual ~SkBGViewArtist();

    const SkPaint&  paint() const { return fPaint; }
    SkPaint&        paint() { return fPaint; }

protected:
    // overrides
    virtual void onDraw(SkView*, SkCanvas*);
    virtual void onInflate(const SkDOM&, const SkDOM::Node*);

private:
    SkPaint fPaint;
};

#endif

