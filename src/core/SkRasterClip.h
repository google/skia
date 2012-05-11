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

    bool isEmpty() const {
        SkASSERT(this->computeIsEmpty() == fIsEmpty);
        return fIsEmpty;
    }

    bool isRect() const {
        SkASSERT(this->computeIsRect() == fIsRect);
        return fIsRect;
    }

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

    bool quickContains(const SkIRect& rect) const;
    bool quickContains(int left, int top, int right, int bottom) const {
        return quickContains(SkIRect::MakeLTRB(left, top, right, bottom));
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

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

private:
    SkRegion    fBW;
    SkAAClip    fAA;
    bool        fIsBW;
    // these 2 are caches based on querying the right obj based on fIsBW
    bool        fIsEmpty;
    bool        fIsRect;

    bool computeIsEmpty() const {
        return fIsBW ? fBW.isEmpty() : fAA.isEmpty();
    }

    bool computeIsRect() const {
        return fIsBW ? fBW.isRect() : false;
    }
    
    bool updateCacheAndReturnNonEmpty() {
        fIsEmpty = this->computeIsEmpty();
        fIsRect = this->computeIsRect();
        return !fIsEmpty;
    }

    void convertToAA();
};

class SkAutoRasterClipValidate : SkNoncopyable {
public:
    SkAutoRasterClipValidate(const SkRasterClip& rc) : fRC(rc) {
        fRC.validate();
    }
    ~SkAutoRasterClipValidate() {
        fRC.validate();
    }
private:
    const SkRasterClip& fRC;
};

#ifdef SK_DEBUG
    #define AUTO_RASTERCLIP_VALIDATE(rc)    SkAutoRasterClipValidate arcv(rc)
#else
    #define AUTO_RASTERCLIP_VALIDATE(rc)
#endif

///////////////////////////////////////////////////////////////////////////////

/**
 *  Encapsulates the logic of deciding if we need to change/wrap the blitter
 *  for aaclipping. If so, getRgn and getBlitter return modified values. If
 *  not, they return the raw blitter and (bw) clip region.
 *
 *  We need to keep the constructor/destructor cost as small as possible, so we
 *  can freely put this guy on the stack, and not pay too much for the case when
 *  we're really BW anyways.
 */
class SkAAClipBlitterWrapper {
public:
    SkAAClipBlitterWrapper();
    SkAAClipBlitterWrapper(const SkRasterClip&, SkBlitter*);
    SkAAClipBlitterWrapper(const SkAAClip*, SkBlitter*);
    
    void init(const SkRasterClip&, SkBlitter*);

    const SkIRect& getBounds() const {
        SkASSERT(fClipRgn);
        return fClipRgn->getBounds();
    }
    const SkRegion& getRgn() const {
        SkASSERT(fClipRgn);
        return *fClipRgn;
    }
    SkBlitter* getBlitter() {
        SkASSERT(fBlitter);
        return fBlitter;
    }
    
private:
    const SkAAClip* fAAClip;
    SkRegion        fBWRgn;
    SkAAClipBlitter fAABlitter;
    // what we return
    const SkRegion* fClipRgn;
    SkBlitter* fBlitter;
};

#endif
