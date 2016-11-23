/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPictureFlat.h"

#include "SkChecksum.h"
#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkMaskFilter.h"
#include "SkRasterizer.h"
#include "SkShader.h"
#include "SkTypeface.h"

///////////////////////////////////////////////////////////////////////////////

SkTypefacePlayback::SkTypefacePlayback() : fCount(0), fArray(nullptr) {}

SkTypefacePlayback::~SkTypefacePlayback() {
    this->reset(nullptr);
}

void SkTypefacePlayback::reset(const SkRefCntSet* rec) {
    for (int i = 0; i < fCount; i++) {
        SkASSERT(fArray[i]);
        fArray[i]->unref();
    }
    delete[] fArray;

    if (rec!= nullptr && rec->count() > 0) {
        fCount = rec->count();
        fArray = new SkRefCnt* [fCount];
        rec->copyToArray(fArray);
        for (int i = 0; i < fCount; i++) {
            fArray[i]->ref();
        }
    } else {
        fCount = 0;
        fArray = nullptr;
    }
}

void SkTypefacePlayback::setCount(int count) {
    this->reset(nullptr);

    fCount = count;
    fArray = new SkRefCnt* [count];
    sk_bzero(fArray, count * sizeof(SkRefCnt*));
}

SkRefCnt* SkTypefacePlayback::set(int index, SkRefCnt* obj) {
    SkASSERT((unsigned)index < (unsigned)fCount);
    SkRefCnt_SafeAssign(fArray[index], obj);
    return obj;
}
