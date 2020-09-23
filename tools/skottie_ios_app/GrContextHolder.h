// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef GrContextHolder_DEFINED
#define GrContextHolder_DEFINED

#include <memory>

class GrDirectContext;

// A struct to take ownership of a GrDirectContext.
struct GrContextRelease { void operator()(GrDirectContext*); };
using GrContextHolder = std::unique_ptr<GrDirectContext, GrContextRelease>;

// Wrapper around GrDirectContext::MakeGL
GrContextHolder SkMakeGLContext();

#endif  // GrContextHolder_DEFINED
