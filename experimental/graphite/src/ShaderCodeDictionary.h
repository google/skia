/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ShaderCodeDictionary_DEFINED
#define skgpu_ShaderCodeDictionary_DEFINED

#include <unordered_map>
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/UniquePaintParamsID.h"
#include "include/private/SkSpinlock.h"
#include "src/core/SkArenaAlloc.h"

namespace skgpu {

class ShaderCodeDictionary {
public:
    ShaderCodeDictionary();

    struct Entry {
    public:
        UniquePaintParamsID uniqueID() const {
            SkASSERT(fUniqueID.isValid());
            return fUniqueID;
        }
        Combination combo() const { return fCombo; }

    private:
        friend class ShaderCodeDictionary;

        Entry(Combination combo) : fCombo(combo) {}

        void setUniqueID(uint32_t newID) {
            SkASSERT(!fUniqueID.isValid());
            fUniqueID = UniquePaintParamsID(newID);
        }

        UniquePaintParamsID fUniqueID;
        // TODO: replace all uses of Combination with the variable length key
        Combination fCombo;
    };

    const Entry* findOrCreate(Combination) SK_EXCLUDES(fSpinLock);

    const Entry* lookup(UniquePaintParamsID) const SK_EXCLUDES(fSpinLock);

private:
    Entry* makeEntry(Combination combo);

    struct Hash {
        size_t operator()(Combination) const;
    };

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    std::unordered_map<Combination, Entry*, Hash> fHash SK_GUARDED_BY(fSpinLock);
    std::vector<Entry *> fEntryVector SK_GUARDED_BY(fSpinLock);

    SkArenaAlloc fArena{256};
};

} // namespace skgpu

#endif // skgpu_ShaderCodeDictionary_DEFINED
