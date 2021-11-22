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
#include "include/private/SkVx.h"
#include "src/gpu/GrFragmentProcessor.h"

#include <atomic>
#include <utility>
#include <vector>

struct GrShaderCaps;
class SkData;
class SkRuntimeEffect;

#ifdef SK_DEBUG
// UNIFORM_TYPE allows C++ types to be mapped onto SkRuntimeEffect::Uniform::Type
template <typename T> struct GrFPUniformType {
    template <typename U> struct add_a_UNIFORM_TYPE_specialization_for {};
    static constexpr add_a_UNIFORM_TYPE_specialization_for<T> value = {};
};
#define UNIFORM_TYPE(E, ...)                                                                       \
    template <> struct GrFPUniformType<__VA_ARGS__> {                                              \
        static constexpr SkRuntimeEffect::Uniform::Type value = SkRuntimeEffect::Uniform::Type::E; \
    };                                                                                             \
    template <> struct GrFPUniformType<SkSpan<__VA_ARGS__>> {                                      \
        static constexpr SkRuntimeEffect::Uniform::Type value = SkRuntimeEffect::Uniform::Type::E; \
    }

UNIFORM_TYPE(kFloat,    float);
UNIFORM_TYPE(kFloat2,   SkV2);
UNIFORM_TYPE(kFloat4,   SkPMColor4f);
UNIFORM_TYPE(kFloat4,   SkRect);
UNIFORM_TYPE(kFloat4,   SkV4);
UNIFORM_TYPE(kFloat4,   skvx::Vec<4, float>);
UNIFORM_TYPE(kFloat4x4, SkM44);
UNIFORM_TYPE(kInt,      int);

#undef UNIFORM_TYPE
#endif

class GrSkSLFP : public GrFragmentProcessor {
public:
    template <typename T> struct GrSpecializedUniform {
        bool specialize;
        T value;
    };
    template <typename T>
    static GrSpecializedUniform<T> Specialize(const T& value) {
        return {true, value};
    }
    template <typename T>
    static GrSpecializedUniform<T> SpecializeIf(bool condition, const T& value) {
        return {condition, value};
    }

    template <typename T> struct GrOptionalUniform {
        bool enabled;
        T value;
    };
    template <typename T>
    static GrOptionalUniform<T> When(bool condition, const T& value) {
        return {condition, value};
    }

    struct GrIgnoreOptFlags {
        std::unique_ptr<GrFragmentProcessor> child;
    };
    static GrIgnoreOptFlags IgnoreOptFlags(std::unique_ptr<GrFragmentProcessor> child) {
        return {std::move(child)};
    }

    enum class OptFlags : uint32_t {
        kNone                          = kNone_OptimizationFlags,
        kCompatibleWithCoverageAsAlpha = kCompatibleWithCoverageAsAlpha_OptimizationFlag,
        kPreservesOpaqueInput          = kPreservesOpaqueInput_OptimizationFlag,
        kAll                           = kCompatibleWithCoverageAsAlpha | kPreservesOpaqueInput,
    };

    /**
     * Both factories support a single 'input' FP, as well as a collection of other 'child' FPs.
     * The 'child' FPs correspond to the children declared in the effect's SkSL. The inputFP is
     * optional, and intended for instances that have color filter semantics. This is an implicit
     * child - if present, it's evaluated to produce the input color fed to the SkSL. Otherwise,
     * the SkSL receives this FP's input color directly.
     */

    /**
     * Creates a new fragment processor from an SkRuntimeEffect and a data blob containing values
     * for all of the 'uniform' variables in the SkSL source. The layout of the uniforms blob is
     * dictated by the SkRuntimeEffect.
     */
    static std::unique_ptr<GrSkSLFP> MakeWithData(
            sk_sp<SkRuntimeEffect> effect,
            const char* name,
            std::unique_ptr<GrFragmentProcessor> inputFP,
            std::unique_ptr<GrFragmentProcessor> destColorFP,
            sk_sp<SkData> uniforms,
            SkSpan<std::unique_ptr<GrFragmentProcessor>> childFPs);

