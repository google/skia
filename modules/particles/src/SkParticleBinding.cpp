/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleBinding.h"

#include "include/core/SkContourMeasure.h"
#include "include/core/SkPath.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkRandom.h"
#include "include/utils/SkTextUtils.h"
#include "modules/particles/include/SkCurve.h"
#include "modules/particles/include/SkReflected.h"
#include "src/sksl/SkSLCompiler.h"

void SkParticleBinding::visitFields(SkFieldVisitor* v) {
    v->visit("Name", fName);
}

// Exposes an SkCurve as an external, callable value. c(x) returns a float.
class SkCurveExternalValue : public SkParticleExternalValue {
public:
    SkCurveExternalValue(const char* name, SkSL::Compiler& compiler, const SkCurve& curve)
        : SkParticleExternalValue(name, compiler, *compiler.context().fFloat_Type)
        , fCurve(curve) { }

    bool canCall() const override { return true; }
    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat_Type.get();
    }

    void call(int index, float* arguments, float* outReturn) override {
        *outReturn = fCurve.eval(*arguments, fRandom[index]);
    }

private:
    SkCurve fCurve;
};

class SkCurveBinding : public SkParticleBinding {
public:
    SkCurveBinding(const char* name = "", const SkCurve& curve = 0.0f)
        : SkParticleBinding(name)
        , fCurve(curve) {}

    REFLECTED(SkCurveBinding, SkParticleBinding)

    void visitFields(SkFieldVisitor* v) override {
        SkParticleBinding::visitFields(v);
        v->visit("Curve", fCurve);
    }

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
                new SkCurveExternalValue(fName.c_str(), compiler, fCurve));
    }

private:
    SkCurve fCurve;
};

// Exposes an SkColorCurve as an external, callable value. c(x) returns a float4.
class SkColorCurveExternalValue : public SkParticleExternalValue {
public:
    SkColorCurveExternalValue(const char* name, SkSL::Compiler& compiler, const SkColorCurve& curve)
        : SkParticleExternalValue(name, compiler, *compiler.context().fFloat4_Type)
        , fCurve(curve) {
    }

    bool canCall() const override { return true; }
    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat_Type.get();
    }

    void call(int index, float* arguments, float* outReturn) override {
        SkColor4f color = fCurve.eval(*arguments, fRandom[index]);
        memcpy(outReturn, color.vec(), 4 * sizeof(float));
    }

private:
    SkColorCurve fCurve;
};

class SkColorCurveBinding : public SkParticleBinding {
public:
    SkColorCurveBinding(const char* name = "",
                        const SkColorCurve& curve = SkColor4f{ 1.0f, 1.0f, 1.0f, 1.0f })
        : SkParticleBinding(name)
        , fCurve(curve) {
    }

    REFLECTED(SkColorCurveBinding, SkParticleBinding)

        void visitFields(SkFieldVisitor* v) override {
        SkParticleBinding::visitFields(v);
        v->visit("Curve", fCurve);
    }

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
            new SkColorCurveExternalValue(fName.c_str(), compiler, fCurve));
    }

private:
    SkColorCurve fCurve;
};

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
        : SkParticleExternalValue(name, compiler, *compiler.context().fFloat4_Type)
        , fPath(path) { }

    bool canCall() const override { return true; }
    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat_Type.get();
    }

    void call(int index, float* arguments, float* outReturn) override {
        SkScalar len = fPath->fTotalLength * arguments[0];
        int idx = 0;
        while (idx < fPath->fContours.count() && len > fPath->fContours[idx]->length()) {
            len -= fPath->fContours[idx++]->length();
        }
        SkVector localXAxis;
        if (!fPath->fContours[idx]->getPosTan(len, (SkPoint*)outReturn, &localXAxis)) {
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
    SkPathBinding(const char* name = "", const char* path = "")
            : SkParticleBinding(name)
            , fPath(path) {
        this->rebuild();
    }

    REFLECTED(SkPathBinding, SkParticleBinding)

    void visitFields(SkFieldVisitor* v) override {
        SkString oldPath = fPath;

        SkParticleBinding::visitFields(v);
        v->visit("Path", fPath);

        if (fPath != oldPath) {
            this->rebuild();
        }
    }

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
            new SkPathExternalValue(fName.c_str(), compiler, &fContours));
    }

private:
    SkString fPath;

    void rebuild() {
        SkPath path;
        if (SkParsePath::FromSVGString(fPath.c_str(), &path)) {
            fContours.rebuild(path);
        }
    }

    // Cached
    SkPathContours fContours;
};

class SkTextBinding : public SkParticleBinding {
public:
    SkTextBinding(const char* name = "", const char* text = "", SkScalar fontSize = 96)
            : SkParticleBinding(name)
            , fText(text)
            , fFontSize(fontSize) {
        this->rebuild();
    }

    REFLECTED(SkTextBinding, SkParticleBinding)

    void visitFields(SkFieldVisitor* v) override {
        SkString oldText = fText;
        SkScalar oldSize = fFontSize;

        SkParticleBinding::visitFields(v);
        v->visit("Text", fText);
        v->visit("FontSize", fFontSize);

        if (fText != oldText || fFontSize != oldSize) {
            this->rebuild();
        }
    }

    std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler& compiler) override {
        return std::unique_ptr<SkParticleExternalValue>(
            new SkPathExternalValue(fName.c_str(), compiler, &fContours));
    }

private:
    SkString fText;
    SkScalar fFontSize;

    void rebuild() {
        if (fText.isEmpty()) {
            return;
        }

        SkFont font(nullptr, fFontSize);
        SkPath path;
        SkTextUtils::GetPath(fText.c_str(), fText.size(), SkTextEncoding::kUTF8, 0, 0, font, &path);
        fContours.rebuild(path);
    }

    // Cached
    SkPathContours fContours;
};

sk_sp<SkParticleBinding> SkParticleBinding::MakeCurve(const char* name, const SkCurve& curve) {
    return sk_sp<SkParticleBinding>(new SkCurveBinding(name, curve));
}

sk_sp<SkParticleBinding> SkParticleBinding::MakeColorCurve(const char* name,
                                                           const SkColorCurve& curve) {
    return sk_sp<SkParticleBinding>(new SkColorCurveBinding(name, curve));
}

sk_sp<SkParticleBinding> SkParticleBinding::MakePathBinding(const char* name, const char* path) {
    return sk_sp<SkParticleBinding>(new SkPathBinding(name, path));
}

void SkParticleBinding::RegisterBindingTypes() {
    REGISTER_REFLECTED(SkParticleBinding);
    REGISTER_REFLECTED(SkCurveBinding);
    REGISTER_REFLECTED(SkColorCurveBinding);
    REGISTER_REFLECTED(SkPathBinding);
    REGISTER_REFLECTED(SkTextBinding);
}
