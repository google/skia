/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "skqp_asset_manager.h"

#include "SkStream.h"
#include "SkString.h"
#include "../../src/core/SkStreamPriv.h"

std::unique_ptr<AssetManager> AssetManager::MakeStdAssetManager(const char* prefix) {
    struct StdAssetManager : public AssetManager {
        SkString fPrefix;
        StdAssetManager(const char* p) : fPrefix(p) {}
        std::unique_ptr<SkStreamAsset> open(const char* path) override {
            SkString fullPath = fPrefix.isEmpty()
                              ? SkString(path)
                              : SkStringPrintf("%s/%s", fPrefix.c_str(), path);
            return SkStream::MakeFromFile(fullPath.c_str());
        }
    };
    return std::unique_ptr<AssetManager>(new StdAssetManager(prefix));
}

bool AssetManager::exists(const char* path) {
    auto stream = this->open(path);
    return nullptr != stream;
}

bool AssetManager::copy(const char* path, const char* dst) {
    if (auto stream = this->open(path)) {
        SkFILEWStream wStream(dst);
        return wStream.isValid() && SkStreamCopy(&wStream, stream.get());
    }
    return false;
}
