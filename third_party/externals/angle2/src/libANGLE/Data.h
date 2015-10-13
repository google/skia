//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Data.h: Container class for all GL relevant state, caps and objects

#ifndef LIBANGLE_DATA_H_
#define LIBANGLE_DATA_H_

#include "libANGLE/State.h"

namespace gl
{

struct Data final
{
  public:
    Data(GLint clientVersion, const State &state, const Caps &caps,
         const TextureCapsMap &textureCaps, const Extensions &extensions,
         const ResourceManager *resourceManager);
    ~Data();

    Data(const Data &other);
    Data &operator=(const Data &other);

    GLint clientVersion;
    const State *state;
    const Caps *caps;
    const TextureCapsMap *textureCaps;
    const Extensions *extensions;
    const ResourceManager *resourceManager;
};

}

#endif // LIBANGLE_DATA_H_
