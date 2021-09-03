/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterClip_DEFINED
#define SkRasterClip_DEFINED

#include "include/core/SkClipOp.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/private/SkMacros.h"
#include "src/core/SkAAClip.h"

class SkRRect;

/**
 *  Wraps a SkRegion and SkAAClip, so we have a single object that can represent either our
 *  BW or antialiased clips.
 */
class SkRasterClip {
public:
    SkRasterClip();
    explicit SkRasterClip(const SkIRect&);
    explicit SkRasterClip(const SkRegion&);
    explicit SkRasterClip(const SkRasterClip&);
    SkRasterClip(const SkPath& path, const SkIRect& bounds, bool doAA);

    ~SkRasterClip();

    SkRasterClip& operator=(const SkRasterClip&);

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

    bool isComplex() const {
        return fIsBW ? fBW.isComplex() : !fAA.isEmpty();
    }
    const SkIRect& getBounds() const {
        return fIsBW ? fBW.getBounds() : fAA.getBounds();
    }

    bool setEmpty();
    bool setRect(const SkIRect&);

    bool op(const SkIRect&, SkClipOp);
    bool op(const SkRegion&, SkClipOp);
    bool op(const SkRect&, const SkMatrix& matrix, SkClipOp, bool doAA);
    bool op(const SkRRect&, const SkMatrix& matrix, SkClipOp, bool doAA);
    bool op(const SkPath&, const SkMatrix& matrix, SkClipOp, bool doAA);
    bool op(sk_sp<SkShader>);

    void translate(int dx, int dy, SkRasterClip* dst) const;

    bool quickContains(const SkIRect& rect) const {
        return fIsBW ? fBW.quickContains(rect) : fAA.quickContains(rect);
    }

    /**
     *  Return true if this region is empty, or if the specified rectangle does
     *  not intersect the region. Returning false is not a guarantee that they
     *  intersect, but returning true is a guarantee that they do not.
     */
    bool quickReject(const SkIRect& rect) const {
        return !SkIRect::Intersects(this->getBounds(), rect);
    }

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    sk_sp<SkShader> clipShader() const { return fShader; }

private:
    SkRegion    fBW;
    SkAAClip    fAA;
    bool        fIsBW;
    // these 2 are caches based on querying the right obj based on fIsBW
    bool        fIsEmpty;
    bool        fIsRect;
    // if present, this augments the clip, not replaces it
    sk_sp<SkShader> fShader;

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
            fAA.setEmpty(); // don't need this anymore
            fIsBW = true;
        }

        fIsRect = this->computeIsRect();
        return !fIsEmpty;
    }

    void convertToAA();

    bool op(const SkRasterClip&, SkClipOp);
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
 *  can freely put this on the stack, and not pay too much for the case when
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
