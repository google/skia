/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPerOpsTaskPaths_DEFINED
#define GrCCPerOpsTaskPaths_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/ccpr/GrCCClipPath.h"

#include <map>

class GrCCPerFlushResources;

/**
 * Tracks all the CCPR paths in a given opsTask that will be drawn when it flushes.
 */
// DDL TODO: given the usage pattern in DDL mode, this could probably be non-atomic refcounting.
struct GrCCPerOpsTaskPaths : public SkRefCnt {
    std::map<uint32_t, GrCCClipPath> fClipPaths;
};

#endif
