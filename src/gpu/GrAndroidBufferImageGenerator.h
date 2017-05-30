/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrAndroidBufferImageGenerator_DEFINED
#define GrAndroidBufferImageGenerator_DEFINED

#include "SkImageGenerator.h"

#include <android/hardware_buffer.h>

class GrAndroidBufferImageGenerator : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> Make(AHardwareBuffer*, SkAlphaType,
                                                  sk_sp<SkColorSpace>);

    ~GrAndroidBufferImageGenerator();

protected:

    bool onIsValid(GrContext*) const override;

#if SK_SUPPORT_GPU
    bool onCanGenerateTexture() const override { return true; }
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&,
                                            const SkIPoint&) override;
#endif

private:
    GrAndroidBufferImageGenerator(const SkImageInfo&, AHardwareBuffer*, SkAlphaType);

    static void deleteImageTexture(void* ctx);

    AHardwareBuffer* fGraphicBuffer;
    SkAlphaType fAlphaType;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrAndroidBufferImageGenerator_DEFINED
