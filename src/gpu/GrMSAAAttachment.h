/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMSAAAttachment_DEFINED
#define GrMSAAAttachment_DEFINED

#include "src/gpu/GrSurface.h"

class GrMSAAAttachment : public GrSurface {
public:
    /**
     * Retrieves the number of samples of the MSAA surface
     */
    int sampleCnt() const { return fSampleCnt; }

    static void ComputeScratchKey(const GrCaps& caps,
                                  const GrBackendFormat& format,
                                  SkISize dimensions,
                                  int sampleCnt,
                                  GrProtected,
                                  GrScratchKey* key);

protected:
    GrMSAAAttachment(GrGpu* gpu, SkISize dimensions, int sampleCnt, GrProtected isProtected)
            : GrSurface(gpu, dimensions, isProtected)
            , fSampleCnt(sampleCnt) {}

private:
    const char* getResourceType() const override { return "MSAA"; }

    int fSampleCnt;
};

#endif

