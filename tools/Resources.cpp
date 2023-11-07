/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/Resources.h"

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkDebug.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommandLineFlags.h"

#include <utility>

static DEFINE_string2(resourcePath, i, "resources",
                      "Directory with test resources: images, fonts, etc.");

sk_sp<SkData> (*gResourceFactory)(const char*) = nullptr;

SkString GetResourcePath(const char* resource) {
    return SkOSPath::Join(FLAGS_resourcePath[0], resource);
}

void SetResourcePath(const char* resource) {
    FLAGS_resourcePath.set(0, resource);
}

std::unique_ptr<SkStreamAsset> GetResourceAsStream(const char* resource, bool useFileStream) {
    if (useFileStream) {
        auto path = GetResourcePath(resource);
        return SkFILEStream::Make(path.c_str());
    } else {
        auto data = GetResourceAsData(resource);
        return data ? std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data)))
                    : nullptr;
    }
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
