/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "skqp_asset_manager.h"

#include "SkStream.h"
#include "../../src/core/SkStreamPriv.h"

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
