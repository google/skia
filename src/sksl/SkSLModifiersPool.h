/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODIFIERSPOOL
#define SKSL_MODIFIERSPOOL

#include "include/private/SkSLModifiers.h"

#include <unordered_set>

namespace SkSL {

/**
 * Deduplicates Modifiers objects and stores them in a shared pool. Modifiers are fairly heavy, and
 * tend to be reused a lot, so deduplication can be a significant win.
 */
class ModifiersPool {
public:
    const Modifiers* addToPool(const Modifiers& modifiers) {
        auto [iter, wasInserted] = fModifiersSet.insert(modifiers);
        return &*iter;
    }

private:
    std::unordered_set<Modifiers> fModifiersSet;
};

} // namespace SkSL

#endif
