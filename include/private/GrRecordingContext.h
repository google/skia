/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordingContext_DEFINED
#define GrRecordingContext_DEFINED

#include "GrImageContext.h"

class GrRecordingContextPriv;

class SK_API GrRecordingContext : public GrImageContext {
public:
    ~GrRecordingContext() override;

    // Provides access to functions that aren't part of the public API.
    GrRecordingContextPriv priv();
    const GrRecordingContextPriv priv() const;

protected:
    friend class GrRecordingContextPriv; // for hidden functions

    GrRecordingContext(GrBackendApi backend, uint32_t uniqueID);

    GrRecordingContext* asRecordingContext() override { return this; }

private:
    typedef GrImageContext INHERITED;
};

#endif
