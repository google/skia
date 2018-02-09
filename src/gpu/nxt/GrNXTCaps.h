/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTCaps_DEFINED
#define GrNXTCaps_DEFINED

#include "GrCaps.h"
#include "GrContextOptions.h"

class GrNXTCaps : public GrCaps {
public:
    GrNXTCaps(const GrContextOptions& contextOptions);

    int getSampleCount(int requestedCount, GrPixelConfig config) const override {
        return 0;
    }

    bool isConfigTexturable(GrPixelConfig config) const override {
        return false;
    }

    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override;

    bool isConfigCopyable(GrPixelConfig config) const override {
        return true;
    }

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override {
        return false;
    }

    bool validateBackendTexture(const GrBackendTexture&, SkColorType,
                                GrPixelConfig*) const override {
        return false;
    }
    bool validateBackendRenderTarget(const GrBackendRenderTarget&, SkColorType,
                                     GrPixelConfig*) const override {
        return false;
    }

    typedef GrCaps INHERITED;
};

#endif
