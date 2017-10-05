/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef gm_runner_DEFINED
#define gm_runner_DEFINED

#include <vector>
#include <string>

#include "gm_knowledge.h"

namespace skiagm {
   class GM;
}

namespace gm_runner {

struct SkiaBackend;

using GMFactory = skiagm::GM* (*)(void*);
using GMConsumer = void (*)(SkiaBackend*, GMFactory);

class GMRunner {
public:
    GMRunner();
    ~GMRunner();
    void ConsumeGMs(GMConsumer consumer);

private:
    struct Impl;
    std::unique_ptr<Impl> fImpl;
};


std::string GetName(const SkiaBackend*);

GMK_ImageData Evaluate(SkiaBackend*, GMFactory, std::vector<uint32_t>* storage);

std::string GetName(GMFactory);

}  // namespace gm_runner

#endif  // gm_runner_DEFINED
