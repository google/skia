/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GlobalCache_DEFINED
#define skgpu_GlobalCache_DEFINED

#include "include/core/SkRefCnt.h"

class SkShaderCodeDictionary;

namespace skgpu {

// TODO: This class needs to be thread safe. In the current version there is no thread safety and
// we need to go back and add protection around access to any of its memebers.
class GlobalCache : public SkRefCnt {
public:
    GlobalCache();
    ~GlobalCache() override;

    SkShaderCodeDictionary* shaderCodeDictionary() const { return fShaderCodeDictionary.get(); }

private:
    std::unique_ptr<SkShaderCodeDictionary> fShaderCodeDictionary;
};

} // namespace skgpu

#endif // skgpu_GlobalCache_DEFINED
