/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePlayback_DEFINED
#define SkPicturePlayback_DEFINED

#include "SkPictureFlat.h"  // for DrawType

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
        , fReplacements(NULL) {
    }
    virtual ~SkPicturePlayback() { }

    virtual void draw(SkCanvas* canvas, SkDrawPictureCallback*);

    // Return the ID of the operation currently being executed when playing
    // back. 0 indicates no call is active.
    size_t curOpID() const { return fCurOffset; }
    void resetOpID() { fCurOffset = 0; }

    void setUseBBH(bool useBBH) { fUseBBH = useBBH; }

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
        fReplacements = replacements;
    }

protected:
    const SkPictureData* fPictureData;

    // The offset of the current operation when within the draw method
    size_t fCurOffset;

    bool   fUseBBH;
    PlaybackReplacements* fReplacements;

    void handleOp(SkReader32* reader, 
                  DrawType op, 
                  uint32_t size, 
                  SkCanvas* canvas,
                  const SkMatrix& initialMatrix);

    static DrawType ReadOpAndSize(SkReader32* reader, uint32_t* size);

    class AutoResetOpID {
    public:
        AutoResetOpID(SkPicturePlayback* playback) : fPlayback(playback) { }
        ~AutoResetOpID() {
            if (NULL != fPlayback) {
                fPlayback->resetOpID();
            }
        }

    private:
        SkPicturePlayback* fPlayback;
    };

private:
    typedef SkNoncopyable INHERITED;
};

#endif
