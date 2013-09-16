/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFlattenableSerialization_DEFINED
#define SkFlattenableSerialization_DEFINED

#include "SkTypes.h"

class SkData;
class SkFlattenable;

SK_API SkData* SkSerializeFlattenable(SkFlattenable*);
SK_API SkFlattenable* SkDeserializeFlattenable(const void* data, size_t size);

#endif
