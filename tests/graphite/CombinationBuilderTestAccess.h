/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CombinationBuilderTestAccess_DEFINED
#define CombinationBuilderTestAccess_DEFINED

#include "include/core/SkCombinationBuilder.h"

class CombinationBuilderTestAccess {
public:
    static int NumCombinations(SkCombinationBuilder* builder) {
        return builder->numCombinations();
    }
#ifdef SK_DEBUG
    static int Epoch(const SkCombinationBuilder& builder) {
        return builder.epoch();
    }
    static int Epoch(const SkCombinationOption& option) {
        return option.epoch();
    }
#endif
};

#endif // CombinationBuilderTestAccess_DEFINED
