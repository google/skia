/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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

SkString GetResourcePath(const char* resource) {
    return SkOSPath::Join(FLAGS_resourcePath[0], resource);
}

void SetResourcePath(const char* resource) {
    FLAGS_resourcePath.set(0, resource);
}

bool GetResourceAsBitmap(const char* resource, SkBitmap* dst) {
    SkString resourcePath = GetResourcePath(resource);
    sk_sp<SkData> resourceData(SkData::MakeFromFileName(resourcePath.c_str()));
    std::unique_ptr<SkImageGenerator> gen(SkImageGenerator::MakeFromEncoded(resourceData));
    if (!gen) {
        return false;
    }
    return dst->tryAllocPixels(gen->getInfo()) &&
        gen->getPixels(gen->getInfo().makeColorSpace(nullptr), dst->getPixels(), dst->rowBytes(),
                       nullptr);
}

sk_sp<SkImage> GetResourceAsImage(const char* resource) {
    SkString path = GetResourcePath(resource);
    sk_sp<SkData> resourceData(SkData::MakeFromFileName(path.c_str()));
    return SkImage::MakeFromEncoded(resourceData);
}

SkStreamAsset* GetResourceAsStream(const char* resource) {
    SkString resourcePath = GetResourcePath(resource);
    std::unique_ptr<SkFILEStream> stream(new SkFILEStream(resourcePath.c_str()));
    if (!stream->isValid()) {
        SkDebugf("Resource %s not found.\n", resource);
        return nullptr;
    }
    return stream.release();
}

sk_sp<SkData> GetResourceAsData(const char* resource) {
    SkString resourcePath = GetResourcePath(resource);
    return SkData::MakeFromFileName(resourcePath.c_str());
}

sk_sp<SkTypeface> MakeResourceAsTypeface(const char* resource) {
    std::unique_ptr<SkStreamAsset> stream(GetResourceAsStream(resource));
    if (!stream) {
        return nullptr;
    }
    return SkTypeface::MakeFromStream(stream.release());
}
