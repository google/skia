/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "tools/Resources.h"

// This gm draws 8 images that are mostly the same when respecting the
// EXIF orientation tag. Each one has four quadrants (red, blue, green,
// yellow), and labels on the left, top, right and bottom. The only
// visual difference is a number in the middle corresponding to the
// EXIF tag for that image's jpg file.
DEF_SIMPLE_GM(orientation, canvas, 400, 320) {
    canvas->save();
    for (char i = '1'; i <= '8'; i++) {
        SkString path = SkStringPrintf("images/orientation/%c.jpg", i);
        auto image = GetResourceAsImage(path.c_str());
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
