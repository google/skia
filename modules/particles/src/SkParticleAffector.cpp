/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleAffector.h"

#include "include/core/SkContourMeasure.h"
#include "include/core/SkPath.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkRandom.h"
#include "include/utils/SkTextUtils.h"
#include "modules/particles/include/SkCurve.h"
#include "modules/particles/include/SkParticleData.h"
#include "src/core/SkMakeUnique.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLExternalValue.h"

// Exposes an SkCurve as an external, callable value. c(x) returns a float.
class SkCurveExternalValue : public SkSL::ExternalValue {
public:
    SkCurveExternalValue(const char* name, SkSL::Compiler& compiler, const SkCurve& curve)
            : INHERITED(name, *compiler.context().fFloat_Type)
            , fCompiler(compiler)
            , fCurve(curve)
            , fRandom(nullptr) { }

    void setRandom(SkRandom* random) { fRandom = random; }
    bool canCall() const override { return true; }
    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat_Type.get();
    }

    void call(int index, float* arguments, float* outReturn) override {
        *outReturn = fCurve.eval(*arguments, fRandom[index]);
    }

private:
    SkSL::Compiler& fCompiler;
    SkCurve fCurve;
    SkRandom* fRandom;
    typedef SkSL::ExternalValue INHERITED;
};

// Exposes an SkPath as an external, callable value. p(x) returns a float4 { pos.xy, normal.xy }
class SkPathExternalValue : public SkSL::ExternalValue {
public:
    SkPathExternalValue(const char* name, SkSL::Compiler& compiler, const SkPath& path)
            : INHERITED(name, *compiler.context().fFloat4_Type)
            , fCompiler(compiler)
            , fTotalLength(0) {
        SkContourMeasureIter iter(path, false);
        while (auto contour = iter.next()) {
            fContours.push_back(contour);
            fTotalLength += contour->length();
        }
    }

    bool canCall() const override { return true; }
    int callParameterCount() const override { return 1; }
    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat_Type.get();
    }

    void call(int index, float* arguments, float* outReturn) override {
        SkScalar len = fTotalLength * arguments[0];
        int idx = 0;
        while (idx < fContours.count() && len > fContours[idx]->length()) {
            len -= fContours[idx++]->length();
        }
        SkVector localXAxis;
        if (!fContours[idx]->getPosTan(len, (SkPoint*)outReturn, &localXAxis)) {
            outReturn[0] = outReturn[1] = 0.0f;
            localXAxis = { 1, 0 };
        }
        outReturn[2] = localXAxis.fY;
        outReturn[3] = -localXAxis.fX;
    }

private:
    SkSL::Compiler& fCompiler;
    SkScalar fTotalLength;
    SkTArray<sk_sp<SkContourMeasure>> fContours;
    typedef SkSL::ExternalValue INHERITED;
};
