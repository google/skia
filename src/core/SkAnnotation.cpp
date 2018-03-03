/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnnotation.h"
#include "SkAnnotationKeys.h"
#include "SkCanvas.h"
#include "SkPoint.h"
#include "SkRect.h"

const char* SkAnnotationKeys::URL_Key() {
    return "SkAnnotationKey_URL";
};

const char* SkAnnotationKeys::Define_Named_Dest_Key() {
    return "SkAnnotationKey_Define_Named_Dest";
};

const char* SkAnnotationKeys::Link_Named_Dest_Key() {
    return "SkAnnotationKey_Link_Named_Dest";
};

//////////////////////////////////////////////////////////////////////////////////////////////////

void SkAnnotateRectWithURL(SkCanvas* canvas, const SkRect& rect, SkData* value) {
    if (nullptr == value) {
        return;
    }
    canvas->drawAnnotation(rect, SkAnnotationKeys::URL_Key(), value);
}

void SkAnnotateNamedDestination(SkCanvas* canvas, const SkPoint& point, SkData* name) {
    if (nullptr == name) {
        return;
    }
    const SkRect rect = SkRect::MakeXYWH(point.x(), point.y(), 0, 0);
    canvas->drawAnnotation(rect, SkAnnotationKeys::Define_Named_Dest_Key(), name);
}

void SkAnnotateLinkToDestination(SkCanvas* canvas, const SkRect& rect, SkData* name) {
    if (nullptr == name) {
        return;
    }
    canvas->drawAnnotation(rect, SkAnnotationKeys::Link_Named_Dest_Key(), name);
}

////////////////////////////
#include "SkDArray.h"

SkDArray::SkDArray(const void* src, size_t elemSize, int count) : fElemSize(elemSize) {
    SkASSERT(elemSize);
    SkASSERT(src || count == 0);

    fReserve = fCount = 0;
    fArray = nullptr;
    if (count) {
        this->setCount(count);
        memcpy(fArray, src, this->countBytes());
    }
}

SkDArray::SkDArray(const SkDArray& src)
: fArray(nullptr), fElemSize(src.fElemSize), fReserve(0), fCount(0) {
    SkDArray tmp(src.fArray, src.fElemSize, src.fCount);
    this->swap(tmp); // use move
}
SkDArray::SkDArray(SkDArray&& src)
: fArray(nullptr), fElemSize(src.fElemSize), fReserve(0), fCount(0) {
    this->swap(src);
}

SkDArray& SkDArray::operator=(const SkDArray& src) {
    if (this != &src) {
        if (src.countBytes() > this->reserveBytes()) {
            SkDArray tmp(src.fArray, src.fElemSize, src.fCount);
            this->swap(tmp);
        } else {
            sk_careful_memcpy(fArray, src.fArray, src.countBytes());
            fElemSize = src.fElemSize;
            fCount = src.fCount;
        }
    }
    return *this;
}
SkDArray& SkDArray::operator=(SkDArray&& src) {
    if (this != &src) {
        this->swap(src);
        src.reset();
    }
    return *this;
}

void* SkDArray::append(int count, const void* src) {
    int oldCount = fCount;
    if (count)  {
        SkASSERT(src == nullptr || fArray == nullptr ||
                 (const char*)src + count * fElemSize <= (const char*)fArray ||
                 (const char*)fArray + oldCount * fElemSize <= (const char*)src);

        this->adjustCount(count);
        if (src) {
            memcpy((char*)fArray + oldCount * fElemSize, src, fElemSize * count);
        }
    }
    return (char*)fArray + oldCount * fElemSize;
}

void* SkDArray::insert(int index, int count, const void* src) {
    SkASSERT(count);
    SkASSERT(index <= fCount);
    size_t oldCount = fCount;
    this->adjustCount(count);
    char* dst = (char*)fArray + index * fElemSize;
    memmove(dst + count * fElemSize, dst, fElemSize * (oldCount - index));
    if (src) {
        memcpy(dst, src, fElemSize * count);
    }
    return dst;
}

void SkDArray::remove(int index, int count) {
    SkASSERT(index + count <= fCount);
    fCount = fCount - count;
    memmove((char*)fArray + index * fElemSize,
            (const char*)fArray + (index + count) * fElemSize,
            fElemSize * (fCount - index));
}

void SkDArray::removeShuffle(int index, int count) {
    SkASSERT(index < fCount);
    int newCount = fCount - count;
    fCount = newCount;
    if (index != newCount) {
        memcpy((char*)fArray + index * fElemSize, (const char*)fArray + newCount * fElemSize,
               fElemSize);
    }
}
