/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenableSerialization.h"
#include "SkData.h"
#include "SkImageFilter.h"

SkData* SkValidatingSerializeFlattenable(SkFlattenable* flattenable) {
    return flattenable->serialize().release();
}

SkFlattenable* SkValidatingDeserializeFlattenable(const void* data, size_t size,
                                                  SkFlattenable::Type type) {
    return SkFlattenable::Deserialize(type, data, size).release();
}

sk_sp<SkImageFilter> SkValidatingDeserializeImageFilter(const void* data, size_t size) {
    return SkImageFilter::Deserialize(data, size);
}
