/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSkSLFP_DEFINED
#define GrSkSLFP_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrFragmentProcessor.h"

#include <atomic>
#include <utility>
#include <vector>

class GrShaderCaps;
class SkData;
class SkRuntimeEffect;

#ifdef SK_DEBUG
// UNIFORM_TYPE allows C++ types to be mapped onto SkRuntimeEffect::Uniform::Type
template <typename T> struct GrFPUniformType {
    template <typename U> struct add_a_UNIFORM_TYPE_specialization_for {};
    static constexpr add_a_UNIFORM_TYPE_specialization_for<T> value = {};
};
#define UNIFORM_TYPE(T, E)                                                                         \
    template <> struct GrFPUniformType<T> {                                                        \
        static constexpr SkRuntimeEffect::Uniform::Type value = SkRuntimeEffect::Uniform::Type::E; \
    }

UNIFORM_TYPE(float,       kFloat);
UNIFORM_TYPE(SkV2,        kFloat2);
UNIFORM_TYPE(SkPMColor4f, kFloat4);
UNIFORM_TYPE(SkV4,        kFloat4);
UNIFORM_TYPE(int,         kInt);

#undef UNIFORM_TYPE
#endif

class GrSkSLFP : public GrFragmentProcessor {
public:
    /**
     * Creates a new fragment processor from an SkRuntimeEffect and a data blob containing values
     * for all of the 'uniform' variables in the SkSL source. The layout of the uniforms blob is
     * dictated by the SkRuntimeEffect.
     */
    static std::unique_ptr<GrSkSLFP> Make(sk_sp<SkRuntimeEffect> effect,
                                          const char* name,
                                          sk_sp<SkData> uniforms);

    const char* name() const override;

    void addChild(std::unique_ptr<GrFragmentProcessor> child);

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    /*
     * Constructs a GrSkSLFP from a series of name-value pairs, corresponding to the children and
     * uniform data members of the effect's SkSL.
     * The variable length args... must contain all of the children and uniforms expected.
     * Each individual argument must be preceded by a name that matches the SkSL name of the value
     * being set. For children, the next argument must be a std::unique_ptr<GrFragmentProcessor>.
     * For uniforms, the next argument must be data of the correct size and type.
     *
     * For example, given:
     *   uniform shader input;
     *   uniform float scale;
     *   uniform half2 pt;
     *   half4 main() { ... }
     *
     * A call to GrSkSLFP would be formatted like:
     *   std::unique_ptr<GrFragmentProcessor> child = ...;
     *   float scaleVal = ...;
     *   SkV2 ptVal = ...;
     *   auto fp = GrSkSLFP::Make(effect, "my_effect",
     *                            "input", std::move(child),
     *                            "scale", scaleVal,
     *                            "pt", ptVal);
     *
     * The uniforms must appear in the correct order, as must the children. Technically, the two
     * lists can be interleaved. In debug builds, the number, names, and sizes of all arguments are
     * checked with assertions. In release builds, all checks are elided. In either case, the
     * uniform data is directly copied into the footer allocated after the FP.
     */
    template <typename... Args>
    static std::unique_ptr<GrSkSLFP> Make(sk_sp<SkRuntimeEffect> effect,
                                          const char* name,
                                          Args&&... args) {
#ifdef SK_DEBUG
        checkArgs(effect->fUniforms.begin(),
                  effect->fUniforms.end(),
                  effect->fChildren.begin(),
                  effect->fChildren.end(),
                  std::forward<Args>(args)...);
#endif

        size_t uniformSize = effect->uniformSize();
        std::unique_ptr<GrSkSLFP> fp(new (uniformSize) GrSkSLFP(std::move(effect), name));
        fp->appendArgs(fp->uniformData(), std::forward<Args>(args)...);
        return fp;
    }

private:
    GrSkSLFP(sk_sp<SkRuntimeEffect> effect, const char* name);
    GrSkSLFP(sk_sp<SkRuntimeEffect> effect, const char* name, sk_sp<SkData> uniforms);
    GrSkSLFP(const GrSkSLFP& other);

    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f&) const override;

    void* uniformData() const { return (void*)(this + 1); }

    // Helpers to attach variadic template args to a newly constructed FP:
    void appendArgs(void* ptr) {}
    template <typename... Args>
    void appendArgs(void* ptr,
                    const char* name,
                    std::unique_ptr<GrFragmentProcessor>&& child,
                    Args&&... remainder) {
        this->addChild(std::move(child));
        this->appendArgs(ptr, std::forward<Args>(remainder)...);
    }
    template <typename T, typename... Args>
    void appendArgs(void* ptr, const char* name, const T& val, Args&&... remainder) {
        memcpy(ptr, &val, sizeof(val));
        this->appendArgs(SkTAddOffset<void>(ptr, sizeof(val)), std::forward<Args>(remainder)...);
    }

#ifdef SK_DEBUG
    using child_iterator = std::vector<SkRuntimeEffect::Child>::const_iterator;
    using uniform_iterator = std::vector<SkRuntimeEffect::Uniform>::const_iterator;

    // Validates that all args passed to the template factory have the right names, sizes, and types
    static void checkArgs(uniform_iterator uIter,
                          uniform_iterator uEnd,
                          child_iterator cIter,
                          child_iterator cEnd) {
        SkASSERTF(uIter == uEnd, "Expected more uniforms, starting with '%s'", uIter->name.c_str());
        SkASSERTF(cIter == cEnd, "Expected more children, starting with '%s'", cIter->name.c_str());
    }
    template <typename... Args>
    static void checkArgs(uniform_iterator uIter,
                          uniform_iterator uEnd,
                          child_iterator cIter,
                          child_iterator cEnd,
                          const char* name,
                          std::unique_ptr<GrFragmentProcessor>&& child,
                          Args&&... remainder) {
        // NOTE: This function (necessarily) gets an rvalue reference to child, but deliberately
        // does not use it. We leave it intact, and our caller (Make) will pass another rvalue
        // reference to appendArgs, which will then move it to call addChild.
        SkASSERTF(cIter != cEnd, "Too many children, wasn't expecting '%s'", name);
        SkASSERTF(cIter->name.equals(name),
                  "Expected child '%s', got '%s' instead",
                  cIter->name.c_str(), name);
        checkArgs(uIter, uEnd, ++cIter, cEnd, std::forward<Args>(remainder)...);
    }
    template <typename T, typename... Args>
    static void checkArgs(uniform_iterator uIter,
                          uniform_iterator uEnd,
                          child_iterator cIter,
                          child_iterator cEnd,
                          const char* name,
                          const T& val,
                          Args&&... remainder) {
        SkASSERTF(uIter != uEnd, "Too many uniforms, wasn't expecting '%s'", name);
        SkASSERTF(uIter->name.equals(name),
                  "Expected uniform '%s', got '%s' instead",
                  uIter->name.c_str(), name);
        SkASSERTF(uIter->sizeInBytes() == sizeof(val),
                  "Expected uniform '%s' to be %zu bytes, got %zu instead",
                  name, uIter->sizeInBytes(), sizeof(val));
        SkASSERTF(GrFPUniformType<T>::value == uIter->type,
                  "Wrong type for uniform '%s'",
                  name);
        checkArgs(++uIter, uEnd, cIter, cEnd, std::forward<Args>(remainder)...);
    }
#endif

    sk_sp<SkRuntimeEffect> fEffect;
    const char*            fName;
    size_t                 fUniformSize;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;

    friend class GrGLSLSkSLFP;

    friend class GrSkSLFPFactory;
};

#endif
