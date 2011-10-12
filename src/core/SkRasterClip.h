/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterClip_DEFINED
#define SkRasterClip_DEFINED

#include "SkRegion.h"
#include "SkAAClip.h"

class SkRasterClip {
public:
    SkRasterClip();
    SkRasterClip(const SkIRect&);
    SkRasterClip(const SkRasterClip&);
    ~SkRasterClip();

    bool isBW() const { return fIsBW; }
    bool isAA() const { return !fIsBW; }

    bool isEmpty() const;
    bool isRect() const;
    bool isComplex() const;
    const SkIRect& getBounds() const;

    bool setEmpty();
    bool setRect(const SkIRect&);

    bool setPath(const SkPath& path, const SkRegion& clip, bool doAA);
    bool setPath(const SkPath& path, const SkIRect& clip, bool doAA);
    bool setPath(const SkPath& path, const SkRasterClip&, bool doAA);

    bool op(const SkIRect&, SkRegion::Op);
    bool op(const SkRegion&, SkRegion::Op);
    bool op(const SkRasterClip&, SkRegion::Op);
    bool op(const SkRect&, SkRegion::Op, bool doAA);

    const SkRegion& bwRgn() const { SkASSERT(fIsBW); return fBW; }
    const SkAAClip& aaRgn() const { SkASSERT(!fIsBW); return fAA; }

    const SkRegion& forceGetBW();

private:
    SkRegion    fBW;
    SkAAClip    fAA;
    bool        fIsBW;

    void convertToAA();
};

#endif
