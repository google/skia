/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_ComputePipelineDesc_DEFINED
#define skgpu_graphite_ComputePipelineDesc_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/SkOpts_spi.h"

#include <array>
#include <string>

namespace skgpu::graphite {

/**
 * ComputePipelineDesc represents the state needed to create a backend specific ComputePipeline.
 */
class ComputePipelineDesc {
public:
    ComputePipelineDesc() = default;

    SkSpan<const uint32_t> asKey() const {
        return SkSpan(reinterpret_cast<const uint32_t*>(fName.data()),
                      this->keySize() / sizeof(uint32_t));
    }

    bool operator==(const ComputePipelineDesc& that) const { return this->fName == that.fName; }

    bool operator!=(const ComputePipelineDesc& other) const { return !(*this == other); }

    // TODO(b/240604614): Until we have a more sophisticated way to dynamically construct compute
    // shader programs, this currently directly takes the entire program SkSL text as input. The
    // caching scheme is based entirely on the name and could be improved.
    const std::string& sksl() const { return fSkSL; }
    const std::string& name() const { return fName; }

    void setProgram(std::string sksl, std::string name) {
        SkASSERT(!sksl.empty());
        SkASSERT(name.size() >= sizeof(uint32_t));

        fSkSL = std::move(sksl);
        fName = std::move(name);
    }

    struct Hash {
        uint32_t operator()(const ComputePipelineDesc& desc) const {
            return SkOpts::hash_fn(desc.fName.data(), desc.keySize(), 0);
        }
    };

private:
    size_t keySize() const {
        size_t nameLength = fName.length();
        return nameLength - (nameLength % sizeof(uint32_t));
    }

    std::string fSkSL;
    std::string fName;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputePipelineDesc_DEFINED
