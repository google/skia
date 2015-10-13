//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Query.h: Defines the gl::Query class

#ifndef LIBANGLE_QUERY_H_
#define LIBANGLE_QUERY_H_

#include "libANGLE/Error.h"
#include "libANGLE/RefCountObject.h"

#include "common/angleutils.h"

#include "angle_gl.h"

namespace rx
{
class QueryImpl;
}

namespace gl
{

class Query : public RefCountObject
{
  public:
    Query(rx::QueryImpl *impl, GLuint id);
    virtual ~Query();

    Error begin();
    Error end();

    Error getResult(GLuint *params);
    Error isResultAvailable(GLuint *available);

    GLenum getType() const;

  private:
    rx::QueryImpl *mQuery;
};

}

#endif   // LIBANGLE_QUERY_H_
