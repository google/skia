
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClipMaskCache.h"

GrClipMaskCache::GrClipMaskCache()
    : fContext(NULL)
    , fStack(sizeof(GrClipStackFrame)) {
    // We need an initial frame to capture the clip state prior to
    // any pushes
    SkNEW_PLACEMENT(fStack.push_back(), GrClipStackFrame);
}

void GrClipMaskCache::push() {
    SkNEW_PLACEMENT(fStack.push_back(), GrClipStackFrame);
}

