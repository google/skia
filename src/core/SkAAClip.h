
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAAClip_DEFINED
#define SkAAClip_DEFINED

#include "SkBlitter.h"
#include "SkRegion.h"

#define SkAAClip_gEmptyPtr   ((SkAAClip::RunHead*)-1)
#define SkAAClip_gRectPtr    NULL

class SkAAClip {
public:
    SkAAClip();
    SkAAClip(const SkAAClip&);
    ~SkAAClip();

    SkAAClip& operator=(const SkAAClip&);
    friend bool operator==(const SkAAClip&, const SkAAClip&);
    friend bool operator!=(const SkAAClip& a, const SkAAClip& b) {
        return !(a == b);
    }

    void swap(SkAAClip&);

    bool isEmpty() const { return SkAAClip_gEmptyPtr == fRunHead; }
    bool isRect() const { return SkAAClip_gRectPtr == fRunHead; }
    bool isComplex() const { return !this->isEmpty() && !this->isRect(); }
    const SkIRect& getBounds() const { return fBounds; }

    bool setEmpty();
    bool setRect(const SkIRect&);
    bool setRect(const SkRect&);
    bool setPath(const SkPath&, const SkRegion& clip);

    bool op(const SkAAClip&, const SkAAClip&, SkRegion::Op);

    // called internally
    
    bool quickContains(int left, int top, int right, int bottom) const;
    const uint8_t* findRow(int y, int* lastYForRow) const;
    const uint8_t* findX(const uint8_t data[], int x, int* initialCount) const;

private:
    struct RunHead;
    struct YOffset;

    SkIRect  fBounds;
    RunHead* fRunHead;

    void freeRuns();

    class Builder;
    friend class Builder;
    class BuilderBlitter;
    friend class BuilderBlitter;
};

///////////////////////////////////////////////////////////////////////////////

class SkAAClipBlitter : public SkBlitter {
public:
    SkAAClipBlitter() : fRuns(NULL) {}
    virtual ~SkAAClipBlitter();

    void init(SkBlitter* blitter, const SkAAClip* aaclip) {
        SkASSERT(aaclip && !aaclip->isEmpty());
        fBlitter = blitter;
        fAAClip = aaclip;
        fAAClipBounds = aaclip->getBounds();
    }
    
    virtual void blitH(int x, int y, int width) SK_OVERRIDE;
    virtual void blitAntiH(int x, int y, const SkAlpha[],
                           const int16_t runs[]) SK_OVERRIDE;
    virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE;
    virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE;
    virtual void blitMask(const SkMask&, const SkIRect& clip) SK_OVERRIDE;
    virtual const SkBitmap* justAnOpaqueColor(uint32_t* value) SK_OVERRIDE;
    
private:
    SkBlitter*      fBlitter;
    const SkAAClip* fAAClip;
    SkIRect         fAAClipBounds;

    // lazily allocated
    int16_t*        fRuns;
    SkAlpha*        fAA;    // points into fRuns allocation

    void ensureRunsAndAA();
};

#endif
