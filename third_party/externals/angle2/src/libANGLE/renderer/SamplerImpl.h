//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplerImpl.h: Defines the abstract rx::SamplerImpl class.

#ifndef LIBANGLE_RENDERER_SAMPLERIMPL_H_
#define LIBANGLE_RENDERER_SAMPLERIMPL_H_

#include "common/angleutils.h"

namespace rx
{

class SamplerImpl : public angle::NonCopyable
{
  public:
    SamplerImpl() {}
    virtual ~SamplerImpl() {}
};
}

#endif  // LIBANGLE_RENDERER_SAMPLERIMPL_H_
