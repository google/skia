/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBigPicture_DEFINED
#define SkBigPicture_DEFINED

#include "include/core/SkBBHFactory.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkRecord.h"

#include <cstddef>
#include <memory>

class SkCanvas;

// An implementation of SkPicture supporting an arbitrary number of drawing commands.
// This is called "big" because there used to be a "mini" that only supported a subset of the
// calls as an optimization.
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
        skia_private::AutoTMalloc<const SkPicture*> fPics;
        int fCount;
    };

    SkBigPicture(const SkRect& cull,
                 sk_sp<SkRecord>,
                 std::unique_ptr<SnapshotArray>,
                 sk_sp<SkBBoxHierarchy>,
                 size_t approxBytesUsedBySubPictures);


// SkPicture overrides
    void playback(SkCanvas*, AbortCallback*) const override;
    SkRect cullRect() const override;
    int approximateOpCount(bool nested) const override;
    size_t approximateBytesUsed() const override;
    const SkBigPicture* asSkBigPicture() const override { return this; }

// Used by GrRecordReplaceDraw
    const SkBBoxHierarchy* bbh() const { return fBBH.get(); }
    const SkRecord*     record() const { return fRecord.get(); }

private:
    int drawableCount() const;
    SkPicture const* const* drawablePicts() const;

    const SkRect                         fCullRect;
    const size_t                         fApproxBytesUsedBySubPictures;
    sk_sp<const SkRecord>                fRecord;
    std::unique_ptr<const SnapshotArray> fDrawablePicts;
    sk_sp<const SkBBoxHierarchy>         fBBH;
};

#endif//SkBigPicture_DEFINED
