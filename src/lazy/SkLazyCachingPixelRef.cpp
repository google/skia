/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk64.h"
#include "SkColorTable.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkImagePriv.h"
#include "SkLazyCachingPixelRef.h"
#include "SkPostConfig.h"

SkLazyCachingPixelRef::SkLazyCachingPixelRef(SkData* data,
                                             SkBitmapFactory::DecodeProc proc)
    : fDecodeProc(proc) {
    if (NULL == data) {
        fData = SkData::NewEmpty();
    } else {
        fData = data;
        fData->ref();
    }
    if (NULL == fDecodeProc) {  // use a reasonable default.
        fDecodeProc = SkImageDecoder::DecodeMemoryToTarget;
    }
    this->setImmutable();
}

SkLazyCachingPixelRef::~SkLazyCachingPixelRef() {
    SkASSERT(fData != NULL);
    fData->unref();
}

bool SkLazyCachingPixelRef::onDecodeInfo(SkImageInfo* info) {
    SkASSERT(info);
    return fDecodeProc(fData->data(), fData->size(), info, NULL);
}

bool SkLazyCachingPixelRef::onDecodePixels(const SkImageInfo& passedInfo,
                                           void* pixels, size_t rowBytes) {
    SkASSERT(pixels);
    SkImageInfo info;
    if (!this->getInfo(&info)) {
        return false;
    }
    if (passedInfo != info) {
        return false;  // This implementation can not handle this case.
    }
    SkBitmapFactory::Target target = {pixels, rowBytes};
    return fDecodeProc(fData->data(), fData->size(), &info, &target);
}

bool SkLazyCachingPixelRef::Install(SkBitmapFactory::DecodeProc proc,
                                    SkData* data,
                                    SkBitmap* destination) {
    SkAutoTUnref<SkLazyCachingPixelRef> ref(
        SkNEW_ARGS(SkLazyCachingPixelRef, (data, proc)));
    return ref->configure(destination) && destination->setPixelRef(ref);
}
