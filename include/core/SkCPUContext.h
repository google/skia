/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skcpu_Context_DEFINED
#define skcpu_Context_DEFINED

#include "include/private/base/SkAPI.h"

#include <memory>

namespace skcpu {
class Recorder;

class SK_API Context {
public:
    struct Options {};

    std::unique_ptr<Recorder> makeRecorder() const;

    static std::unique_ptr<const Context> Make(const Options&);
    static std::unique_ptr<const Context> Make();

protected:
    Context() = default;
};
}  // namespace skcpu

#endif
