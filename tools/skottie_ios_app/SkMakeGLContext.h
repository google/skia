// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkMakeGLContext_DEFINED
#define SkMakeGLContext_DEFINED

#include "tools/skottie_ios_app/GrContextHolder.h"
#include <memory>

class GrContext;
struct GrGLInterface;

struct GrGLInterfaceRelease { void operator()(const GrGLInterface*); };
using GrGLInterfaceHolder = std::unique_ptr<const GrGLInterface, GrGLInterfaceRelease>;

GrGLInterfaceHolder SkMakeGLInterface();

GrContextHolder SkMakeGLContext(const GrGLInterface*);

#endif  // SkMakeGLContext_DEFINED
