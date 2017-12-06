/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkMatrixPriv.h"
#include "SkValidatingReadBuffer.h"
#include "SkStream.h"
#include "SkTypeface.h"

#if 0
SkValidatingReadBuffer::SkValidatingReadBuffer(const void* data, size_t size) :
    fError(false) {
    this->setMemory(data, size);
    this->setFlags(SkReadBuffer::kValidation_Flag);
}

SkValidatingReadBuffer::~SkValidatingReadBuffer() {
}

bool SkValidatingReadBuffer::validate(bool isValid) {
    if (!fError && !isValid) {
        // When an error is found, send the read cursor to the end of the stream
        fReader.skip(fReader.available());
        fError = true;
    }
    return !fError;
}

bool SkValidatingReadBuffer::isValid() const {
    return !fError;
}

void SkValidatingReadBuffer::setMemory(const void* data, size_t size) {
    this->validate(IsPtrAlign4(data) && (SkAlign4(size) == size));
    if (!fError) {
        fReader.setMemory(data, size);
    }
}

const void* SkValidatingReadBuffer::skip(size_t size) {
    size_t inc = SkAlign4(size);
    this->validate(inc >= size);
    const void* addr = fReader.peek();
    this->validate(IsPtrAlign4(addr) && fReader.isAvailable(inc));
    if (fError) {
        return nullptr;
    }

    fReader.skip(size);
    return addr;
}
#endif
