/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenableSerialization.h"

#include "SkData.h"
#include "SkValidatingReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

SkData* SkValidatingSerializeFlattenable(SkFlattenable* flattenable) {
    SkBinaryWriteBuffer writer;
    writer.writeFlattenable(flattenable);
    size_t size = writer.bytesWritten();
    auto data = SkData::MakeUninitialized(size);
    writer.writeToMemory(data->writable_data());
    return data.release();
}

SkFlattenable* SkValidatingDeserializeFlattenable(const void* data, size_t size,
                                                  SkFlattenable::Type type) {
    SkValidatingReadBuffer buffer(data, size);
    return buffer.readFlattenable(type);
}

sk_sp<SkImageFilter> SkValidatingDeserializeImageFilter(const void* data, size_t size) {
    return sk_sp<SkImageFilter>((SkImageFilter*)SkValidatingDeserializeFlattenable(
                                data, size, SkImageFilter::GetFlattenableType()));
}

sk_sp<SkData> SkValidatingSerializeTypeface(SkTypeface* typeface) {
    SkBinaryWriteBuffer writer;
    writer.writeTypeface(typeface);
    size_t size = writer.bytesWritten();
    auto data = SkData::MakeUninitialized(size);
    writer.writeToMemory(data->writable_data());
    return data;
}

sk_sp<SkTypeface> SkValidatingDeserializeTypeface(const void* data, size_t size) {
    SkValidatingReadBuffer buffer(data, size);
    return buffer.readTypeface();
}

sk_sp<SkData> SkValidatingSerializeTextBlob(SkTextBlob* blob) {
    SkBinaryWriteBuffer writer;
    blob->flatten(writer);
    size_t size = writer.bytesWritten();
    auto data = SkData::MakeUninitialized(size);
    writer.writeToMemory(data->writable_data());
    return data;
}

sk_sp<SkTextBlob> SkValidatingDeserializeTextBlob(const void* data, size_t size) {
    SkValidatingReadBuffer buffer(data, size);
    return SkTextBlob::MakeFromBuffer(buffer);
}
