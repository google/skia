/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef gm_runner_DEFINED
#define gm_runner_DEFINED

#include <memory>
#include <string>
#include <vector>

#include "gm_knowledge.h"

namespace skiagm {
   class GM;
}

namespace gm_runner {

struct SkiaBackend;

using GMFactory = skiagm::GM* (*)(void*);

class SkiaContext {
public:
    SkiaContext();
    ~SkiaContext();
    // These backend pointers are only valid for as long as the SkiaContext
    // is is scope.
    std::vector<SkiaBackend*> backends() const;

private:
    struct Impl;
    std::unique_ptr<Impl> fImpl;
};

std::vector<GMFactory> GetGMs();

std::string GetName(const SkiaBackend*);

GMK_ImageData Evaluate(SkiaBackend*, GMFactory, std::vector<uint32_t>* storage);

std::string GetName(GMFactory);

}  // namespace gm_runner

#endif  // gm_runner_DEFINED
