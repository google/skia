//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Version.h: Encapsulation of a GL version.

#ifndef LIBANGLE_VERSION_H_
#define LIBANGLE_VERSION_H_

#include <angle_gl.h>

namespace gl
{

struct Version
{
    Version();
    Version(GLuint major, GLuint minor);

    GLuint major;
    GLuint minor;
};

bool operator>=(const Version &a, const Version &b);
bool operator<(const Version &a, const Version &b);

}

#include "Version.inl"

#endif // LIBANGLE_VERSION_H_
