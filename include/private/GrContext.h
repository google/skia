/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GrRecordingContext.h"

/**
 * This deprecated class is being merged into GrDirectContext and removed.
 * Do not add new subclasses, new API, or attempt to instantiate one.
 * If new API requires direct GPU access, add it to GrDirectContext.
 * Otherwise, add it to GrRecordingContext.
 */
class SK_API GrContext : public GrRecordingContext {
public:
    ~GrContext() override;

protected:
    GrContext(sk_sp<GrContextThreadSafeProxy>);

private:
    using INHERITED = GrRecordingContext;
};

#endif
