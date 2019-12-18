// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef GrContextHolder_DEFINED
#define GrContextHolder_DEFINED

#include <memory>

class GrContext;

// A struct to take ownership of a GrContext.
struct GrContextRelease { void operator()(GrContext*); };
using GrContextHolder = std::unique_ptr<GrContext, GrContextRelease>;

// Wrapper around GrContext::MakeGL
GrContextHolder SkMakeGLContext();
#endif  // GrContextHolder_DEFINED
