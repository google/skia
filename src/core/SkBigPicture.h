/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBigPicture_DEFINED
#define SkBigPicture_DEFINED

#include "SkOncePtr.h"
#include "SkPicture.h"
#include "SkRect.h"
#include "SkTemplates.h"

class SkBBoxHierarchy;
class SkMatrix;
class SkRecord;

// An implementation of SkPicture supporting an arbitrary number of drawing commands.
class SkBigPicture final : public SkPicture {
public:
    // AccelData provides a base class for device-specific acceleration data.
    class AccelData : public SkRefCnt { };

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
                 AccelData*,           // We take ownership of the caller's ref.
                 size_t approxBytesUsedBySubPictures);


// SkPicture overrides
    void playback(SkCanvas*, AbortCallback*) const override;
    SkRect cullRect() const override;
    bool hasText() const override;
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
    const SkBBoxHierarchy* bbh() const { return fBBH; }
    const SkRecord*     record() const { return fRecord; }
    const AccelData* accelData() const { return fAccelData; }

private:
    struct Analysis {
        explicit Analysis(const SkRecord&);

        bool suitableForGpuRasterization(const char** reason) const;

        uint8_t fNumSlowPathsAndDashEffects;
        bool    fWillPlaybackBitmaps : 1;
        bool    fHasText             : 1;
    };

    int numSlowPaths() const override;
    const Analysis& analysis() const;
    int drawableCount() const;
    SkPicture const* const* drawablePicts() const;

    const SkRect                          fCullRect;
    const size_t                          fApproxBytesUsedBySubPictures;
    SkOncePtr<const Analysis>             fAnalysis;
    SkAutoTUnref<const SkRecord>          fRecord;
    SkAutoTDelete<const SnapshotArray>    fDrawablePicts;
    SkAutoTUnref<const SkBBoxHierarchy>   fBBH;
    SkAutoTUnref<const AccelData>         fAccelData;
};

#endif//SkBigPicture_DEFINED
