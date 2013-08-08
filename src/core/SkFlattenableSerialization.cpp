/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenableSerialization.h"

#include "SkData.h"
#include "SkFlattenable.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"

SkData* SkSerializeFlattenable(SkFlattenable* flattenable) {
    SkOrderedWriteBuffer writer(1024);
    writer.setFlags(SkOrderedWriteBuffer::kCrossProcess_Flag);
    writer.writeFlattenable(flattenable);
    uint32_t size = writer.bytesWritten();
    void* data = sk_malloc_throw(size);
    writer.writeToMemory(data);
    return SkData::NewFromMalloc(data, size);
}

SkFlattenable* SkDeserializeFlattenable(const void* data, size_t size) {
    SkOrderedReadBuffer buffer(data, size);
    return buffer.readFlattenable();
}
