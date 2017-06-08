/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapSourceDeserializer.h"

#include "SkBitmap.h"
#include "SkFilterQuality.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "SkReadBuffer.h"

sk_sp<SkFlattenable> SkBitmapSourceDeserializer::CreateProc(SkReadBuffer& buffer) {
    SkFilterQuality filterQuality = (SkFilterQuality)buffer.readInt();
    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);
    sk_sp<SkImage> image = buffer.readBitmapAsImage();
    if (image) {
        return SkImageSource::Make(std::move(image), src, dst, filterQuality);
    }
    return nullptr;
}
