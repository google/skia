
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkImage.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkMovie.h"
#include "SkStream.h"

class SkColorTable;

// Empty implementations for SkImageDecoder.

SkImageDecoder::SkImageDecoder() {}

SkImageDecoder::~SkImageDecoder() {}

SkImageDecoder* SkImageDecoder::Factory(SkStreamRewindable*) {
    return NULL;
}

void SkImageDecoder::copyFieldsToOther(SkImageDecoder* ) {}

bool SkImageDecoder::DecodeFile(const char[], SkBitmap*, SkColorType, Mode, Format*) {
    return false;
}

SkImageDecoder::Result SkImageDecoder::decode(SkStream*, SkBitmap*, SkColorType, Mode) {
    return kFailure;
}

bool SkImageDecoder::DecodeStream(SkStreamRewindable*, SkBitmap*, SkColorType, Mode, Format*) {
    return false;
}

bool SkImageDecoder::DecodeMemory(const void*, size_t, SkBitmap*, SkColorType, Mode, Format*) {
    return false;
}

bool SkImageDecoder::buildTileIndex(SkStreamRewindable*, int *width, int *height) {
    return false;
}

bool SkImageDecoder::onBuildTileIndex(SkStreamRewindable* stream,
                                      int* /*width*/, int* /*height*/) {
    SkDELETE(stream);
    return false;
}


bool SkImageDecoder::decodeSubset(SkBitmap*, const SkIRect&, SkColorType) {
    return false;
}

SkImageDecoder::Format SkImageDecoder::getFormat() const {
    return kUnknown_Format;
}

SkImageDecoder::Format SkImageDecoder::GetStreamFormat(SkStreamRewindable*) {
    return kUnknown_Format;
}

const char* SkImageDecoder::GetFormatName(Format) {
    return NULL;
}

SkImageDecoder::Peeker* SkImageDecoder::setPeeker(Peeker*) {
    return NULL;
}

SkBitmap::Allocator* SkImageDecoder::setAllocator(SkBitmap::Allocator*) {
    return NULL;
}

void SkImageDecoder::setSampleSize(int) {}

bool SkImageDecoder::cropBitmap(SkBitmap*, SkBitmap*, int, int, int, int, int,
                    int, int) {
    return false;
}

bool SkImageDecoder::allocPixelRef(SkBitmap*, SkColorTable*) const {
    return false;
}

/////////////////////////////////////////////////////////////////////////

// Empty implementation for SkMovie.

SkMovie* SkMovie::DecodeStream(SkStreamRewindable* stream) {
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
