/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Resources_DEFINED
#define Resources_DEFINED

#include "SkRefCnt.h"
#include "SkString.h"

class SkBitmap;
class SkImage;
class SkStreamAsset;
class SkTypeface;

SkString GetResourcePath(const char* resource = "");
void SetResourcePath(const char* );

bool GetResourceAsBitmap(const char* resource, SkBitmap* dst);
sk_sp<SkImage> GetResourceAsImage(const char* resource);
SkStreamAsset* GetResourceAsStream(const char* resource);
sk_sp<SkTypeface> MakeResourceAsTypeface(const char* resource);

#endif  // Resources_DEFINED
