/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCanvas.h"
#include "gm.h"

/*
 *  Test decoding grayscale JPEG
 *  http://crbug.com/436079
 */
DEF_SIMPLE_GM(grayscalejpg, canvas, 128, 128) {
    const char kResource[] = "grayscale.jpg";
    SkBitmap bitmap;
    if (GetResourceAsBitmap(kResource, &bitmap)) {
        canvas->drawBitmap(bitmap, 0.0f, 0.0f);
    } else {
        SkDebugf("\nCould not decode file '%s'. Did you forget"
                 " to set the resourcePath?\n", kResource);
    }
}
