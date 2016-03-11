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

SkFlattenable* SkBitmapSourceDeserializer::CreateProc(SkReadBuffer& buffer) {
    SkFilterQuality filterQuality;
    if (buffer.isVersionLT(SkReadBuffer::kBitmapSourceFilterQuality_Version)) {
        filterQuality = kHigh_SkFilterQuality;
    } else {
        filterQuality = (SkFilterQuality)buffer.readInt();
    }
    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);
    SkBitmap bitmap;
    if (!buffer.readBitmap(&bitmap)) {
        return nullptr;
    }
    bitmap.setImmutable();

    SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bitmap));
    return SkImageSource::Create(image, src, dst, filterQuality);
}
