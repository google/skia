
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkMovie.h"

class SkBitmap;
class SkStream;

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream) {
    return NULL;
}

bool SkImageDecoder::DecodeFile(const char file[], SkBitmap*, SkBitmap::Config,
                                SkImageDecoder::Mode, SkImageDecoder::Format*) {
    return false;
}

bool SkImageDecoder::decode(SkStream*, SkBitmap* bitmap, SkBitmap::Config pref, Mode) {
    return false;
}

bool SkImageDecoder::DecodeStream(SkStream*, SkBitmap*, SkBitmap::Config, SkImageDecoder::Mode,
                                  SkImageDecoder::Format*) {
    return false;
}

bool SkImageDecoder::DecodeMemory(const void*, size_t, SkBitmap*, SkBitmap::Config,
                                  SkImageDecoder::Mode, SkImageDecoder::Format*) {
    return false;
}

SkImageDecoder* CreateJPEGImageDecoder() {
    return NULL;
}
/////////////////////////////////////////////////////////////////////////

SkMovie* SkMovie::DecodeStream(SkStream* stream) {
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

SkImageEncoder* SkImageEncoder::Create(Type t) {
    return NULL;
}

bool SkImageEncoder::EncodeFile(const char file[], const SkBitmap&, Type, int quality) {
    return false;
}

bool SkImageEncoder::EncodeStream(SkWStream*, const SkBitmap&, SkImageEncoder::Type, int) {
    return false;
}

bool SkImageEncoder::encodeStream(SkWStream*, const SkBitmap&, int) {
    return false;
}

/////////////////////////////////////////////////////////////////////////

#include "SkImages.h"

void SkImages::InitializeFlattenables() {}
