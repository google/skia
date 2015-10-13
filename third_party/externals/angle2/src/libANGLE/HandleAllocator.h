//
// Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HandleAllocator.h: Defines the gl::HandleAllocator class, which is used to
// allocate GL handles.

#ifndef LIBANGLE_HANDLEALLOCATOR_H_
#define LIBANGLE_HANDLEALLOCATOR_H_

#include "common/angleutils.h"

#include "angle_gl.h"

#include <stack>

namespace gl
{

class HandleAllocator final : angle::NonCopyable
{
  public:
    // Maximum handle = MAX_UINT-1
    HandleAllocator();
    // Specify maximum handle value
    HandleAllocator(GLuint maximumHandleValue);

    ~HandleAllocator();

    void setBaseHandle(GLuint value);

    GLuint allocate();
    void release(GLuint handle);
    void reserve(GLuint handle);

  private:
    GLuint mBaseValue;
    GLuint mNextValue;
    typedef std::vector<GLuint> HandleList;
    HandleList mFreeValues;

    struct HandleRange
    {
        HandleRange(GLuint beginIn, GLuint endIn) : begin(beginIn), end(endIn) {}

        GLuint begin;
        GLuint end;
    };

    struct HandleRangeComparator;

    // The freelist consists of never-allocated handles, stored
    // as ranges, and handles that were previously allocated and
    // released, stored in a stack.
    std::vector<HandleRange> mUnallocatedList;
    std::vector<GLuint> mReleasedList;
};

}

#endif   // LIBANGLE_HANDLEALLOCATOR_H_
