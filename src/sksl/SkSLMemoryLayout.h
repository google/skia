/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKIASL_MEMORYLAYOUT
#define SKIASL_MEMORYLAYOUT

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class MemoryLayout {
public:
    enum Standard {
        k140_Standard,
        k430_Standard,
        kMetal_Standard
    };

    MemoryLayout(Standard std)
    : fStd(std) {}

    static size_t VectorAlignment(size_t componentSize, int columns);

    /**
     * Rounds up to the nearest multiple of 16 if in std140, otherwise returns the parameter
     * unchanged (std140 requires various things to be rounded up to the nearest multiple of 16,
     * std430 does not).
     */
    size_t roundUpIfNeeded(size_t raw) const;

    /**
     * Returns a type's required alignment when used as a standalone variable.
     */
    size_t alignment(const Type& type) const;

    /**
     * For matrices and arrays, returns the number of bytes from the start of one entry (row, in
     * the case of matrices) to the start of the next.
     */
    size_t stride(const Type& type) const;

    /**
     * Returns the size of a type in bytes.
     */
    size_t size(const Type& type) const;

    const Standard fStd;
};

} // namespace

#endif
