/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureReplacementPlayback_DEFINED
#define SkPictureReplacementPlayback_DEFINED

#include "SkPicturePlayback.h"

// This playback class replaces complete "saveLayer ... restore" runs with a
// single drawBitmap call.
class SkPictureReplacementPlayback : public SkPicturePlayback {
public:
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

        // look up a replacement range by its start offset
        ReplacementInfo* lookupByStart(size_t start);

    private:
        SkTDArray<ReplacementInfo> fReplacements;

        void freeAll();

#ifdef SK_DEBUG
        void validate() const;
#endif
    };

    // This class doesn't take ownership of either 'replacements' or 'activeOpsList'
    // The caller must guarantee they exist across any calls to 'draw'.
    // 'activeOpsList' can be NULL but in that case BBH acceleration will not 
    // be used ('replacements' can be NULL too but that defeats the purpose
    // of using this class).
    SkPictureReplacementPlayback(const SkPicture* picture, 
                                 PlaybackReplacements* replacements,
                                 const SkPicture::OperationList* activeOpsList)
        : INHERITED(picture)
        , fReplacements(replacements)
        , fActiveOpsList(activeOpsList) {
    }

    virtual void draw(SkCanvas* canvas, SkDrawPictureCallback*) SK_OVERRIDE;

private:
    PlaybackReplacements*           fReplacements;
    const SkPicture::OperationList* fActiveOpsList;

    // This method checks if the current op pointed at by 'iter' and 'reader'
    // is within a replacement range. If so, it issues the drawBitmap call,
    // updates 'iter' and 'reader' to be after the restore operation, and
    // returns true. If the operation is not in a replacement range (and thus
    // needs to be drawn normally) false is returned.
    bool replaceOps(SkPictureStateTree::Iterator* iter,
                    SkReader32* reader,
                    SkCanvas* canvas,
                    const SkMatrix& initialMatrix);

    typedef SkPicturePlayback INHERITED;
};

#endif
