/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include "include/ports/SkImageGeneratorCG.h"
#endif
#include "tools/Resources.h"

// This gm draws 8 images that are mostly the same when respecting the
// EXIF orientation tag. Each one has four quadrants (red, blue, green,
// yellow), and labels on the left, top, right and bottom. The only
// visual difference is a number in the middle corresponding to the
// EXIF tag for that image's jpg file.
DEF_SIMPLE_GM(orientation, canvas, 400, 320) {
    canvas->save();
    const SkGraphics::ImageGeneratorFromEncodedDataFactory factory =
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
            !canvas->getGrContext() ? SkImageGeneratorCG::MakeFromEncodedCG :
#endif
                                      SkImageGenerator::MakeFromEncoded;
    for (char i = '1'; i <= '8'; i++) {
        SkString path = SkStringPrintf("images/orientation/%c.jpg", i);
        auto data = GetResourceAsData(path.c_str());
        if (!data) {
            continue;
        }
        auto image = SkImage::MakeFromGenerator(factory(std::move(data)));
        if (!image) {
            continue;
        }
        canvas->drawImage(image, 0, 0);
        if ('4' == i) {
            canvas->restore();
            canvas->translate(0, image->height());
        } else {
            canvas->translate(image->width(), 0);
        }
    }
}
