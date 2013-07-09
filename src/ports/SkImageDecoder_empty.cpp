
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapFactory.h"
#include "SkImage.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkMovie.h"

class SkColorTable;
class SkStream;

// Empty implementations for SkImageDecoder.

SkImageDecoder* SkImageDecoder::Factory(SkStream*) {
    return NULL;
}

void SkImageDecoder::copyFieldsToOther(SkImageDecoder* ) {}

bool SkImageDecoder::DecodeFile(const char[], SkBitmap*, SkBitmap::Config,
                                SkImageDecoder::Mode, SkImageDecoder::Format*) {
    return false;
}

bool SkImageDecoder::decode(SkStream*, SkBitmap*, SkBitmap::Config, Mode) {
    return false;
}

bool SkImageDecoder::DecodeStream(SkStream*, SkBitmap*, SkBitmap::Config,
                                  SkImageDecoder::Mode,
                                  SkImageDecoder::Format*) {
    return false;
}

bool SkImageDecoder::DecodeMemory(const void*, size_t, SkBitmap*,
                                  SkBitmap::Config, SkImageDecoder::Mode,
                                  SkImageDecoder::Format*) {
    return false;
}

bool SkImageDecoder::buildTileIndex(SkStream*, int *width, int *height) {
    return false;
}

bool SkImageDecoder::decodeSubset(SkBitmap*, const SkIRect&, SkBitmap::Config) {
    return false;
}

SkImageDecoder::Format SkImageDecoder::getFormat() const {
    return kUnknown_Format;
}

SkImageDecoder::Format SkImageDecoder::GetStreamFormat(SkStream*) {
    return kUnknown_Format;
}

const char* SkImageDecoder::GetFormatName(Format) {
    return NULL;
}

SkImageDecoder::Peeker* SkImageDecoder::setPeeker(Peeker*) {
    return NULL;
}

SkImageDecoder::Chooser* SkImageDecoder::setChooser(Chooser*) {
    return NULL;
}

SkBitmap::Allocator* SkImageDecoder::setAllocator(SkBitmap::Allocator*) {
    return NULL;
}

void SkImageDecoder::setSampleSize(int) {}

bool SkImageDecoder::DecodeMemoryToTarget(const void*, size_t, SkImage::Info*,
                                          const SkBitmapFactory::Target*) {
    return false;
}

SkBitmap::Config SkImageDecoder::GetDeviceConfig() {
    return SkBitmap::kNo_Config;
}

void SkImageDecoder::SetDeviceConfig(SkBitmap::Config) {}

bool SkImageDecoder::cropBitmap(SkBitmap*, SkBitmap*, int, int, int, int, int,
                    int, int) {
    return false;
}

bool SkImageDecoder::chooseFromOneChoice(SkBitmap::Config, int, int) const {
    return false;
}

bool SkImageDecoder::allocPixelRef(SkBitmap*, SkColorTable*) const {
    return false;
}

SkBitmap::Config SkImageDecoder::getPrefConfig(SrcDepth, bool) const {
    return SkBitmap::kNo_Config;
}


/////////////////////////////////////////////////////////////////////////

// Empty implementation for SkMovie.

SkMovie* SkMovie::DecodeStream(SkStream* stream) {
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

// Empty implementations for SkImageEncoder.

SkImageEncoder* SkImageEncoder::Create(Type t) {
    return NULL;
}

bool SkImageEncoder::EncodeFile(const char file[], const SkBitmap&, Type, int quality) {
    return false;
}

bool SkImageEncoder::EncodeStream(SkWStream*, const SkBitmap&, SkImageEncoder::Type, int) {
    return false;
}

SkData* SkImageEncoder::EncodeData(const SkBitmap&, Type, int quality) {
    return NULL;
}

bool SkImageEncoder::encodeStream(SkWStream*, const SkBitmap&, int) {
    return false;
}

SkData* SkImageEncoder::encodeData(const SkBitmap&, int) {
    return NULL;
}

bool SkImageEncoder::encodeFile(const char file[], const SkBitmap& bm, int quality) {
    return false;
}
/////////////////////////////////////////////////////////////////////////

// Empty implementation for SkImages.

#include "SkImages.h"

void SkImages::InitializeFlattenables() {}
