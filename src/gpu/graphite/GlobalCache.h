/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GlobalCache_DEFINED
#define skgpu_graphite_GlobalCache_DEFINED

#include "include/core/SkRefCnt.h"

class SkShaderCodeDictionary;

namespace SkSL {
struct ShaderCaps;
}

namespace skgpu::graphite {

// TODO: This class needs to be thread safe. In the current version there is no thread safety and
// we need to go back and add protection around access to any of its memebers.
class GlobalCache : public SkRefCnt {
public:
    GlobalCache(const SkSL::ShaderCaps*);
    ~GlobalCache() override;

    SkShaderCodeDictionary* shaderCodeDictionary() const { return fShaderCodeDictionary.get(); }

private:
    std::unique_ptr<SkShaderCodeDictionary> fShaderCodeDictionary;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GlobalCache_DEFINED
