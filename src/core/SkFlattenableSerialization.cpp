/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenableSerialization.h"

#include "SkData.h"
#include "SkValidatingReadBuffer.h"
#include "SkOrderedWriteBuffer.h"

SkData* SkValidatingSerializeFlattenable(SkFlattenable* flattenable) {
    SkOrderedWriteBuffer writer(1024);
    writer.setFlags(SkOrderedWriteBuffer::kValidation_Flag);
    writer.writeFlattenable(flattenable);
    uint32_t size = writer.bytesWritten();
    void* data = sk_malloc_throw(size);
    writer.writeToMemory(data);
    return SkData::NewFromMalloc(data, size);
}

SkFlattenable* SkValidatingDeserializeFlattenable(const void* data, size_t size,
                                                  SkFlattenable::Type type) {
    SkValidatingReadBuffer buffer(data, size);
    return buffer.readFlattenable(type);
}
