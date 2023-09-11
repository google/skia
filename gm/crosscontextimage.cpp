/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "tools/Resources.h"

DEF_SIMPLE_GPU_GM_CAN_FAIL(cross_context_image, rContext, canvas, errorMsg,
                           3 * 256 + 40, 256 + 128 + 30) {
    sk_sp<SkData> encodedData = GetResourceAsData("images/mandrill_256.png");
    if (!encodedData) {
        *errorMsg = "Could not load mandrill_256.png. Did you forget to set the resourcePath?";
        return skiagm::DrawResult::kFail;
    }

    auto dContext = rContext->asDirectContext();
    if (!dContext) {
        *errorMsg = "CrossContext image creation requires a direct context.";
        return skiagm::DrawResult::kSkip;
    }

    sk_sp<SkImage> images[3];
    images[0] = SkImages::DeferredFromEncodedData(encodedData);

    SkBitmap bmp;
    SkPixmap pixmap;
    SkAssertResult(images[0]->asLegacyBitmap(&bmp) &&
                   bmp.peekPixels(&pixmap));

    images[1] = SkImages::CrossContextTextureFromPixmap(dContext, pixmap, false);
    images[2] = SkImages::CrossContextTextureFromPixmap(dContext, pixmap, true);

    canvas->translate(10, 10);

    for (size_t i = 0; i < std::size(images); ++i) {
        canvas->save();

        canvas->drawImage(images[i], 0, 0);
        canvas->translate(0, 256 + 10);

        auto subset = images[i]->makeSubset(dContext, SkIRect::MakeXYWH(64, 64, 128, 128));
        SkASSERTF(subset, "subset was null");
        canvas->drawImage(subset, 0, 0);
        canvas->translate(128, 0);

        canvas->drawImageRect(images[i], SkRect::MakeWH(128, 128),
                              SkSamplingOptions(SkFilterMode::kLinear,
                                                SkMipmapMode::kLinear));

        canvas->restore();
        canvas->translate(256 + 10, 0);
    }
    return skiagm::DrawResult::kOk;
}
