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

    bool isEmpty() const { return nullptr == fRunHead; }
    const SkIRect& getBounds() const { return fBounds; }

    // Returns true iff the clip is not empty, and is just a hard-edged rect (no partial alpha).
    // If true, getBounds() can be used in place of this clip.
    bool isRect() const;

    bool setEmpty();
    bool setRect(const SkIRect&);
    bool setRect(const SkRect&, bool doAA = true);
    bool setPath(const SkPath&, const SkRegion* clip = nullptr, bool doAA = true);
    bool setRegion(const SkRegion&);
    bool set(const SkAAClip&);

    bool op(const SkAAClip&, const SkAAClip&, SkRegion::Op);

    // Helpers for op()
    bool op(const SkIRect&, SkRegion::Op);
    bool op(const SkRect&, SkRegion::Op, bool doAA);
    bool op(const SkAAClip&, SkRegion::Op);

    bool translate(int dx, int dy, SkAAClip* dst) const;
    bool translate(int dx, int dy) {
        return this->translate(dx, dy, this);
    }

    /**
     *  Allocates a mask the size of the aaclip, and expands its data into
     *  the mask, using kA8_Format
     */
    void copyToMask(SkMask*) const;

    // called internally

    bool quickContains(int left, int top, int right, int bottom) const;
    bool quickContains(const SkIRect& r) const {
        return this->quickContains(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

    const uint8_t* findRow(int y, int* lastYForRow = nullptr) const;
    const uint8_t* findX(const uint8_t data[], int x, int* initialCount = nullptr) const;

    class Iter;
    struct RunHead;
    struct YOffset;
    class Builder;

#ifdef SK_DEBUG
    void validate() const;
    void debug(bool compress_y=false) const;
#else
    void validate() const {}
    void debug(bool compress_y=false) const {}
#endif

private:
    SkIRect  fBounds;
    RunHead* fRunHead;

    void freeRuns();
    bool trimBounds();
    bool trimTopBottom();
    bool trimLeftRight();

    friend class Builder;
    class BuilderBlitter;
    friend class BuilderBlitter;
};

///////////////////////////////////////////////////////////////////////////////

class SkAAClipBlitter : public SkBlitter {
public:
    SkAAClipBlitter() : fScanlineScratch(nullptr) {}
    virtual ~SkAAClipBlitter();

    void init(SkBlitter* blitter, const SkAAClip* aaclip) {
        SkASSERT(aaclip && !aaclip->isEmpty());
        fBlitter = blitter;
        fAAClip = aaclip;
        fAAClipBounds = aaclip->getBounds();
    }

    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitMask(const SkMask&, const SkIRect& clip) override;
    const SkPixmap* justAnOpaqueColor(uint32_t* value) override;

private:
    SkBlitter*      fBlitter;
    const SkAAClip* fAAClip;
    SkIRect         fAAClipBounds;

    // point into fScanlineScratch
    int16_t*        fRuns;
    SkAlpha*        fAA;

    enum {
        kSize = 32 * 32
    };
    SkAutoSMalloc<kSize> fGrayMaskScratch;  // used for blitMask
    void* fScanlineScratch;  // enough for a mask at 32bit, or runs+aa

    void ensureRunsAndAA();
};

#endif
