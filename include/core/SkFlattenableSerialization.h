/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFlattenableSerialization_DEFINED
#define SkFlattenableSerialization_DEFINED

#include "SkFlattenable.h"

class SkData;
class SkImageFilter;
class SkTextBlob;
class SkTypeface;

SK_API SkData* SkValidatingSerializeFlattenable(SkFlattenable*);
SK_API SkFlattenable* SkValidatingDeserializeFlattenable(const void* data, size_t size,
                                                         SkFlattenable::Type type);

SK_API sk_sp<SkImageFilter> SkValidatingDeserializeImageFilter(const void* data, size_t size);

SK_API sk_sp<SkData> SkValidatingSerializeTypeface(SkTypeface*);
SK_API sk_sp<SkTypeface> SkValidatingDeserializeTypeface(const void* data, size_t size);

SK_API sk_sp<SkData> SkValidatingSerializeTextBlob(SkTextBlob*);
SK_API sk_sp<SkTextBlob> SkValidatingDeserializeTextBlob(const void* data, size_t size);

#endif
