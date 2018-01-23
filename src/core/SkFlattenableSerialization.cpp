/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenableSerialization.h"
#include "SkImageFilter.h"

sk_sp<SkImageFilter> SkValidatingDeserializeImageFilter(const void* data, size_t size) {
    return SkImageFilter::Deserialize(data, size);
}
