// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "tools/Resources.h"

// http://crbug.com/957275
DEF_SIMPLE_GM(tilemodes_alpha, canvas, 512, 512) {
    sk_sp<SkImage> image = GetResourceAsImage("images/mandrill_64.png");
    if (!image) {
        return;
    }
    constexpr SkTileMode kModes[4] = {
        SkTileMode::kClamp,
        SkTileMode::kRepeat,
        SkTileMode::kMirror,
        SkTileMode::kDecal,
    };
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            SkRect rect = SkRect::MakeXYWH(128 * x + 1, 128 * y + 1, 126, 126);
            SkMatrix matrix = SkMatrix::MakeTrans(rect.x(), rect.y());
            SkPaint paint(SkColor4f{0, 0, 0, 0.5f});
            paint.setShader(image->makeShader(kModes[x], kModes[y], &matrix));
            canvas->drawRect(rect, paint);
        }
    }
}
