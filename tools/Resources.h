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

SkString GetResourcePath(const char* resource = "");
void SetResourcePath(const char* );

bool GetResourceAsBitmap(const char* resource, SkBitmap* dst);

#endif  // Resources_DEFINED
