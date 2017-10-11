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

/**
A Skia GM is a single rendering test that can be executed on any Skia backend Canvas.
*/
namespace skiagm {
   class GM;
}

namespace sk_gpu_test {
    class GrContextFactory;
}

namespace gm_runner {

using GMFactory = skiagm::GM* (*)(void*);

enum class SkiaBackend {
    kGL,
    kGLES,
    kVulkan,
};

/**
This class initializes Skia and a GrContextFactory.
*/
struct SkiaContext {
    SkiaContext();
    ~SkiaContext();
    void resetContextFactory();
    std::unique_ptr<sk_gpu_test::GrContextFactory> fGrContextFactory;
};

bool BackendSupported(SkiaBackend, sk_gpu_test::GrContextFactory*);

/**
@return a list of all Skia GMs in lexicographic order.
*/
std::vector<GMFactory> GetGMFactories();

/**
@return a descriptive name for the GM.
*/
std::string GetGMName(GMFactory);
/**
@return a descriptive name for the backend.
*/
const char* GetBackendName(SkiaBackend);

/**
Execute the given GM on the given Skia backend.  Then copy the pixels into the
storage (overwriting existing contents of storage).

@return the rendered image.  Return a null ImageData on error.
*/
GMK_ImageData Evaluate(SkiaBackend,
                       GMFactory,
                       sk_gpu_test::GrContextFactory*,
                       std::vector<uint32_t>* storage);

}  // namespace gm_runner

#endif  // gm_runner_DEFINED
