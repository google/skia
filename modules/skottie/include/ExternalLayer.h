/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieExternalLayer_DEFINED
#define SkottieExternalLayer_DEFINED

#include "include/core/SkRefCnt.h"

class SkCanvas;
struct SkSize;

namespace skottie {

/**
 * Interface for externally-rendered layers.
 */
class ExternalLayer : public SkRefCnt {
public:
    /** Render layer content into the given canvas.
     *
     * @param canvas  Destination canvas
     * @param t       Time in seconds, relative to the layer in-point (start time)
     */
    virtual void render(SkCanvas* canvas, double t) = 0;
};

/**
 * Interface for intercepting pre-composed layer creation.
 *
 * Embedders can register interceptors with animation builders, to substitute target layers
 * with arbitrary/externally-controlled content (see ExternalLayer above).
 */
class PrecompInterceptor : public SkRefCnt {
public:
    /**
     * Invoked at animation build time, for each precomp layer.
     *
     * @param id    The target composition ID (usually assigned automatically by BM: comp_0, ...)
     * @param name  The name of the precomp layer (by default it matches the target comp name,
     *              but can be changed in AE)
     * @param size  Lottie-specified precomp layer size
     * @return      An ExternalLayer implementation (to be used instead of the actual Lottie file
     *              content), or nullptr (to use the Lottie file content).
     */
    virtual sk_sp<ExternalLayer> onLoadPrecomp(const char id[],
                                               const char name[],
                                               const SkSize& size) = 0;
};

}  // namespace skottie

#endif // SkottieExternalLayer_DEFINED
