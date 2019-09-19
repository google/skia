/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleBinding_DEFINED
#define SkParticleBinding_DEFINED

#include "include/core/SkString.h"
#include "modules/particles/include/SkReflected.h"
#include "src/sksl/SkSLExternalValue.h"

#include <memory>

struct SkCurve;
struct SkColorCurve;
class SkParticleEffect;
class SkParticleEffectParams;
class SkRandom;

namespace SkSL {
    class Compiler;
}

class SkParticleExternalValue : public SkSL::ExternalValue {
public:
    SkParticleExternalValue(const char* name, SkSL::Compiler& compiler, const SkSL::Type& type)
        : SkSL::ExternalValue(name, type)
        , fCompiler(compiler)
        , fRandom(nullptr)
        , fEffect(nullptr) {}

    void setRandom(SkRandom* random) { fRandom = random; }
    void setEffect(SkParticleEffect* effect) { fEffect = effect; }

protected:
    SkSL::Compiler&   fCompiler;

    SkRandom*         fRandom;
    SkParticleEffect* fEffect;
};

class SkParticleBinding : public SkReflected {
public:
    SkParticleBinding(const char* name = "name") : fName(name) {}

    REFLECTED_ABSTRACT(SkParticleBinding, SkReflected)

    void visitFields(SkFieldVisitor* v) override;
    virtual std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler&) = 0;

    static void RegisterBindingTypes();

    /*
     * All SkParticleBinding objects expose a particular native object to an effect's SkSL code.
     * In all cases, the 'name' is the symbol that will be used to access the object from the SkSL.
     * Each binding is a callable object, so the SkSL name behaves like a function. The behavior of
     * each kind of binding is described below.
     */

    // Binds an SkCurve to an effect's SkSL. The curve is a one-dimensional function, described
    // in SkCurve.h. It is called in the SkSL as 'name(t)', and returns a single float value.
    static sk_sp<SkParticleBinding> MakeCurve(const char* name, const SkCurve& curve);

    // Binds an SkColorCurve to an effect's SkSL. The curve is a one-dimensional, function,
    // described in SkCurve.h. It is called in the SkSL as 'name(t)', and returns a float4 value.
    static sk_sp<SkParticleBinding> MakeColorCurve(const char* name, const SkColorCurve& curve);

    // Binds an SkPath to an effect's SkSL. The path is specified using SVG syntax. It is called
    // in the SkSL as 'name(t)'. 't' is a normalized distance along the path. This returns a float4
    // value, containing the position in .xy, and the normal in .zw.
    static sk_sp<SkParticleBinding> MakePathBinding(const char* name, const char* path);

    static sk_sp<SkParticleBinding> MakeEffectBinding(const char* name,
                                                      sk_sp<SkParticleEffectParams> effect);

protected:
    SkString fName;
};

#endif // SkParticleBinding_DEFINED
