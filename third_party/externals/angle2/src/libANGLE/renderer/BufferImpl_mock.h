//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferImpl_mock.h: Defines a mock of the BufferImpl class.

#ifndef LIBANGLE_RENDERER_BUFFERIMPLMOCK_H_
#define LIBANGLE_RENDERER_BUFFERIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/renderer/BufferImpl.h"

namespace rx
{

class MockBufferImpl : public BufferImpl
{
  public:
    ~MockBufferImpl() { destructor(); }

    MOCK_METHOD3(setData, gl::Error(const void*, size_t, GLenum));
    MOCK_METHOD3(setSubData, gl::Error(const void*, size_t, size_t));
    MOCK_METHOD4(copySubData, gl::Error(BufferImpl *, GLintptr, GLintptr, GLsizeiptr));
    MOCK_METHOD2(map, gl::Error(GLenum, GLvoid **));
    MOCK_METHOD4(mapRange, gl::Error(size_t, size_t, GLbitfield, GLvoid **));
    MOCK_METHOD1(unmap, gl::Error(GLboolean *result));

    MOCK_METHOD5(getIndexRange, gl::Error(GLenum, size_t, size_t, bool, gl::IndexRange *));

    MOCK_METHOD0(destructor, void());
};

}

#endif // LIBANGLE_RENDERER_BUFFERIMPLMOCK_H_
