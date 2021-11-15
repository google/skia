/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ProgramCache_DEFINED
#define skgpu_ProgramCache_DEFINED

#include <unordered_map>
#include <string>
#include <vector>
#include "experimental/graphite/src/ContextUtils.h"
#include "include/core/SkRefCnt.h"

namespace skgpu {

class ProgramCache {
public:
    ProgramCache();

    static constexpr uint32_t kInvalidProgramID = 0;

    // TODO: this is a bit underspecified. It still needs the rendering technique info.
    // Additionally, it still needs an entry point to generate the text of the program.
    class ProgramInfo : public SkRefCnt {
    public:
        ProgramInfo(uint32_t uniqueID, Combination c);
        ~ProgramInfo() override;

        uint32_t id() const { return fID; }
        Combination combo() const { return fCombination; }

        std::string getMSL() const;

    private:
        const uint32_t    fID;
        const Combination fCombination;
        // TODO: store the rendering technique info from Chris here
    };

    // TODO: we need the rendering technique info from Chris for this look up
    sk_sp<ProgramInfo> findOrCreateProgram(Combination);

    sk_sp<ProgramInfo> lookup(uint32_t uniqueID);

    // The number of unique programs in the cache
    size_t count() const {
        SkASSERT(fProgramHash.size()+1 == fProgramVector.size());
        return fProgramHash.size();
    }

private:
    struct Hash {
        size_t operator()(Combination) const;
    };

    std::unordered_map<Combination, sk_sp<ProgramInfo>, Hash> fProgramHash;
    std::vector<sk_sp<ProgramInfo>> fProgramVector;
    // The ProgramInfo's unique ID is only unique w/in a Recorder _not_ globally
    uint32_t fNextUniqueID = 1;
};

} // namespace skgpu

#endif // skgpu_ProgramCache_DEFINED