    /*
     * Constructs a GrSkSLFP from a series of name-value pairs, corresponding to the children and
     * uniform data members of the effect's SkSL.
     * The variable length args... must contain all of the children and uniforms expected.
     * Each individual argument must be preceded by a name that matches the SkSL name of the value
     * being set. For children, the next argument must be a std::unique_ptr<GrFragmentProcessor>.
     * For uniforms, the next argument must be data of the correct size and type.
     *
     * For example, given:
     *   uniform shader child;
     *   uniform float scale;
     *   uniform half2 pt;
     *   half4 main() { ... }
     *
     * A call to GrSkSLFP would be formatted like:
     *   std::unique_ptr<GrFragmentProcessor> child = ...;
     *   float scaleVal = ...;
     *   SkV2 ptVal = ...;
     *   auto fp = GrSkSLFP::Make(effect, "my_effect", nullptr, GrSkSLFP::OptFlags::...,
     *                            "child", std::move(child),
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
                                          std::unique_ptr<GrFragmentProcessor> inputFP,
                                          OptFlags optFlags,
                                          Args&&... args) {
#ifdef SK_DEBUG
        checkArgs(effect->fUniforms.begin(),
                  effect->fUniforms.end(),
                  effect->fChildren.begin(),
                  effect->fChildren.end(),
                  std::forward<Args>(args)...);
#endif

        size_t uniformPayloadSize = UniformPayloadSize(effect.get());
        std::unique_ptr<GrSkSLFP> fp(new (uniformPayloadSize)
                                             GrSkSLFP(std::move(effect), name, optFlags));
        fp->appendArgs(fp->uniformData(), fp->uniformFlags(), std::forward<Args>(args)...);
        if (inputFP) {
            fp->setInput(std::move(inputFP));
        }
        return fp;
    }

    const char* name() const override { return fName; }
    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    class Impl;

    GrSkSLFP(sk_sp<SkRuntimeEffect> effect, const char* name, OptFlags optFlags);
    GrSkSLFP(const GrSkSLFP& other);

    void addChild(std::unique_ptr<GrFragmentProcessor> child, bool mergeOptFlags);
    void setInput(std::unique_ptr<GrFragmentProcessor> input);
    void setDestColorFP(std::unique_ptr<GrFragmentProcessor> destColorFP);

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f&) const override;

    // An instance of GrSkSLFP is always allocated with a payload immediately following the FP.
    // First the values of all the uniforms, and then a set of flags (one per uniform).
    static size_t UniformPayloadSize(const SkRuntimeEffect* effect) {
        return effect->uniformSize() + effect->uniforms().size() * sizeof(UniformFlags);
    }

    const uint8_t* uniformData() const { return reinterpret_cast<const uint8_t*>(this + 1); }
          uint8_t* uniformData()       { return reinterpret_cast<      uint8_t*>(this + 1); }

    enum UniformFlags : uint8_t {
        kSpecialize_Flag = 0x1,
    };

    const UniformFlags* uniformFlags() const {
        return reinterpret_cast<const UniformFlags*>(this->uniformData() + fUniformSize);
    }
    UniformFlags* uniformFlags() {
        return reinterpret_cast<UniformFlags*>(this->uniformData() + fUniformSize);
    }

    // Helpers to attach variadic template args to a newly constructed FP:

    void appendArgs(uint8_t* uniformDataPtr, UniformFlags* uniformFlagsPtr) {
        // Base case -- no more args to append, so we're done
    }
    template <typename... Args>
    void appendArgs(uint8_t* uniformDataPtr,
                    UniformFlags* uniformFlagsPtr,
                    const char* name,
                    std::unique_ptr<GrFragmentProcessor>&& child,
                    Args&&... remainder) {
        // Child FP case -- register the child, then continue processing the remaining arguments.
        // Children aren't "uniforms" here, so the data & flags pointers don't advance.
        this->addChild(std::move(child), /*mergeOptFlags=*/true);
        this->appendArgs(uniformDataPtr, uniformFlagsPtr, std::forward<Args>(remainder)...);
    }
    // As above, but we don't merge in the child's optimization flags
    template <typename... Args>
    void appendArgs(uint8_t* uniformDataPtr,
                    UniformFlags* uniformFlagsPtr,
                    const char* name,
                    GrIgnoreOptFlags&& child,
                    Args&&... remainder) {
        // Child FP case -- register the child, then continue processing the remaining arguments.
        // Children aren't "uniforms" here, so the data & flags pointers don't advance.
        this->addChild(std::move(child.child), /*mergeOptFlags=*/false);
        this->appendArgs(uniformDataPtr, uniformFlagsPtr, std::forward<Args>(remainder)...);
    }
    template <typename T, typename... Args>
    void appendArgs(uint8_t* uniformDataPtr,
                    UniformFlags* uniformFlagsPtr,
                    const char* name,
                    const GrSpecializedUniform<T>& val,
                    Args&&... remainder) {
        // Specialized uniform case -- This just handles the specialization logic. If we want to
        // specialize on this particular value, set the flag. Then, continue processing the actual
        // value (by just peeling off the wrapper). This lets our generic `const T&` case (below)
        // handle copying the data into our uniform block, and advancing the per-value uniform
        // data and flags pointers.
        if (val.specialize) {
            *uniformFlagsPtr = static_cast<UniformFlags>(*uniformFlagsPtr | kSpecialize_Flag);
        }
        this->appendArgs(
                uniformDataPtr, uniformFlagsPtr, name, val.value, std::forward<Args>(remainder)...);
    }
    template <typename T, typename... Args>
    void appendArgs(uint8_t* uniformDataPtr,
                    UniformFlags* uniformFlagsPtr,
                    const char* name,
                    const GrOptionalUniform<T>& val,
                    Args&&... remainder) {
        // Optional uniform case. Copy the data and advance pointers, but only if the uniform is
        // enabled. Then proceed as normal.
        if (val.enabled) {
            memcpy(uniformDataPtr, &val.value, sizeof(val.value));
            uniformDataPtr += sizeof(val.value);
            uniformFlagsPtr++;
        }

        this->appendArgs(uniformDataPtr, uniformFlagsPtr, std::forward<Args>(remainder)...);
    }
    template <typename T, typename... Args>
    void appendArgs(uint8_t* uniformDataPtr,
                    UniformFlags* uniformFlagsPtr,
                    const char* name,
                    SkSpan<T> val,
                    Args&&... remainder) {
        // Uniform array case -- We copy the supplied values into our uniform data area,
        // then advance our uniform data and flags pointers.
        memcpy(uniformDataPtr, val.data(), val.size_bytes());
        uniformDataPtr += val.size_bytes();
        uniformFlagsPtr++;
        this->appendArgs(uniformDataPtr, uniformFlagsPtr, std::forward<Args>(remainder)...);
    }
    template <typename T, typename... Args>
    void appendArgs(uint8_t* uniformDataPtr,
                    UniformFlags* uniformFlagsPtr,
                    const char* name,
                    const T& val,
                    Args&&... remainder) {
        // Raw uniform value case -- We copy the supplied value into our uniform data area,
        // then advance our uniform data and flags pointers.
        memcpy(uniformDataPtr, &val, sizeof(val));
        uniformDataPtr += sizeof(val);
        uniformFlagsPtr++;
        this->appendArgs(uniformDataPtr, uniformFlagsPtr, std::forward<Args>(remainder)...);
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
    static void checkOneChild(child_iterator cIter, child_iterator cEnd, const char* name) {
        SkASSERTF(cIter != cEnd, "Too many children, wasn't expecting '%s'", name);
        SkASSERTF(cIter->name.equals(name),
                  "Expected child '%s', got '%s' instead",
                  cIter->name.c_str(), name);
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
        checkOneChild(cIter, cEnd, name);
        checkArgs(uIter, uEnd, ++cIter, cEnd, std::forward<Args>(remainder)...);
    }
    template <typename... Args>
    static void checkArgs(uniform_iterator uIter,
                          uniform_iterator uEnd,
                          child_iterator cIter,
                          child_iterator cEnd,
                          const char* name,
                          GrIgnoreOptFlags&& child,
                          Args&&... remainder) {
        // NOTE: This function (necessarily) gets an rvalue reference to child, but deliberately
        // does not use it. We leave it intact, and our caller (Make) will pass another rvalue
        // reference to appendArgs, which will then move it to call addChild.
        checkOneChild(cIter, cEnd, name);
        checkArgs(uIter, uEnd, ++cIter, cEnd, std::forward<Args>(remainder)...);
    }
    template <typename T, typename... Args>
    static void checkArgs(uniform_iterator uIter,
                          uniform_iterator uEnd,
                          child_iterator cIter,
                          child_iterator cEnd,
                          const char* name,
                          const GrSpecializedUniform<T>& val,
                          Args&&... remainder) {
        static_assert(!std::is_array<T>::value);  // No specializing arrays
        checkArgs(uIter, uEnd, cIter, cEnd, name, val.value, std::forward<Args>(remainder)...);
    }
    template <typename T, typename... Args>
    static void checkArgs(uniform_iterator uIter,
                          uniform_iterator uEnd,
                          child_iterator cIter,
                          child_iterator cEnd,
                          const char* name,
                          const GrOptionalUniform<T>& val,
                          Args&&... remainder) {
        if (val.enabled) {
            checkArgs(uIter, uEnd, cIter, cEnd, name, val.value, std::forward<Args>(remainder)...);
        } else {
            checkArgs(uIter, uEnd, cIter, cEnd, std::forward<Args>(remainder)...);
        }
    }
    template <typename T>
    static void checkOneUniform(uniform_iterator uIter,
                                uniform_iterator uEnd,
                                const char* name,
                                const T* /*val*/,
                                size_t valSize) {
        SkASSERTF(uIter != uEnd, "Too many uniforms, wasn't expecting '%s'", name);
        SkASSERTF(uIter->name.equals(name),
                  "Expected uniform '%s', got '%s' instead",
                  uIter->name.c_str(), name);
        SkASSERTF(uIter->sizeInBytes() == valSize,
                  "Expected uniform '%s' to be %zu bytes, got %zu instead",
                  name, uIter->sizeInBytes(), valSize);
        SkASSERTF(GrFPUniformType<T>::value == uIter->type,
                  "Wrong type for uniform '%s'",
                  name);
    }
    template <typename T, typename... Args>
    static void checkArgs(uniform_iterator uIter,
                          uniform_iterator uEnd,
                          child_iterator cIter,
                          child_iterator cEnd,
                          const char* name,
                          SkSpan<T> val,
                          Args&&... remainder) {
        checkOneUniform(uIter, uEnd, name, val.data(), val.size_bytes());
        checkArgs(++uIter, uEnd, cIter, cEnd, std::forward<Args>(remainder)...);
    }
    template <typename T, typename... Args>
    static void checkArgs(uniform_iterator uIter,
                          uniform_iterator uEnd,
                          child_iterator cIter,
                          child_iterator cEnd,
                          const char* name,
                          const T& val,
                          Args&&... remainder) {
        checkOneUniform(uIter, uEnd, name, &val, sizeof(val));
        checkArgs(++uIter, uEnd, cIter, cEnd, std::forward<Args>(remainder)...);
    }
#endif

    sk_sp<SkRuntimeEffect> fEffect;
    const char*            fName;
    uint32_t               fUniformSize;
    int                    fInputChildIndex = -1;
    int                    fDestColorChildIndex = -1;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;

    friend class GrSkSLFPFactory;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrSkSLFP::OptFlags)

#endif
