/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleBinding.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkContourMeasure.h"
#include "include/core/SkImage.h"
#include "include/core/SkPath.h"
#include "include/private/SkTPin.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkTextUtils.h"
#include "modules/particles/include/SkReflected.h"
#include "modules/skresources/include/SkResources.h"
#include "src/sksl/SkSLCompiler.h"

void SkParticleBinding::visitFields(SkFieldVisitor* v) {
    v->visit("Name", fName);
}

struct SkPathContours {
    SkScalar fTotalLength;
    SkTArray<sk_sp<SkContourMeasure>> fContours;

    void rebuild(const SkPath& path) {
        fTotalLength = 0;
        fContours.reset();

        SkContourMeasureIter iter(path, false);
        while (auto contour = iter.next()) {
            fContours.push_back(contour);
            fTotalLength += contour->length();
        }
    }
};

// Exposes an SkPath as an external, callable value. p(x) returns a float4 { pos.xy, normal.xy }
class SkPathExternalValue : public SkParticleExternalValue {
public:
    SkPathExternalValue(const char* name, SkSL::Compiler& compiler, const SkPathContours* path)
        : SkParticleExternalValue(name, compiler, *compiler.context().fTypes.fFloat4)
        , fPath(path) { }

    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fTypes.fFloat.get();
    }

    void call(int index, float* arguments, float* outReturn) const override {
        SkScalar len = fPath->fTotalLength * arguments[0];
        int idx = 0;
        while (idx < fPath->fContours.count() - 1 && len > fPath->fContours[idx]->length()) {
            len -= fPath->fContours[idx++]->length();
        }
        SkVector localXAxis;
        if (idx >= fPath->fContours.count() ||
            !fPath->fContours[idx]->getPosTan(len, (SkPoint*)outReturn, &localXAxis)) {
            outReturn[0] = outReturn[1] = 0.0f;
            localXAxis = { 1, 0 };
        }
        outReturn[2] = localXAxis.fY;
        outReturn[3] = -localXAxis.fX;
    }

private:
    const SkPathContours* fPath;
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

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
            new SkPathExternalValue(fName.c_str(), compiler, &fContours));
    }

    void prepare(const skresources::ResourceProvider* resourceProvider) override {
        if (auto pathData = resourceProvider->load(fPathPath.c_str(), fPathName.c_str())) {
            SkPath path;
            if (0 != path.readFromMemory(pathData->data(), pathData->size())) {
                fContours.rebuild(path);
            }
        }
    }

private:
    SkString fPathPath;
    SkString fPathName;

    // Cached
    SkPathContours fContours;
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

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
            new SkPathExternalValue(fName.c_str(), compiler, &fContours));
    }

    void prepare(const skresources::ResourceProvider*) override {
        if (fText.isEmpty()) {
            return;
        }

        SkFont font(nullptr, fFontSize);
        SkPath path;
        SkTextUtils::GetPath(fText.c_str(), fText.size(), SkTextEncoding::kUTF8, 0, 0, font, &path);
        fContours.rebuild(path);
    }

private:
    SkString fText;
    SkScalar fFontSize;

    // Cached
    SkPathContours fContours;
};

// Exposes an SkBitmap as an external, callable value. p(xy) returns a float4
class SkBitmapExternalValue : public SkParticleExternalValue {
public:
    SkBitmapExternalValue(const char* name, SkSL::Compiler& compiler, const SkBitmap& bitmap)
            : SkParticleExternalValue(name, compiler, *compiler.context().fTypes.fFloat4)
            , fBitmap(bitmap) {
        SkASSERT(bitmap.colorType() == kRGBA_F32_SkColorType);
    }

    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fTypes.fFloat2.get();
    }

    void call(int index, float* arguments, float* outReturn) const override {
        int x = SkTPin(static_cast<int>(arguments[0] * fBitmap.width()), 0, fBitmap.width() - 1);
        int y = SkTPin(static_cast<int>(arguments[1] * fBitmap.height()), 0, fBitmap.height() - 1);
        float* p = static_cast<float*>(fBitmap.getAddr(x, y));
        memcpy(outReturn, p, 4 * sizeof(float));
    }

private:
    SkBitmap fBitmap;
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

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
            new SkBitmapExternalValue(fName.c_str(), compiler, fBitmap));
    }

    void prepare(const skresources::ResourceProvider* resourceProvider) override {
        if (auto asset = resourceProvider->loadImageAsset(fImagePath.c_str(), fImageName.c_str(),
                                                          nullptr)) {
            if (auto image = asset->getFrame(0)) {
                fBitmap.allocPixels(image->imageInfo().makeColorType(kRGBA_F32_SkColorType));
                image->readPixels(nullptr, fBitmap.pixmap(), 0, 0);
                return;
            }
        }

        fBitmap.allocPixels(SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kPremul_SkAlphaType));
        fBitmap.eraseColor(SK_ColorWHITE);
    }

private:
    SkString fImagePath;
    SkString fImageName;

    // Cached
    SkBitmap fBitmap;
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
