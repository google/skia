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
#include "src/core/SkPaintParamsKey.h"

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
        const SkPaintParamsKey& paintParamsKey() const { return fPaintParamsKey; }

    private:
        friend class ShaderCodeDictionary;

        Entry(const SkPaintParamsKey& paintParamsKey) : fPaintParamsKey(paintParamsKey) {}

        void setUniqueID(uint32_t newID) {
            SkASSERT(!fUniqueID.isValid());
            fUniqueID = UniquePaintParamsID(newID);
        }

        UniquePaintParamsID fUniqueID;    // fixed-size (uint32_t) unique ID assigned to a key
        SkPaintParamsKey fPaintParamsKey; // variable-length paint key descriptor
    };

    const Entry* findOrCreate(const SkPaintParamsKey&) SK_EXCLUDES(fSpinLock);

    const Entry* lookup(UniquePaintParamsID) const SK_EXCLUDES(fSpinLock);

private:
    Entry* makeEntry(const SkPaintParamsKey&);

    struct Hash {
        size_t operator()(const SkPaintParamsKey&) const;
    };

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    std::unordered_map<SkPaintParamsKey, Entry*, Hash> fHash SK_GUARDED_BY(fSpinLock);
    std::vector<Entry*> fEntryVector SK_GUARDED_BY(fSpinLock);

    SkArenaAlloc fArena{256};
};

} // namespace skgpu

#endif // skgpu_ShaderCodeDictionary_DEFINED
