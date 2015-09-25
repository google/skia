/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapSourceDeserializer_DEFINED
#define SkBitmapSourceDeserializer_DEFINED

#include "SkFlattenable.h"

// A temporary utility class to support deserializing legacy SkBitmapSource as SkImageSource.
// Should be removed when SKP versions which may contain SkBitmapSource records are phased out.
class SkBitmapSourceDeserializer : public SkFlattenable {
public:
    SK_DEFINE_FLATTENABLE_TYPE(SkImageFilter)
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBitmapSource)
};

#endif
