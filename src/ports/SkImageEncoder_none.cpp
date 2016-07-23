/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkImage.h"
#include "SkImageEncoder.h"
#include "SkMovie.h"
#include "SkPixelSerializer.h"
#include "SkStream.h"

/////////////////////////////////////////////////////////////////////////

// Empty implementation for SkMovie.

SkMovie* SkMovie::DecodeStream(SkStreamRewindable* stream) {
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////

// Empty implementations for SkImageEncoder.

SkImageEncoder* SkImageEncoder::Create(Type t) {
    return nullptr;
}

bool SkImageEncoder::EncodeFile(const char file[], const SkBitmap&, Type, int quality) {
    return false;
}

bool SkImageEncoder::EncodeStream(SkWStream*, const SkBitmap&, SkImageEncoder::Type, int) {
    return false;
}

SkData* SkImageEncoder::EncodeData(const SkBitmap&, Type, int quality) {
    return nullptr;
}

SkData* SkImageEncoder::EncodeData(const SkImageInfo&, const void* pixels, size_t rowBytes,
                                   Type, int quality) {
    return nullptr;
}

SkData* SkImageEncoder::EncodeData(const SkPixmap&, Type, int) {
    return nullptr;
}

bool SkImageEncoder::encodeStream(SkWStream*, const SkBitmap&, int) {
    return false;
}

SkData* SkImageEncoder::encodeData(const SkBitmap&, int) {
    return nullptr;
}

bool SkImageEncoder::encodeFile(const char file[], const SkBitmap& bm, int quality) {
    return false;
}

namespace {
class ImageEncoderPixelSerializer final : public SkPixelSerializer {
protected:
    bool onUseEncodedData(const void*, size_t) override { return true; }
    SkData* onEncode(const SkPixmap&) override { return nullptr; }
};
}  // namespace

SkPixelSerializer* SkImageEncoder::CreatePixelSerializer() {
    return new ImageEncoderPixelSerializer;
}

/////////////////////////////////////////////////////////////////////////
