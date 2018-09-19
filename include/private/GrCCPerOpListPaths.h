/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPerOpListPaths_DEFINED
#define GrCCPerOpListPaths_DEFINED

#include "SkArenaAlloc.h"
#include "SkRefCnt.h"
#include "SkTInternalLList.h"
#include "GrCCClipPath.h"

#include <map>

class GrCCDrawPathsOp;
class GrCCPerFlushResources;

/**
 * Tracks all the CCPR paths in a given opList that will be drawn when it flushes.
 */
// DDL TODO: given the usage pattern in DDL mode, this could probably be non-atomic refcounting.
struct GrCCPerOpListPaths : public SkRefCnt {
    SkTInternalLList<GrCCDrawPathsOp> fDrawOps;  // This class does not own these ops.
    std::map<uint32_t, GrCCClipPath> fClipPaths;
    SkSTArenaAlloc<10 * 1024> fAllocator{10 * 1024 * 2};
    sk_sp<const GrCCPerFlushResources> fFlushResources;
};

#endif
