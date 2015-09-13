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
    SkAutoTUnref<SkData> resourceData(SkData::NewFromFileName(resourcePath.c_str()));
    SkAutoTDelete<SkImageGenerator> gen(SkImageGenerator::NewFromEncoded(resourceData));
    return gen && gen->tryGenerateBitmap(dst);
}

SkImage* GetResourceAsImage(const char* resource) {
    SkString path = GetResourcePath(resource);
    SkAutoTUnref<SkData> resourceData(SkData::NewFromFileName(path.c_str()));
    return SkImage::NewFromEncoded(resourceData);
}

SkStreamAsset* GetResourceAsStream(const char* resource) {
    SkString resourcePath = GetResourcePath(resource);
    SkAutoTDelete<SkFILEStream> stream(new SkFILEStream(resourcePath.c_str()));
    if (stream->isValid()) {
        return stream.detach();
    } else {
        SkDebugf("Resource %s not found.\n", resource);
        return nullptr;
    }
}

SkTypeface* GetResourceAsTypeface(const char* resource) {
    SkAutoTDelete<SkStreamAsset> stream(GetResourceAsStream(resource));
    if (!stream) {
        return nullptr;
    }
    return SkTypeface::CreateFromStream(stream.detach());
}
