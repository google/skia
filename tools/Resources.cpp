/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ResourceFactory.h"
#include "Resources.h"
#include "SkBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageGenerator.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkStream.h"
#include "SkTypeface.h"

DEFINE_string2(resourcePath, i, "resources", "Directory with test resources: images, fonts, etc.");

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
        gen->getPixels(gen->getInfo().makeColorSpace(nullptr), dst->getPixels(), dst->rowBytes(),
                       nullptr);
}

std::unique_ptr<SkStreamAsset> GetResourceAsStream(const char* resource) {
    auto data = GetResourceAsData(resource);
    return data ? std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data)))
                : nullptr;
}

sk_sp<SkData> GetResourceAsData(const char* resource) {
    if (gResourceFactory) {
        if (auto data = gResourceFactory(resource)) {
            return data;
        }
        SkDebugf("Resource \"%s\" not found.\n", resource);
        SK_ABORT("missing resource");
    }
    if (auto data = SkData::MakeFromFileName(GetResourcePath(resource).c_str())) {
        return data;
    }
    SkDebugf("Resource \"%s\" not found.\n", resource);
    return nullptr;
}

sk_sp<SkTypeface> MakeResourceAsTypeface(const char* resource) {
    std::unique_ptr<SkStreamAsset> stream(GetResourceAsStream(resource));
    if (!stream) {
        return nullptr;
    }
    return SkTypeface::MakeFromStream(stream.release());
}
