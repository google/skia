/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageGenerator.h"

#ifndef SK_SUPPORT_LEGACY_IMAGEGENERATORAPI
bool SkImageGenerator::getInfo(SkImageInfo* info) {
    SkImageInfo dummy;
    if (NULL == info) {
        info = &dummy;
    }
    return this->onGetInfo(info);
}

bool SkImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                 SkPMColor ctable[], int* ctableCount) {
    // Dummy (out-of-range) value to catch subclasses that forgot to respect ctable params.
    int localCTableCount = -1;
    int* localCTableCountPtr = NULL;

    if (kIndex_8_SkColorType == info.colorType()) {
        if (NULL == ctable || NULL == ctableCount) {
            return false;
        }
    } else {
        if (ctableCount) {
            *ctableCount = 0;
        }
        ctableCount = NULL;
        ctable = NULL;
    }

    if (kUnknown_SkColorType == info.colorType()) {
        return false;
    }
    if (NULL == pixels) {
        return false;
    }
    if (rowBytes < info.minRowBytes()) {
        return false;
    }

    bool success = this->onGetPixels(info, pixels, rowBytes, ctable, localCTableCountPtr);

    if (success && localCTableCountPtr) {
        SkASSERT(localCTableCount >= 0 && localCTableCount <= 256);
        if (ctableCount) {
            *ctableCount = localCTableCount;
        }
    }
    return success;
}

bool SkImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    SkASSERT(kIndex_8_SkColorType != info.colorType());
    if (kIndex_8_SkColorType == info.colorType()) {
        return false;
    }
    return this->getPixels(info, pixels, rowBytes, NULL, NULL);
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////

SkData* SkImageGenerator::onRefEncodedData() {
    return NULL;
}

bool SkImageGenerator::onGetInfo(SkImageInfo*) {
    return false;
}

bool SkImageGenerator::onGetPixels(const SkImageInfo&, void*, size_t, SkPMColor*, int*) {
    return false;
}
