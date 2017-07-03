/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSerialUtils_DEFINED
#define SkSerialUtils_DEFINED

#include "SkData.h"

class SkColorFilter;
class SkDrawLooper;
class SkImageFilter;
class SkMaskFilter;
class SkPathEffect;

class SkSerialUtils {
public:
    static sk_sp<SkData> WriteColorFilter(const SkColorFilter*);
    static sk_sp<SkData> WriteDrawLooper (const SkDrawLooper*);
    static sk_sp<SkData> WriteImageFilter(const SkImageFilter*);
    static sk_sp<SkData> WriteMaskFilter (const SkMaskFilter*);
    static sk_sp<SkData> WritePathEffect (const SkPathEffect*);

    sk_sp<SkColorFilter> ReadColorFilter(const void*, size_t);
    sk_sp<SkDrawLooper>  ReadDrawLooper (const void*, size_t);
    sk_sp<SkImageFilter> ReadImageFilter(const void*, size_t);
    sk_sp<SkMaskFilter>  ReadMaskFilter (const void*, size_t);
    sk_sp<SkPathEffect>  ReadPathEffect (const void*, size_t);
};

#endif
