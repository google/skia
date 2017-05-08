/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "SkImage.h"

DEF_SIMPLE_GM(cross_context_image, canvas, 512 + 512 + 30, 512 + 128 + 30) {
    GrContext* context = canvas->getGrContext();
    if (!context) {
        skiagm::GM::DrawGpuOnlyMessage(canvas);
        return;
    }

    sk_sp<SkData> encodedData = GetResourceAsData("mandrill_512.png");

    sk_sp<SkImage> encodedImage = SkImage::MakeFromEncoded(encodedData);
    canvas->drawImage(encodedImage, 10, 10);

    sk_sp<SkImage> crossContextImage = SkImage::MakeCrossContextFromEncoded(
        context, encodedData, false, canvas->imageInfo().colorSpace());
    canvas->drawImage(crossContextImage, 512 + 30, 10);

    SkIRect subset = SkIRect::MakeXYWH(256 - 64, 256 - 64, 128, 128);
    sk_sp<SkImage> encodedSubset = encodedImage->makeSubset(subset);
    sk_sp<SkImage> crossContextSubset = crossContextImage->makeSubset(subset);

    canvas->drawImage(encodedSubset, 10, 512 + 30);
    canvas->drawImage(crossContextSubset, 512 + 30, 512 + 30);
}

#endif
