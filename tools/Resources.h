/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Resources_DEFINED
#define Resources_DEFINED

#include "include/core/SkData.h"  // IWYU pragma: keep
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"

#include <memory>
#include <string>

class SkStreamAsset;
class SkTypeface;

SkString GetResourcePath(const char* resource = "");

void SetResourcePath(const char*);

sk_sp<SkData> GetResourceAsData(const char* resource);

inline sk_sp<SkData> GetResourceAsData(const std::string& resource) {
    return GetResourceAsData(resource.c_str());
}

std::unique_ptr<SkStreamAsset> GetResourceAsStream(const char* resource,
                                                   bool useFileStream = false);

sk_sp<SkTypeface> MakeResourceAsTypeface(const char* resource, int ttcIndex = 0);

#endif  // Resources_DEFINED
