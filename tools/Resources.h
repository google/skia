/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Resources_DEFINED
#define Resources_DEFINED

#include "SkString.h"

class SkBitmap;
class SkImage;
class SkStreamAsset;
class SkTypeface;

SkString GetResourcePath(const char* resource = "");
void SetResourcePath(const char* );

bool GetResourceAsBitmap(const char* resource, SkBitmap* dst);
SkImage* GetResourceAsImage(const char* resource);
SkStreamAsset* GetResourceAsStream(const char* resource);
SkTypeface* GetResourceAsTypeface(const char* resource);

#endif  // Resources_DEFINED
