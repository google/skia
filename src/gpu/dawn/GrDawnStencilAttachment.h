/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrDawnStencil_DEFINED
#define GrDawnStencil_DEFINED

#include "src/gpu/GrStencilAttachment.h"

#include "dawn/dawncpp.h"

class GrDawnGpu;

class GrDawnStencilAttachment : public GrStencilAttachment {
public:
    static GrDawnStencilAttachment* Create(GrDawnGpu* gpu, int width, int height,
                                           int sampleCnt);

    ~GrDawnStencilAttachment() override;
    dawn::TextureView view() const { return fView; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    size_t onGpuMemorySize() const override;

    GrDawnStencilAttachment(GrDawnGpu* gpu, int width, int height, int bits, int samples,
                            dawn::Texture texture, dawn::TextureView view);

    GrDawnGpu* getDawnGpu() const;

    dawn::Texture fTexture;
    dawn::TextureView fView;

    typedef GrStencilAttachment INHERITED;
};

#endif
