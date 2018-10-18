/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRecordedDrawable_DEFINED
#define SkRecordedDrawable_DEFINED

#include "SkBBoxHierarchy.h"
#include "SkDrawable.h"
#include "SkRecord.h"
#include "SkRecorder.h"

class SkRecordedDrawable : public SkDrawable {
public:
    SkRecordedDrawable(sk_sp<SkRecord> record, sk_sp<SkBBoxHierarchy> bbh,
                       std::unique_ptr<SkDrawableList> drawableList, const SkRect& bounds)
        : fRecord(std::move(record))
        , fBBH(std::move(bbh))
        , fDrawableList(std::move(drawableList))
        , fBounds(bounds)
    {}

    void flatten(SkWriteBuffer& buffer) const override;

protected:
    SkRect onGetBounds() override { return fBounds; }

    void onDraw(SkCanvas* canvas) override;

    SkPicture* onNewPictureSnapshot() override;

private:
    SK_FLATTENABLE_HOOKS(SkRecordedDrawable)

    sk_sp<SkRecord>                 fRecord;
    sk_sp<SkBBoxHierarchy>          fBBH;
    std::unique_ptr<SkDrawableList> fDrawableList;
    const SkRect                    fBounds;
};
#endif  // SkRecordedDrawable_DEFINED
