/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERSPOOL
#define SKSL_MODIFIERSPOOL

#include <unordered_map>

namespace SkSL {

struct Modifiers;

/**
 * Stores a pool of Modifiers objects. Modifiers is fairly heavy, so to reduce IRNode's size we only
 * store a handle to the Modifiers inside of the node and keep the object itself in a ModifiersPool.
 */
class ModifiersPool {
public:
    class Handle {
    public:
        Handle() = default;

        Handle(const ModifiersPool* pool, int index)
            : fPool(pool)
            , fIndex(index) {}

        const Modifiers* operator->() const {
            return &fPool->fModifiers[fIndex];
        }

        const Modifiers& operator*() const {
            return fPool->fModifiers[fIndex];
        }

    private:
        const ModifiersPool* fPool;
        int fIndex;
    };

    bool empty() {
        return fModifiers.empty();
    }

    Handle handle(const Modifiers& modifiers) {
        SkASSERT(fModifiers.size() == fModifiersMap.size());
        int index;
        auto found = fModifiersMap.find(modifiers);
        if (found != fModifiersMap.end()) {
            index = found->second;
        } else {
            index = fModifiers.size();
            fModifiers.push_back(modifiers);
            fModifiersMap.insert({modifiers, index});
        }
        return Handle(this, index);
    }

private:
    std::vector<Modifiers> fModifiers;
    std::unordered_map<Modifiers, int> fModifiersMap;
};

} // namespace SkSL

#endif
