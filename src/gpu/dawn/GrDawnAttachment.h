/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnAttachment_DEFINED
#define GrDawnAttachment_DEFINED

#include "src/gpu/GrAttachment.h"

#include "dawn/webgpu_cpp.h"

class GrDawnGpu;

class GrDawnAttachment : public GrAttachment {
public:
    static sk_sp<GrDawnAttachment> MakeStencil(GrDawnGpu* gpu,  SkISize dimensions, int sampleCnt);

    ~GrDawnAttachment() override;
    wgpu::TextureView view() const { return fView; }
    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeDawn(wgpu::TextureFormat::Depth24PlusStencil8);
    }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    GrDawnAttachment(GrDawnGpu* gpu,
                     SkISize dimensions,
                     UsageFlags supportedUsages,
                     int samples,
                     wgpu::Texture texture,
                     wgpu::TextureView view);

    GrDawnGpu* getDawnGpu() const;

    wgpu::Texture fTexture;
    wgpu::TextureView fView;

    using INHERITED = GrAttachment;
};

#endif
