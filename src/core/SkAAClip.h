/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAAClip_DEFINED
#define SkAAClip_DEFINED

#include "include/core/SkClipOp.h"
#include "include/core/SkRect.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkBlitter.h"

class SkPath;
class SkRegion;

class SkAAClip {
public:
    SkAAClip();
    SkAAClip(const SkAAClip&);
    ~SkAAClip();

    SkAAClip& operator=(const SkAAClip&);

    bool isEmpty() const { return nullptr == fRunHead; }
    const SkIRect& getBounds() const { return fBounds; }

    // Returns true iff the clip is not empty, and is just a hard-edged rect (no partial alpha).
    // If true, getBounds() can be used in place of this clip.
    bool isRect() const;

    bool setEmpty();
    bool setRect(const SkIRect&);
    bool setPath(const SkPath&, const SkIRect& bounds, bool doAA = true);
    bool setRegion(const SkRegion&);

    bool op(const SkIRect&, SkClipOp);
    bool op(const SkRect&, SkClipOp, bool doAA);
    bool op(const SkAAClip&, SkClipOp);

    bool translate(int dx, int dy, SkAAClip* dst) const;

    /**
     *  Allocates a mask the size of the aaclip, and expands its data into
     *  the mask, using kA8_Format. Used for tests and visualization purposes.
     */
    void copyToMask(SkMask*) const;

    bool quickContains(const SkIRect& r) const {
        return this->quickContains(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }

#ifdef SK_DEBUG
    void validate() const;
    void debug(bool compress_y=false) const;
#else
    void validate() const {}
    void debug(bool compress_y=false) const {}
#endif

private:
    class Builder;
    struct RunHead;
    friend class SkAAClipBlitter;

    SkIRect  fBounds;
    RunHead* fRunHead;

    void freeRuns();

    bool quickContains(int left, int top, int right, int bottom) const;

    bool trimBounds();
    bool trimTopBottom();
    bool trimLeftRight();

    // For SkAAClipBlitter and quickContains
    const uint8_t* findRow(int y, int* lastYForRow = nullptr) const;
    const uint8_t* findX(const uint8_t data[], int x, int* initialCount = nullptr) const;
};

///////////////////////////////////////////////////////////////////////////////

class SkAAClipBlitter : public SkBlitter {
public:
    SkAAClipBlitter() : fScanlineScratch(nullptr) {}
    ~SkAAClipBlitter() override;

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
