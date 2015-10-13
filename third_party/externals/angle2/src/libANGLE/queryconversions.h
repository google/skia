//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// queryconversions.h: Declaration of state query cast conversions

#ifndef LIBANGLE_QUERY_CONVERSIONS_H_
#define LIBANGLE_QUERY_CONVERSIONS_H_

#include "angle_gl.h"
#include "common/angleutils.h"

namespace gl
{
class Context;

// Helper class for converting a GL type to a GLenum:
// We can't use CastStateValueEnum generally, because of GLboolean + GLubyte overlap.
// We restrict our use to CastStateValue, where it eliminates duplicate parameters.

template <typename GLType>
struct GLTypeToGLenum
{
    static GLenum value;
};

// The GL state query API types are: bool, int, uint, float, int64
template <typename QueryT>
void CastStateValues(Context *context, GLenum nativeType, GLenum pname,
                     unsigned int numParams, QueryT *outParams);

}

#endif  // LIBANGLE_QUERY_CONVERSIONS_H_
