/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBoxHierarchy.h"
#include "SkDrawable.h"
#include "SkRecord.h"
#include "SkRecorder.h"

class SkRecordedDrawable : public SkDrawable {
public:
    SkRecordedDrawable(SkRecord* record, SkBBoxHierarchy* bbh, SkDrawableList* drawableList,
                       const SkRect& bounds, bool doSaveLayerInfo)
        : fRecord(SkRef(record))
        , fBBH(SkSafeRef(bbh))
        , fDrawableList(drawableList)   // we take ownership
        , fBounds(bounds)
        , fDoSaveLayerInfo(doSaveLayerInfo)
    {}

    void flatten(SkWriteBuffer& buffer) const override;

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer);

    Factory getFactory() const override { return CreateProc; }

protected:
    SkRect onGetBounds() override { return fBounds; }

    void onDraw(SkCanvas* canvas) override;

    SkPicture* onNewPictureSnapshot() override;

private:
    SkAutoTUnref<SkRecord>          fRecord;
    SkAutoTUnref<SkBBoxHierarchy>   fBBH;
    SkAutoTDelete<SkDrawableList>   fDrawableList;
    const SkRect                    fBounds;
    const bool                      fDoSaveLayerInfo;
};
