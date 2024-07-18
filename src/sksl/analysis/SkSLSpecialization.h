/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPECIALIZATION
#define SKSL_SPECIALIZATION

#include "include/private/base/SkTArray.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLPosition.h"

#include <cstddef>
#include <functional>

namespace SkSL {

class FunctionDeclaration;
class Variable;
struct Program;

namespace Analysis {

// The current index of the specialization function being walked through, used to
// track what the proper specialization function call should be if walking through a
// specialized function call stack.
using SpecializationIndex = int;
static constexpr SpecializationIndex kUnspecialized = -1;

// Global uniforms used by a specialization, maps <function parameter, global uniform>
using SpecializedParameters = skia_private::THashMap<const Variable*, const Variable*>;
// The set of specializated implementations needed for a given function.
using Specializations = skia_private::TArray<SpecializedParameters>;
// The full set of all specializations required by the program.
using SpecializationMap = skia_private::THashMap<const FunctionDeclaration*, Specializations>;

// Our map of specialized calls is keyed on three things: the function containing the call, the
// specialization index of the parent function, and the textual position of the call within the
// function. Tracking the parent function and position is more roundabout than tracking a
// FunctionCall* directly, but it will still match an Expression that has been cloned or otherwise
// reconstructed, whereas a pointer won't.
struct SpecializedCallKey {
    struct Hash {
        size_t operator()(const SpecializedCallKey& entry) {
            return SkGoodHash()(entry.fParentFunction) ^
                   SkGoodHash()(entry.fParentSpecializationIndex) ^
                   SkGoodHash()(entry.fCallPosition.startOffset());
        }
    };

    bool operator==(const SpecializedCallKey& other) const {
        return fParentFunction == other.fParentFunction &&
               fParentSpecializationIndex == other.fParentSpecializationIndex &&
               fCallPosition == other.fCallPosition;
    }

    const FunctionDeclaration* fParentFunction;
    SpecializationIndex fParentSpecializationIndex;
    Position fCallPosition;
};

// The mapping of function calls and their inherited specialization to their corresponding
// specialization index in `Specializations`
using SpecializedCallMap = skia_private::THashMap<SpecializedCallKey,
                                                  SpecializationIndex,
                                                  SpecializedCallKey::Hash>;
struct SpecializationInfo {
    SpecializationMap fSpecializationMap;
    SpecializedCallMap fSpecializedCallMap;
};

// A function that returns true if the parameter variable fits the criteria
// to create a specialization.
using ParameterMatchesFn = std::function<bool(const Variable&)>;

// Finds functions that contain parameters that should be specialized on and writes the
// specialization info to the provided `SpecializationInfo`.
void FindFunctionsToSpecialize(const Program& program,
                               SpecializationInfo* info,
                               const ParameterMatchesFn& specializationFn);

}  // namespace Analysis
}  // namespace SkSL

#endif
