/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/GlobalCache.h"

#include "include/private/SkShaderCodeDictionary.h"

namespace skgpu {

GlobalCache::GlobalCache() : fShaderCodeDictionary(std::make_unique<SkShaderCodeDictionary>()) {}

GlobalCache::~GlobalCache() {};

} // namespace skgpu

