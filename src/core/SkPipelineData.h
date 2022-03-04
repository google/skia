/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPipelineData_DEFINED
#define SkPipelineData_DEFINED

#include <vector>
#include "include/core/SkRefCnt.h"
#include "src/core/SkUniformData.h"

class SkUniformData;

// TODO: The current plan for fixing uniform padding is for the SkPipelineData to hold a
// persistent uniformManager. A stretch goal for this system would be for this combination
// to accumulate all the uniforms and then rearrange them to minimize padding. This would,
// obviously, vastly complicate uniform accumulation.
class SkPipelineData {
public:
    SkPipelineData() = default;
    SkPipelineData(sk_sp<SkUniformData> initial);

    void add(sk_sp<SkUniformData>);

    bool empty() const { return fUniformData.empty(); }
    size_t totalSize() const;  // TODO: cache this?
    int count() const;         // TODO: cache this?

    bool operator==(const SkPipelineData&) const;
    bool operator!=(const SkPipelineData& other) const { return !(*this == other);  }
    size_t hash() const;

    using container = std::vector<sk_sp<SkUniformData>>;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

    inline iterator begin() noexcept { return fUniformData.begin(); }
    inline const_iterator cbegin() const noexcept { return fUniformData.cbegin(); }
    inline iterator end() noexcept { return fUniformData.end(); }
    inline const_iterator cend() const noexcept { return fUniformData.cend(); }

private:
    // TODO: SkUniformData should be held uniquely
    std::vector<sk_sp<SkUniformData>> fUniformData;
};

#endif // SkPipelineData_DEFINED
