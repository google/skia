/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skqp_asset_manager_DEFINED
#define skqp_asset_manager_DEFINED

#include <memory>

class SkStreamAsset;

namespace skqp {
class AssetManager {
public:
    virtual ~AssetManager() {}
    virtual std::unique_ptr<SkStreamAsset> open(const char* path) = 0;
};
}  // namespace skqp
#endif  // skqp_asset_manager_DEFINED
