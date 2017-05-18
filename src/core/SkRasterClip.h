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

class SkRRect;

class SkConservativeClip {
    SkIRect         fBounds;
    const SkIRect*  fClipRestrictionRect;

    inline void applyClipRestriction(SkRegion::Op op, SkIRect* bounds) {
        if (op >= SkRegion::kUnion_Op && fClipRestrictionRect
            && !fClipRestrictionRect->isEmpty()) {
            if (!bounds->intersect(*fClipRestrictionRect)) {
                bounds->setEmpty();
            }
        }
    }

public:
    SkConservativeClip() : fBounds(SkIRect::MakeEmpty()), fClipRestrictionRect(nullptr) {}

    bool isEmpty() const { return fBounds.isEmpty(); }
    bool isRect() const { return true; }
    const SkIRect& getBounds() const { return fBounds; }

    void setEmpty() { fBounds.setEmpty(); }
    void setRect(const SkIRect& r) { fBounds = r; }
    void setDeviceClipRestriction(const SkIRect* rect) {
        fClipRestrictionRect = rect;
    }

    void op(const SkRect&, const SkMatrix&, const SkIRect& limit, SkRegion::Op, bool isAA);
    void op(const SkRRect&, const SkMatrix&, const SkIRect& limit, SkRegion::Op, bool isAA);
    void op(const SkPath&, const SkMatrix&, const SkIRect& limit, SkRegion::Op, bool isAA);
    void op(const SkRegion&, SkRegion::Op);
    void op(const SkIRect&, SkRegion::Op);
};

/**
 *  Wraps a SkRegion and SkAAClip, so we have a single object that can represent either our
 *  BW or antialiased clips.
 *
 *  This class is optimized for the raster backend of canvas, but can be expense to keep up2date,
 *  so it supports a runtime option (force-conservative-rects) to turn it into a super-fast
 *  rect-only tracker. The gpu backend uses this since it does not need the result (it uses
 *  SkClipStack instead).
 */
class SkRasterClip {
public:
    SkRasterClip();
    SkRasterClip(const SkIRect&);
    SkRasterClip(const SkRegion&);
    SkRasterClip(const SkRasterClip&);
    ~SkRasterClip();

    // Only compares the current state. Does not compare isForceConservativeRects(), so that field
    // could be different but this could still return true.
    bool operator==(const SkRasterClip&) const;
    bool operator!=(const SkRasterClip& other) const {
        return !(*this == other);
    }

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

    bool op(const SkIRect&, SkRegion::Op);
    bool op(const SkRegion&, SkRegion::Op);
    bool op(const SkRect&, const SkMatrix& matrix, const SkIRect&, SkRegion::Op, bool doAA);
    bool op(const SkRRect&, const SkMatrix& matrix, const SkIRect&, SkRegion::Op, bool doAA);
    bool op(const SkPath&, const SkMatrix& matrix, const SkIRect&, SkRegion::Op, bool doAA);

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
        return !SkIRect::Intersects(this->getBounds(), rect);
    }

    // hack for SkCanvas::getTotalClip
    const SkRegion& forceGetBW();

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    void setDeviceClipRestriction(const SkIRect* rect) {
        fClipRestrictionRect = rect;
    }

private:
    SkRegion    fBW;
    SkAAClip    fAA;
    bool        fIsBW;
    // these 2 are caches based on querying the right obj based on fIsBW
    bool        fIsEmpty;
    bool        fIsRect;
    const SkIRect*    fClipRestrictionRect = nullptr;

    bool computeIsEmpty() const {
        return fIsBW ? fBW.isEmpty() : fAA.isEmpty();
    }

    bool computeIsRect() const {
        return fIsBW ? fBW.isRect() : fAA.isRect();
    }

    bool updateCacheAndReturnNonEmpty(bool detectAARect = true) {
        fIsEmpty = this->computeIsEmpty();

        // detect that our computed AA is really just a (hard-edged) rect
        if (detectAARect && !fIsEmpty && !fIsBW && fAA.isRect()) {
            fBW.setRect(fAA.getBounds());
            fAA.setEmpty(); // don't need this guy anymore
            fIsBW = true;
        }

        fIsRect = this->computeIsRect();
        return !fIsEmpty;
    }

    void convertToAA();

    bool setPath(const SkPath& path, const SkRegion& clip, bool doAA);
    bool setPath(const SkPath& path, const SkIRect& clip, bool doAA);
    bool op(const SkRasterClip&, SkRegion::Op);
    bool setConservativeRect(const SkRect& r, const SkIRect& clipR, bool isInverse);

    inline void applyClipRestriction(SkRegion::Op op, SkIRect* bounds) {
        if (op >= SkRegion::kUnion_Op && fClipRestrictionRect
            && !fClipRestrictionRect->isEmpty()) {
            if (!bounds->intersect(*fClipRestrictionRect)) {
                bounds->setEmpty();
            }
        }
    }

    inline void applyClipRestriction(SkRegion::Op op, SkRect* bounds) {
        if (op >= SkRegion::kUnion_Op && fClipRestrictionRect
            && !fClipRestrictionRect->isEmpty()) {
            if (!bounds->intersect(SkRect::Make(*fClipRestrictionRect))) {
                bounds->setEmpty();
            }
        }
    }
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
#define SkAutoRasterClipValidate(...) SK_REQUIRE_LOCAL_VAR(SkAutoRasterClipValidate)

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
    SkRegion        fBWRgn;
    SkAAClipBlitter fAABlitter;
    // what we return
    const SkRegion* fClipRgn;
    SkBlitter* fBlitter;
};

#endif
