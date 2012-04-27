
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSoftwarePathRenderer.h"


bool GrSoftwarePathRenderer::canDrawPath(const SkPath& path,
                                         GrPathFill fill,
                                         const GrDrawTarget* target,
                                         bool antiAlias) const {
    if (!antiAlias) {
        // TODO: the SW renderer can also handle non-AA paths
        return false;
    }

    // TODO: set to true when filled out
    return false;
}

bool GrSoftwarePathRenderer::onDrawPath(const SkPath& path,
                                        GrPathFill fill,
                                        const GrVec* translate,
                                        GrDrawTarget* target,
                                        GrDrawState::StageMask stageMask,
                                        bool antiAlias) {

    // TODO: move onDrawPath routine (& its helpers) here from GrContext.cpp
    return false;
}

