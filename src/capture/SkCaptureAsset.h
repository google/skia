/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCaptureAsset_DEFINED
#define SkCaptureAsset_DEFINED

#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkPixelStorage.h"

#include <cstdint>

class SkCaptureAsset : public SkRefCnt {
public:
    enum class Type : uint32_t {
        kPicture = 0,
        kDerivedStorage = 1,
    };

    SkCaptureAsset() {}
    ~SkCaptureAsset() override = default;

    uint32_t fPixelStorageID = 0;
    uint32_t fContentID = 0;
    Type fType = Type::kPicture;
    sk_sp<SkPicture> fPicture;
    SkPixelStorage::Provenance fProvenance;
};

#endif // SkCaptureAsset_DEFINED
