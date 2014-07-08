/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePlayback_DEFINED
#define SkPicturePlayback_DEFINED

#include "SkTypes.h"

class SkBitmap;
class SkCanvas;
class SkDrawPictureCallback;
class SkPaint;
class SkPictureData;

class SkPicturePlayback : SkNoncopyable {
public:
    SkPicturePlayback(const SkPicture* picture)
        : fPictureData(picture->fData.get())
        , fCurOffset(0)
        , fUseBBH(true)
        , fStart(0)
        , fStop(0)
        , fReplacements(NULL) {
    }
    virtual ~SkPicturePlayback() { }

    void draw(SkCanvas* canvas, SkDrawPictureCallback*);

    // Return the ID of the operation currently being executed when playing
    // back. 0 indicates no call is active.
    size_t curOpID() const { return fCurOffset; }
    void resetOpID() { fCurOffset = 0; }

    void setUseBBH(bool useBBH) { fUseBBH = useBBH; }

    // Limit the opcode playback to be between the offsets 'start' and 'stop'.
    // The opcode at 'start' should be a saveLayer while the opcode at
    // 'stop' should be a restore. Neither of those commands will be issued.
    // Set both start & stop to 0 to disable draw limiting
    // Draw limiting cannot be enabled at the same time as draw replacing
    void setDrawLimits(size_t start, size_t stop) {
        SkASSERT(NULL == fReplacements);
        fStart = start;
        fStop = stop;
    }

    // PlaybackReplacements collects op ranges that can be replaced with
    // a single drawBitmap call (using a precomputed bitmap).
    class PlaybackReplacements {
    public:
        // All the operations between fStart and fStop (inclusive) will be replaced with
        // a single drawBitmap call using fPos, fBM and fPaint.
        // fPaint will be NULL if the picture's paint wasn't copyable
        struct ReplacementInfo {
            size_t          fStart;
            size_t          fStop;
            SkIPoint        fPos;
            SkBitmap*       fBM;     // fBM is allocated so ReplacementInfo can remain POD
            const SkPaint*  fPaint;  // Note: this object doesn't own the paint

            SkIRect         fSrcRect;
        };

        ~PlaybackReplacements() { this->freeAll(); }

        // Add a new replacement range. The replacement ranges should be
        // sorted in increasing order and non-overlapping (esp. no nested
        // saveLayers).
        ReplacementInfo* push();

    private:
        friend class SkPicturePlayback; // for access to lookupByStart

        // look up a replacement range by its start offset
        ReplacementInfo* lookupByStart(size_t start);

        void freeAll();

#ifdef SK_DEBUG
        void validate() const;
#endif

        SkTDArray<ReplacementInfo> fReplacements;
    };

    // Replace all the draw ops in the replacement ranges in 'replacements' with
    // the associated drawBitmap call
    // Draw replacing cannot be enabled at the same time as draw limiting
    void setReplacements(PlaybackReplacements* replacements) {
        SkASSERT(fStart == 0 && fStop == 0);
        fReplacements = replacements;
    }

protected:
    const SkPictureData* fPictureData;

    // The offset of the current operation when within the draw method
    size_t fCurOffset;

    bool   fUseBBH;
    size_t fStart;
    size_t fStop;
    PlaybackReplacements* fReplacements;

    void handleOp(SkReader32* reader, 
                  DrawType op, 
                  uint32_t size, 
                  SkCanvas* canvas,
                  const SkMatrix& initialMatrix);

#ifdef SK_DEVELOPER
    virtual bool preDraw(int opIndex, int type) { return false; }
    virtual void postDraw(int opIndex) { }
#endif

private:
    typedef SkNoncopyable INHERITED;
};

#endif
