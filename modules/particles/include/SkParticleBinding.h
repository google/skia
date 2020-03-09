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

class SkParticleEffect;
class SkParticleEffectParams;
class SkRandom;

namespace skresources {
    class ResourceProvider;
}

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
    virtual void prepare(const skresources::ResourceProvider*) = 0;

    static void RegisterBindingTypes();

    /*
     * All SkParticleBinding objects expose a particular native object to an effect's SkSL code.
     * In all cases, the 'name' is the symbol that will be used to access the object from the SkSL.
     * Each binding is a callable object, so the SkSL name behaves like a function. The behavior of
     * each kind of binding is described below.
     */

    // void name(loop) -- Creates an effect instance. Effect will loop if 'loop' is true, otherwise
    // it's a one-shot. The new effect inherits all properties from the calling effect or particle.
    static sk_sp<SkParticleBinding> MakeEffect(const char* name,
                                               sk_sp<SkParticleEffectParams> effect);

    // float4 name(xy) -- Fetches RGBA data from an image. 'xy' are normalized image coordinates.
    static sk_sp<SkParticleBinding> MakeImage(const char* name,
                                              const char* imagePath, const char* imageName);

    // float4 name(t) -- Fetches position and normal from an SkPath. 't' is the normalized distance
    // along the path. The return value contains position in .xy and normal in .zw.
    static sk_sp<SkParticleBinding> MakePath(const char* name,
                                             const char* pathPath, const char* pathName);

protected:
    SkString fName;
};

#endif // SkParticleBinding_DEFINED
