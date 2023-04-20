/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFilterColorProgram_DEFINED
#define SkFilterColorProgram_DEFINED

#include "include/private/SkColorData.h"
#include "src/core/SkVM.h"

#include <functional>
#include <memory>
#include <vector>

class SkRuntimeEffect;

#if defined(SK_ENABLE_SKSL)
#if defined(SK_ENABLE_SKVM)

/**
 * Runtime effects are often long lived & cached. Individual color filters or FPs created from them
 * and are often short-lived. However, color filters and FPs may need to operate on a single color
 * (on the CPU). This may be done at the paint level (eg, filter the paint color), or as part of
 * FP tree analysis.
 *
 * SkFilterColorProgram is an skvm program representing a (color filter) SkRuntimeEffect. It can
 * process a single color, without knowing the details of a particular instance (uniform values or
 * children).
 */
class SkFilterColorProgram {
public:
    static std::unique_ptr<SkFilterColorProgram> Make(const SkRuntimeEffect* effect);

    SkPMColor4f eval(const SkPMColor4f& inColor,
                     const void* uniformData,
                     std::function<SkPMColor4f(int, SkPMColor4f)> evalChild) const;

private:
    struct SampleCall {
        enum class Kind {
            kInputColor,  // eg child.eval(inputColor)
            kImmediate,   // eg child.eval(half4(1))
            kPrevious,    // eg child1.eval(child2.eval(...))
            kUniform,     // eg uniform half4 color; ... child.eval(color)
        };

        int  fChild;
        Kind fKind;
        union {
            SkPMColor4f fImm;       // for kImmediate
            int         fPrevious;  // for kPrevious
            int         fOffset;    // for kUniform
        };
    };

    SkFilterColorProgram(skvm::Program program, std::vector<SampleCall> sampleCalls);

    skvm::Program           fProgram;
    std::vector<SampleCall> fSampleCalls;
};

#else  // !defined(SK_ENABLE_SKVM)

// SkRP does not use SkFilterColorProgram; this stub implementation can be removed post-SkVM.
class SkFilterColorProgram {
public:
    static std::unique_ptr<SkFilterColorProgram> Make(const SkRuntimeEffect*) { return nullptr; }
};

#endif  // SK_ENABLE_SKVM
#endif  // SK_ENABLE_SKSL
#endif  // SkFilterColorProgram_DEFINED
