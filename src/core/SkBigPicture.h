/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBigPicture_DEFINED
#define SkBigPicture_DEFINED

#include "SkOnce.h"
#include "SkPicture.h"
#include "SkRect.h"
#include "SkTemplates.h"

class SkBBoxHierarchy;
class SkMatrix;
class SkRecord;

// An implementation of SkPicture supporting an arbitrary number of drawing commands.
class SkBigPicture final : public SkPicture {
public:
    // An array of refcounted const SkPicture pointers.
    class SnapshotArray : ::SkNoncopyable {
    public:
        SnapshotArray(const SkPicture* pics[], int count) : fPics(pics), fCount(count) {}
        ~SnapshotArray() { for (int i = 0; i < fCount; i++) { fPics[i]->unref(); } }

        const SkPicture* const* begin() const { return fPics; }
        int count() const { return fCount; }
    private:
        SkAutoTMalloc<const SkPicture*> fPics;
        int fCount;
    };

    SkBigPicture(const SkRect& cull,
                 SkRecord*,            // We take ownership of the caller's ref.
                 SnapshotArray*,       // We take exclusive ownership.
                 SkBBoxHierarchy*,     // We take ownership of the caller's ref.
                 size_t approxBytesUsedBySubPictures);


// SkPicture overrides
    void playback(SkCanvas*, AbortCallback*) const override;
    SkRect cullRect() const override;
    bool willPlayBackBitmaps() const override;
    int approximateOpCount() const override;
    size_t approximateBytesUsed() const override;
    const SkBigPicture* asSkBigPicture() const override { return this; }

// Used by GrLayerHoister
    void partialPlayback(SkCanvas*,
                         int start,
                         int stop,
                         const SkMatrix& initialCTM) const;
// Used by GrRecordReplaceDraw
    const SkBBoxHierarchy* bbh() const { return fBBH.get(); }
    const SkRecord*     record() const { return fRecord.get(); }

private:
    struct Analysis {
        void init(const SkRecord&);

        bool suitableForGpuRasterization(const char** reason) const;

        uint8_t fNumSlowPathsAndDashEffects;
        bool    fWillPlaybackBitmaps : 1;
    };

    int numSlowPaths() const override;
    const Analysis& analysis() const;
    int drawableCount() const;
    SkPicture const* const* drawablePicts() const;

    const SkRect                         fCullRect;
    const size_t                         fApproxBytesUsedBySubPictures;
    mutable SkOnce                       fAnalysisOnce;
    mutable Analysis                     fAnalysis;
    sk_sp<const SkRecord>                fRecord;
    std::unique_ptr<const SnapshotArray> fDrawablePicts;
    sk_sp<const SkBBoxHierarchy>         fBBH;
};

#endif//SkBigPicture_DEFINED
