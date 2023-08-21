/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLocalMatrixShader_DEFINED
#define SkLocalMatrixShader_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkImageShader.h"
#include "src/shaders/SkShaderBase.h"

#include <type_traits>
#include <utility>

class SkArenaAlloc;
class SkImage;
class SkReadBuffer;
class SkWriteBuffer;
enum class SkTileMode;
struct SkStageRec;

class SkLocalMatrixShader final : public SkShaderBase {
public:
    template <typename T, typename... Args>
    static std::enable_if_t<std::is_base_of_v<SkShader, T>, sk_sp<SkShader>>
    MakeWrapped(const SkMatrix* localMatrix, Args&&... args) {
        auto t = sk_make_sp<T>(std::forward<Args>(args)...);
        bool isGraphiteImageShader = false;
        if (t->type() == SkShaderBase::ShaderType::kImage) {
            auto imgShader = static_cast<const SkImageShader*>(as_SB(t));
            auto imgBase = as_IB(imgShader->image());
            SkASSERT(imgBase);
            isGraphiteImageShader = imgBase->isGraphiteBacked();
        }
        // In Graphite we can safely handle a local matrix shader with identity by not emitting code
        // for it. Additionally, Graphite uses the local matrix shader to add the matrix for the
        // origin y-flip if needed. Thus we always emit the local matrix shader here so we can
        // connect the y-flip, but it doesn't hurt us if there is no flip.
        if ((!localMatrix || localMatrix->isIdentity()) && !isGraphiteImageShader) {
            return t;
        }

        return sk_make_sp<SkLocalMatrixShader>(sk_sp<SkShader>(std::move(t)),
                                               localMatrix ? *localMatrix : SkMatrix::I());
    }

    SkLocalMatrixShader(sk_sp<SkShader> wrapped, const SkMatrix& localMatrix)
            : fLocalMatrix(localMatrix), fWrappedShader(std::move(wrapped)) {}

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;
    ShaderType type() const override { return ShaderType::kLocalMatrix; }

    sk_sp<SkShader> makeAsALocalMatrixShader(SkMatrix* localMatrix) const override {
        if (localMatrix) {
            *localMatrix = fLocalMatrix;
        }
        return fWrappedShader;
    }

    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    sk_sp<SkShader> wrappedShader() const { return fWrappedShader; }

protected:
    void flatten(SkWriteBuffer&) const override;

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

    SkImage* onIsAImage(SkMatrix* matrix, SkTileMode* mode) const override;

    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkLocalMatrixShader)

    SkMatrix fLocalMatrix;
    sk_sp<SkShader> fWrappedShader;
};

/**
 *  Replaces the CTM when used. Created to support clipShaders, which have to be evaluated
 *  using the CTM that was present at the time they were specified (which may be different
 *  from the CTM at the time something is drawn through the clip.
 */
class SkCTMShader final : public SkShaderBase {
public:
    SkCTMShader(sk_sp<SkShader> proxy, const SkMatrix& ctm);

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;

    ShaderType type() const override { return ShaderType::kCTM; }

    const SkMatrix& ctm() const { return fCTM; }
    sk_sp<SkShader> proxyShader() const { return fProxyShader; }

protected:
    void flatten(SkWriteBuffer&) const override { SkASSERT(false); }

    bool appendStages(const SkStageRec& rec, const SkShaders::MatrixRec&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkCTMShader)

    sk_sp<SkShader> fProxyShader;
    SkMatrix fCTM;
};

#endif
