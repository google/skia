/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleBinding.h"

#include "include/core/SkContourMeasure.h"
#include "include/core/SkImage.h"
#include "include/core/SkPath.h"
#include "include/private/SkTPin.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkTextUtils.h"
#include "modules/particles/include/SkReflected.h"
#include "modules/skresources/include/SkResources.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkVM.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/SkSLCompiler.h"

void SkParticleBinding::visitFields(SkFieldVisitor* v) {
    v->visit("Name", fName);
}

namespace {
    struct PosNrm { SkPoint pos; SkVector nrm; };
    using LinearizedPath = std::vector<PosNrm>;
}  // namespace

static LinearizedPath linearize_path(const SkPath& path) {
    LinearizedPath lin;
    SkContourMeasureIter iter(path, false);
    while (auto contour = iter.next()) {
        for (SkScalar x = 0; x < contour->length(); x++) {
            SkPoint pos;
            SkVector tan;
            SkAssertResult(contour->getPosTan(x, &pos, &tan));
            lin.push_back({pos, {tan.fY, -tan.fX}});
        }
    }
    return lin;
}

// Exposes an SkPath as an external, callable function. p(x) returns a float4 { pos.xy, normal.xy }
class SkPathExternalFunction : public SkParticleExternalFunction {
public:
    SkPathExternalFunction(const char* name,
                           SkSL::Compiler& compiler,
                           const LinearizedPath& path,
                           skvm::Uniforms* uniforms,
                           SkArenaAlloc* alloc)
            : SkParticleExternalFunction(
                      name, compiler, *compiler.context().fTypes.fFloat4, uniforms, alloc)
            , fPath(path) {}

    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fTypes.fFloat.get();
    }

    void call(skvm::Builder* builder,
              skvm::F32* arguments,
              skvm::F32* outResult,
              skvm::I32 mask) const override {
        if (fPath.empty()) {
            return;
        }

        skvm::Uniform ptr = fUniforms->pushPtr(fPath.data());
        skvm::I32 index = trunc(clamp(arguments[0] * fPath.size(), 0, fPath.size() - 1));

        outResult[0] = builder->gatherF(ptr, (index<<2)+0);
        outResult[1] = builder->gatherF(ptr, (index<<2)+1);
        outResult[2] = builder->gatherF(ptr, (index<<2)+2);
        outResult[3] = builder->gatherF(ptr, (index<<2)+3);
    }

private:
    const LinearizedPath& fPath;
};

class SkPathBinding : public SkParticleBinding {
public:
    SkPathBinding(const char* name = "", const char* pathPath = "", const char* pathName = "")
            : SkParticleBinding(name)
            , fPathPath(pathPath)
            , fPathName(pathName) {}

    REFLECTED(SkPathBinding, SkParticleBinding)

    void visitFields(SkFieldVisitor* v) override {
        SkParticleBinding::visitFields(v);
        v->visit("PathPath", fPathPath);
        v->visit("PathName", fPathName);
    }

    std::unique_ptr<SkParticleExternalFunction> toFunction(SkSL::Compiler& compiler,
                                                           skvm::Uniforms* uniforms,
                                                           SkArenaAlloc* alloc) override {
        return std::make_unique<SkPathExternalFunction>(fName.c_str(), compiler, fData, uniforms,
                                                        alloc);
    }

    void prepare(const skresources::ResourceProvider* resourceProvider) override {
        if (auto pathData = resourceProvider->load(fPathPath.c_str(), fPathName.c_str())) {
            SkPath path;
            if (0 != path.readFromMemory(pathData->data(), pathData->size())) {
                fData = linearize_path(path);
            }
        }
    }

private:
    SkString fPathPath;
    SkString fPathName;

    // Cached
    LinearizedPath fData;
};

class SkTextBinding : public SkParticleBinding {
public:
    SkTextBinding(const char* name = "", const char* text = "", SkScalar fontSize = 96)
            : SkParticleBinding(name)
            , fText(text)
            , fFontSize(fontSize) {}

    REFLECTED(SkTextBinding, SkParticleBinding)

    void visitFields(SkFieldVisitor* v) override {
        SkParticleBinding::visitFields(v);
        v->visit("Text", fText);
        v->visit("FontSize", fFontSize);
    }

    std::unique_ptr<SkParticleExternalFunction> toFunction(SkSL::Compiler& compiler,
                                                           skvm::Uniforms* uniforms,
                                                           SkArenaAlloc* alloc) override {
        return std::make_unique<SkPathExternalFunction>(fName.c_str(), compiler, fData, uniforms,
                                                        alloc);
    }

