/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/GrSlug.h"

#include "include/core/SkCanvas.h"

GrTextReferenceFrame::~GrTextReferenceFrame() = default;

GrSlug::~GrSlug() = default;
sk_sp<GrSlug> GrSlug::ConvertBlob(
        SkCanvas* canvas, const SkTextBlob& blob, SkPoint origin, const SkPaint& paint) {
    return canvas->convertBlobToSlug(blob, origin, paint);
}

sk_sp<GrSlug> SkMakeSlugFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client);
sk_sp<GrSlug> GrSlug::MakeFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client) {
    return SkMakeSlugFromBuffer(buffer, client);
}

void GrSlug::draw(SkCanvas* canvas) {
    canvas->drawSlug(this);
}

// Most of GrSlug's implementation is in GrTextBlob.cpp to share common code.



