/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrNXTStencil_DEFINED
#define GrNXTStencil_DEFINED

#include "GrStencilAttachment.h"

#include "nxt/nxtcpp.h"

class GrNXTGpu;

class GrNXTStencilAttachment : public GrStencilAttachment {
public:
    static GrNXTStencilAttachment* Create(GrNXTGpu* gpu, int width, int height,
                                          int sampleCnt);

    ~GrNXTStencilAttachment() override;
    nxt::TextureView view() const { return fView.Clone(); }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    size_t onGpuMemorySize() const override;

    GrNXTStencilAttachment(GrNXTGpu* gpu,
                           int width,
                           int height,
                           int bits,
                           int samples,
                           nxt::Texture texture,
                           nxt::TextureView view);

    GrNXTGpu* getNXTGpu() const;

    nxt::Texture fTexture;
    nxt::TextureView fView;

    typedef GrStencilAttachment INHERITED;
};

#endif
