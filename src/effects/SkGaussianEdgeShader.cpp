/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGaussianEdgeShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

class SkArenaAlloc;

 /** \class SkGaussianEdgeShaderImpl
 This subclass of shader applies a Gaussian to shadow edge

 If the primitive supports an implicit distance to the edge, the radius of the blur is specified
 by r & g values of the color in 14.2 fixed point. For spot shadows, we increase the stroke width
 to set the shadow against the shape. This pad is specified by b, also in 6.2 fixed point.

 When not using implicit distance, then b in the input color represents the input to the
 blur function.
 */
class SkGaussianEdgeShaderImpl : public SkShader {
public:
    SkGaussianEdgeShaderImpl() {}

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkGaussianEdgeShaderImpl)

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onMakeContext(const ContextRec& rec, SkArenaAlloc* storage) const override {
        return nullptr;
    }
private:
    friend class SkGaussianEdgeShader;

    typedef SkShader INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "effects/GrBlurredEdgeFragmentProcessor.h"

////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkGaussianEdgeShaderImpl::asFragmentProcessor(const AsFPArgs&) const {
    return GrBlurredEdgeFP::Make(GrBlurredEdgeFP::kGaussian_Mode);
}

#endif

////////////////////////////////////////////////////////////////////////////

bool SkGaussianEdgeShaderImpl::isOpaque() const {
    return false;
}

////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkGaussianEdgeShaderImpl::toString(SkString* str) const {
    str->appendf("GaussianEdgeShader: ()");
}
#endif

sk_sp<SkFlattenable> SkGaussianEdgeShaderImpl::CreateProc(SkReadBuffer& buf) {
    return sk_make_sp<SkGaussianEdgeShaderImpl>();
}

void SkGaussianEdgeShaderImpl::flatten(SkWriteBuffer& buf) const {
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkGaussianEdgeShader::Make() {
    return sk_make_sp<SkGaussianEdgeShaderImpl>();
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkGaussianEdgeShader)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkGaussianEdgeShaderImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////
