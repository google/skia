/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Resources_DEFINED
#define Resources_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkString.h"

class SkBitmap;
class SkData;
class SkStreamAsset;
class SkTypeface;

SkString GetResourcePath(const char* resource = "");

void SetResourcePath(const char* );

bool DecodeDataToBitmap(sk_sp<SkData> data, SkBitmap* dst);

sk_sp<SkData> GetResourceAsData(const char* resource);

inline bool GetResourceAsBitmap(const char* resource, SkBitmap* dst) {
    return DecodeDataToBitmap(GetResourceAsData(resource), dst);
}

inline sk_sp<SkImage> GetResourceAsImage(const char* resource) {
    return SkImage::MakeFromEncoded(GetResourceAsData(resource));
}

std::unique_ptr<SkStreamAsset> GetResourceAsStream(const char* resource);

sk_sp<SkTypeface> MakeResourceAsTypeface(const char* resource, int ttcIndex = 0);

#endif  // Resources_DEFINED
