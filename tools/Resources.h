/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Resources_DEFINED
#define Resources_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkString.h"
#include "modules/skresources/include/SkResources.h"
#include <unordered_map>

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

// A simple ResourceProvider implementing base functionality for loading images and skotties
class TestingResourceProvider : public skresources::ResourceProvider {
public:
    TestingResourceProvider() {}
    sk_sp<SkData> load(const char resource_path[], const char resource_name[]) const override;
    sk_sp<skresources::ImageAsset> loadImageAsset(const char resource_path[],
                                                  const char resource_name[],
                                                  const char /*resource_id*/[]) const override;
    void addPath(const char resource_name[], const SkPath& path);

private:
    std::unordered_map<std::string, sk_sp<SkData>> fResources;
};

#endif  // Resources_DEFINED
