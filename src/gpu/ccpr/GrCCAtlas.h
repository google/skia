/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCAtlas_DEFINED
#define GrCCAtlas_DEFINED

#include "src/gpu/GrDynamicAtlas.h"
#include "src/gpu/GrTBlockList.h"

class GrCCCachedAtlas;

/**
 * GrDynamicAtlas with CCPR caching capabilities.
 */
class GrCCAtlas : public GrDynamicAtlas {
public:
    // This struct encapsulates the minimum and desired requirements for an atlas, as well as an
    // approximate number of pixels to help select a good initial size.
    struct Specs {
        int fMaxPreferredTextureSize = 0;
        int fMinTextureSize = 0;
        int fMinWidth = 0;  // If there are 100 20x10 paths, this should be 20.
        int fMinHeight = 0;  // If there are 100 20x10 paths, this should be 10.
        int fApproxNumPixels = 0;

        // Add space for a rect in the desired atlas specs.
        void accountForSpace(int width, int height);
    };

    static sk_sp<GrTextureProxy> MakeLazyAtlasProxy(LazyInstantiateAtlasCallback&& callback,
                                                    const GrCaps& caps,
                                                    GrSurfaceProxy::UseAllocator useAllocator) {
        return GrDynamicAtlas::MakeLazyAtlasProxy(std::move(callback),
                                                  GrColorType::kAlpha_8,
                                                  InternalMultisample::kYes,
                                                  caps,
                                                  useAllocator);
    }

    GrCCAtlas(const Specs&, const GrCaps&);
    ~GrCCAtlas() override;
};

inline void GrCCAtlas::Specs::accountForSpace(int width, int height) {
    fMinWidth = std::max(width, fMinWidth);
    fMinHeight = std::max(height, fMinHeight);
    fApproxNumPixels += (width + kPadding) * (height + kPadding);
}

#endif
