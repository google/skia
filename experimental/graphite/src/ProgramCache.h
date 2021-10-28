/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ProgramCache_DEFINED
#define skgpu_ProgramCache_DEFINED

#include <unordered_map>
#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "include/core/SkRefCnt.h"

namespace skgpu {

class ProgramCache {
public:
    static constexpr uint32_t kInvalidProgramID = 0;

    // TODO: this is a bit underspecified. It still needs the rendering technique info.
    // Additionally, it still needs an entry point to generate the text of the program.
    class ProgramInfo : public SkRefCnt {
    public:
        ProgramInfo(Combination c);
        ~ProgramInfo() override;

        uint32_t id() const { return fID; }
        Combination combo() const { return fCombination; }

    private:
        const uint32_t    fID;
        const Combination fCombination;
        // TODO: store the rendering technique info from Chris here
    };

    // TODO: we need the rendering technique info from Chris for this look up
    sk_sp<ProgramInfo> findOrCreateProgram(Combination);

    size_t count() const { return fPrograms.size(); }

private:
    struct Hash {
        size_t operator()(Combination) const;
    };

    std::unordered_map<Combination, sk_sp<ProgramInfo>, Hash> fPrograms;
};

} // namespace skgpu

#endif // skgpu_ProgramCache_DEFINED
