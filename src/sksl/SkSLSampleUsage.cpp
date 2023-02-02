/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLSampleUsage.h"

#include <algorithm>

namespace SkSL {

SampleUsage SampleUsage::merge(const SampleUsage& other) {
    // This function is only used in Analysis::MergeSampleUsageVisitor to determine the combined
    // SampleUsage for a child fp/shader/etc. We should never see matrix sampling here.
    SkASSERT(fKind != Kind::kUniformMatrix && other.fKind != Kind::kUniformMatrix);

    static_assert(Kind::kExplicit > Kind::kPassThrough);
    static_assert(Kind::kPassThrough > Kind::kNone);
    fKind = std::max(fKind, other.fKind);

    return *this;
}

}  // namespace SkSL
