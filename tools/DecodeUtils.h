/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DecodeUtils_DEFINED
#define DecodeUtils_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "tools/Resources.h"

class SkBitmap;
class SkData;
enum SkColorType : int;

namespace ToolUtils {
bool DecodeDataToBitmap(sk_sp<SkData> data, SkBitmap* dst);
bool DecodeDataToBitmapWithColorType(sk_sp<SkData> data, SkBitmap* dst, SkColorType dstCT);

inline bool GetResourceAsBitmap(const char* resource, SkBitmap* dst) {
    return DecodeDataToBitmap(GetResourceAsData(resource), dst);
}

inline bool GetResourceAsBitmapWithColortype(const char* resource, SkBitmap* dst, SkColorType dstCT) {
    return DecodeDataToBitmapWithColorType(GetResourceAsData(resource), dst, dstCT);
}

inline sk_sp<SkImage> GetResourceAsImage(const char* resource) {
    return SkImages::DeferredFromEncodedData(GetResourceAsData(resource));
}

}  // namespace ToolUtils

#endif
