/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tools/ResourceFactory.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"

static DEFINE_string2(resourcePath, i, "resources",
                      "Directory with test resources: images, fonts, etc.");

sk_sp<SkData> (*gResourceFactory)(const char*) = nullptr;

SkString GetResourcePath(const char* resource) {
    return SkOSPath::Join(FLAGS_resourcePath[0], resource);
}

void SetResourcePath(const char* resource) {
    FLAGS_resourcePath.set(0, resource);
}

bool DecodeDataToBitmap(sk_sp<SkData> data, SkBitmap* dst) {
    std::unique_ptr<SkImageGenerator> gen(SkImageGenerator::MakeFromEncoded(std::move(data)));
    return gen && dst->tryAllocPixels(gen->getInfo()) &&
        gen->getPixels(gen->getInfo().makeColorSpace(nullptr), dst->getPixels(), dst->rowBytes());
}

std::unique_ptr<SkStreamAsset> GetResourceAsStream(const char* resource) {
    auto data = GetResourceAsData(resource);
    return data ? std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data)))
                : nullptr;
}

sk_sp<SkData> GetResourceAsData(const char* resource) {
    if (sk_sp<SkData> data = gResourceFactory
                           ? gResourceFactory(resource)
                           : SkData::MakeFromFileName(GetResourcePath(resource).c_str())) {
        return data;
    }
    SkDebugf("Resource \"%s\" not found.\n", GetResourcePath(resource).c_str());
    #ifdef SK_TOOLS_REQUIRE_RESOURCES
    SK_ABORT("missing resource");
    #endif
    return nullptr;
}

sk_sp<SkTypeface> MakeResourceAsTypeface(const char* resource, int ttcIndex) {
    return SkTypeface::MakeFromStream(GetResourceAsStream(resource), ttcIndex);
}
