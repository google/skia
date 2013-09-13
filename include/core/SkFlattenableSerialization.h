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

/**
 *  These utility functions are used by the chromium codebase to safely
 *  serialize and deserialize SkFlattenable objects. These aren't made for
 *  optimal speed, but rather designed with security in mind in order to
 *  prevent Skia from being an entry point for potential attacks.
 */
SK_API SkData* SkValidatingSerializeFlattenable(SkFlattenable*);
SK_API SkFlattenable* SkValidatingDeserializeFlattenable(const void* data, size_t size);

#endif