    void prepare(const skresources::ResourceProvider*) override {
        if (fText.isEmpty()) {
            return;
        }

        SkFont font(nullptr, fFontSize);
        SkPath path;
        SkTextUtils::GetPath(fText.c_str(), fText.size(), SkTextEncoding::kUTF8, 0, 0, font, &path);
        fData = linearize_path(path);
    }

private:
    SkString fText;
    SkScalar fFontSize;

    // Cached
    LinearizedPath fData;
};

// Exposes an SkShader as an external, callable function. p(xy) returns a float4
class SkShaderExternalFunction : public SkParticleExternalFunction {
public:
    SkShaderExternalFunction(const char* name,
                             SkSL::Compiler& compiler,
                             sk_sp<SkShader> shader,
                             skvm::Uniforms* uniforms,
                             SkArenaAlloc* alloc)
            : SkParticleExternalFunction(
                      name, compiler, *compiler.context().fTypes.fFloat4, uniforms, alloc)
            , fShader(std::move(shader)) {}

    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fTypes.fFloat2.get();
    }

    void call(skvm::Builder* builder,
              skvm::F32* arguments,
              skvm::F32* outResult,
              skvm::I32 mask) const override {
        skvm::Coord coord = {arguments[0], arguments[1]};
        skvm::F32 zero = builder->splat(0.0f);
        SkOverrideDeviceMatrixProvider matrixProvider(SkMatrix::I());
        SkColorInfo colorInfo(kRGBA_8888_SkColorType, kPremul_SkAlphaType, /*cs=*/nullptr);

        skvm::Color result = as_SB(fShader)->program(
                builder, /*device=*/coord, /*local=*/coord, /*paint=*/{zero, zero, zero, zero},
                matrixProvider, /*localM=*/nullptr, colorInfo, fUniforms,
                fAlloc);
        SkASSERT(result);
        outResult[0] = result.r;
        outResult[1] = result.g;
        outResult[2] = result.b;
        outResult[3] = result.a;
    }

private:
    sk_sp<SkShader> fShader;
};

class SkImageBinding : public SkParticleBinding {
public:
    SkImageBinding(const char* name = "", const char* imagePath = "", const char* imageName = "")
            : SkParticleBinding(name)
            , fImagePath(imagePath)
            , fImageName(imageName) {}

    REFLECTED(SkImageBinding, SkParticleBinding)

    void visitFields(SkFieldVisitor* v) override {
        SkParticleBinding::visitFields(v);
        v->visit("ImagePath", fImagePath);
        v->visit("ImageName", fImageName);
    }

    std::unique_ptr<SkParticleExternalFunction> toFunction(SkSL::Compiler& compiler,
                                                           skvm::Uniforms* uniforms,
                                                           SkArenaAlloc* alloc) override {
        return std::make_unique<SkShaderExternalFunction>(fName.c_str(), compiler, fShader,
                                                          uniforms, alloc);
    }

    void prepare(const skresources::ResourceProvider* resourceProvider) override {
        if (auto asset = resourceProvider->loadImageAsset(fImagePath.c_str(), fImageName.c_str(),
                                                          nullptr)) {
            if (auto image = asset->getFrame(0)) {
                SkMatrix normalize = SkMatrix::Scale(1.0f / image->width(), 1.0f / image->height());
                fShader = image->makeShader(SkSamplingOptions(SkFilterMode::kLinear), &normalize);
                return;
            }
        }

        fShader = SkShaders::Color(SK_ColorWHITE);
    }

private:
    SkString fImagePath;
    SkString fImageName;

    // Cached
    sk_sp<SkShader> fShader;
};

sk_sp<SkParticleBinding> SkParticleBinding::MakeImage(const char* name, const char* imagePath,
                                                      const char* imageName) {
    return sk_sp<SkParticleBinding>(new SkImageBinding(name, imagePath, imageName));
}

sk_sp<SkParticleBinding> SkParticleBinding::MakePath(const char* name, const char* pathPath,
                                                     const char* pathName) {
    return sk_sp<SkParticleBinding>(new SkPathBinding(name, pathPath, pathName));
}

void SkParticleBinding::RegisterBindingTypes() {
    REGISTER_REFLECTED(SkParticleBinding);
    REGISTER_REFLECTED(SkImageBinding);
    REGISTER_REFLECTED(SkPathBinding);
    REGISTER_REFLECTED(SkTextBinding);
}
