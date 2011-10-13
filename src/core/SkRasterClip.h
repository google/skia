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
    const SkRegion& bwRgn() const { SkASSERT(fIsBW); return fBW; }
    const SkAAClip& aaRgn() const { SkASSERT(!fIsBW); return fAA; }

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

    void translate(int dx, int dy, SkRasterClip* dst) const;
    void translate(int dx, int dy) {
        this->translate(dx, dy, this);
    }

    /**
     *  Return true if this region is empty, or if the specified rectangle does
     *  not intersect the region. Returning false is not a guarantee that they
     *  intersect, but returning true is a guarantee that they do not.
     */
    bool quickReject(const SkIRect& rect) const {
        return this->isEmpty() || rect.isEmpty() ||
               !SkIRect::Intersects(this->getBounds(), rect);
    }
    
    // hack for SkCanvas::getTotalClip
    const SkRegion& forceGetBW();

private:
    SkRegion    fBW;
    SkAAClip    fAA;
    bool        fIsBW;

    void convertToAA();
};

#endif
