/*
 * Copyright 2009, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkImageEncoder.h"
#include "SkBitmap.h"
#include "SkStream.h"
#include "SkTemplates.h"

SkImageEncoder::~SkImageEncoder() {}

bool SkImageEncoder::encodeStream(SkWStream* stream, const SkBitmap& bm,
                                  int quality) {
    quality = SkMin32(100, SkMax32(0, quality));
    return this->onEncode(stream, bm, quality);
}

bool SkImageEncoder::encodeFile(const char file[], const SkBitmap& bm,
                                int quality) {
    quality = SkMin32(100, SkMax32(0, quality));
    SkFILEWStream   stream(file);
    return this->onEncode(&stream, bm, quality);
}

bool SkImageEncoder::EncodeFile(const char file[], const SkBitmap& bm, Type t,
                                int quality) {
    SkAutoTDelete<SkImageEncoder> enc(SkImageEncoder::Create(t));
    return enc.get() && enc.get()->encodeFile(file, bm, quality);
}

bool SkImageEncoder::EncodeStream(SkWStream* stream, const SkBitmap& bm, Type t,
                                int quality) {
    SkAutoTDelete<SkImageEncoder> enc(SkImageEncoder::Create(t));
    return enc.get() && enc.get()->encodeStream(stream, bm, quality);
}

