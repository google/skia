//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TransformFeedbackD3D.cpp is a no-op implementation for both the D3D9 and D3D11 renderers.

#include "libANGLE/renderer/d3d/TransformFeedbackD3D.h"

namespace rx
{

TransformFeedbackD3D::TransformFeedbackD3D()
{
}

TransformFeedbackD3D::~TransformFeedbackD3D()
{
}

void TransformFeedbackD3D::begin(GLenum primitiveMode)
{
}

void TransformFeedbackD3D::end()
{
}

void TransformFeedbackD3D::pause()
{
}

void TransformFeedbackD3D::resume()
{
}

void TransformFeedbackD3D::bindGenericBuffer(const BindingPointer<gl::Buffer> &binding)
{
}

void TransformFeedbackD3D::bindIndexedBuffer(size_t index, const OffsetBindingPointer<gl::Buffer> &binding)
{
}

}
