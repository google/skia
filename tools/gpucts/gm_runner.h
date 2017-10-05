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

namespace gm_runner {

/**
An opaque type that represents a Skia backend.
*/
struct SkiaBackend;

using GMFactory = skiagm::GM* (*)(void*);

/**
This class initializes Skia and a list of backends.
*/
class SkiaContext {
public:
    SkiaContext();
    ~SkiaContext();
    /*
    @return a vector of SkiaBackend* pointers.

    These backend pointers are only valid for as long as the SkiaContext
    is in scope.
    */
    std::vector<SkiaBackend*> backends() const;

private:
    struct Impl;
    std::unique_ptr<Impl> fImpl;
};

/**
@return a list of all Skia GMs in lexicographic order.
*/
std::vector<GMFactory> GetGMs();

/**
@return a descriptive name for the GM.
*/
std::string GetName(GMFactory);

/**
@return a descriptive name for the backend.
*/
std::string GetName(const SkiaBackend*);

/**
Execute the given GM on the given Skia backend.  Then copy the pixels into the
storage (overwriting existing contents of storage).

@return the rendered image.  Return a null ImageData on error.
*/
GMK_ImageData Evaluate(SkiaBackend*, GMFactory, std::vector<uint32_t>* storage);

}  // namespace gm_runner

#endif  // gm_runner_DEFINED
