/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDecodingImageGenerator.h"

#include "SkBitmapFactory.h"
#include "SkData.h"
#include "SkDiscardablePixelRef.h"
#include "SkImageDecoder.h"

SkDecodingImageGenerator::SkDecodingImageGenerator(SkData* data)
    : fData(data) {
    SkASSERT(fData != NULL);
    fData->ref();
}

SkDecodingImageGenerator::~SkDecodingImageGenerator() {
    fData->unref();
}

SkData* SkDecodingImageGenerator::refEncodedData() {
    fData->ref();
    return fData;
}

bool SkDecodingImageGenerator::getInfo(SkImageInfo* info) {
    SkASSERT(info != NULL);
    return SkImageDecoder::DecodeMemoryToTarget(fData->data(),
                                                fData->size(),
                                                info, NULL);
}

bool SkDecodingImageGenerator::getPixels(const SkImageInfo& info,
                                         void* pixels,
                                         size_t rowBytes) {
    SkASSERT(pixels != NULL);
    SkBitmapFactory::Target target = {pixels, rowBytes};
    SkImageInfo tmpInfo = info;
    return SkImageDecoder::DecodeMemoryToTarget(fData->data(),
                                                fData->size(),
                                                &tmpInfo, &target);
}
bool SkDecodingImageGenerator::Install(SkData* data, SkBitmap* dst) {
    SkASSERT(data != NULL);
    SkASSERT(dst != NULL);
    SkImageGenerator* gen(SkNEW_ARGS(SkDecodingImageGenerator, (data)));
    return SkDiscardablePixelRef::Install(gen, dst);
}
