/*
* Copyright 2021 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrMtlFramebuffer_DEFINED
#define GrMtlFramebuffer_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "include/private/GrTypesPriv.h"

class GrMtlAttachment;

class GrMtlFramebuffer : public SkRefCnt {
public:
    static sk_sp<const GrMtlFramebuffer> Make(GrMtlAttachment* colorAttachment,
                                              GrMtlAttachment* resolveAttachment,
                                              GrMtlAttachment* stencilAttachment);

    GrMtlAttachment* colorAttachment() { return fColorAttachment.get(); }
    GrMtlAttachment* resolveAttachment() { return fResolveAttachment.get(); }
    GrMtlAttachment* stencilAttachment() { return fStencilAttachment.get(); }

private:
    GrMtlFramebuffer(sk_sp<GrMtlAttachment> colorAttachment,
                     sk_sp<GrMtlAttachment> resolveAttachment,
                     sk_sp<GrMtlAttachment> stencilAttachment);

    ~GrMtlFramebuffer() override {}

    sk_sp<GrMtlAttachment> fColorAttachment;
    sk_sp<GrMtlAttachment> fResolveAttachment;
    sk_sp<GrMtlAttachment> fStencilAttachment;
};

#endif
