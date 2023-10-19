/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkMipmapBuilder.h"

#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkTypes.h"
#include "src/core/SkMipmap.h"
struct SkImageInfo;

SkMipmapBuilder::SkMipmapBuilder(const SkImageInfo& info) {
    fMM = sk_sp<SkMipmap>(SkMipmap::Build({info, nullptr, 0}, nullptr, false));
}

SkMipmapBuilder::~SkMipmapBuilder() {}

int SkMipmapBuilder::countLevels() const {
    return fMM ? fMM->countLevels() : 0;
}

SkPixmap SkMipmapBuilder::level(int index) const {
    SkPixmap pm;

    SkMipmap::Level level;
    if (fMM && fMM->getLevel(index, &level)) {
        pm = level.fPixmap;
    }
    return pm;
}

sk_sp<SkImage> SkMipmapBuilder::attachTo(const sk_sp<const SkImage>& src) {
    return src->withMipmaps(fMM);
}
