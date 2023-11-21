/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnErrorChecker_DEFINED
#define skgpu_graphite_DawnErrorChecker_DEFINED

#include "src/base/SkEnumBitMask.h"

#include "src/gpu/graphite/dawn/DawnAsyncWait.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {

class DawnCaps;

enum class DawnErrorType : uint32_t {
    kNoError     = 0b00000000,
    kValidation  = 0b00000001,
    kOutOfMemory = 0b00000010,
    kInternal    = 0b00000100,
};
SK_MAKE_BITMASK_OPS(DawnErrorType);

// DawnErrorChecker immediately pushes error scopes for all known Dawn error filter types
// (Validation, OutOfMemory, Internal) upon construction and detects any errors that are
// reported within those scopes. Errors can be detected synchronously by either
//
//    1. calling `check()`, which returns false if any errors were reported, or
//    2. destroying the DawnErrorChecker instance, which asserts if any errors are reported
//       which weren't previously caught by calling `check()` directly.
//
class DawnErrorChecker {
public:
    explicit DawnErrorChecker(const DawnSharedContext*);
    ~DawnErrorChecker();

    SkEnumBitMask<DawnErrorType> popErrorScopes();

private:
    bool fArmed = true;
    const DawnSharedContext* fSharedContext;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnErrorChecker_DEFINED
