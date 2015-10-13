//
// Copyright (c) 2012-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderExecutable.cpp: Implements a class to contain D3D shader executable
// implementation details.

#include "libANGLE/renderer/d3d/ShaderExecutableD3D.h"

#include "common/angleutils.h"

namespace rx
{

ShaderExecutableD3D::ShaderExecutableD3D(const void *function, size_t length)
    : mFunctionBuffer(length)
{
    memcpy(mFunctionBuffer.data(), function, length);
}

ShaderExecutableD3D::~ShaderExecutableD3D()
{
}

const uint8_t *ShaderExecutableD3D::getFunction() const
{
    return mFunctionBuffer.data();
}

size_t ShaderExecutableD3D::getLength() const
{
    return mFunctionBuffer.size();
}

const std::string &ShaderExecutableD3D::getDebugInfo() const
{
    return mDebugInfo;
}

void ShaderExecutableD3D::appendDebugInfo(const std::string &info)
{
    mDebugInfo += info;
}


UniformStorageD3D::UniformStorageD3D(size_t initialSize) : mSize(initialSize)
{
}

UniformStorageD3D::~UniformStorageD3D()
{
}

size_t UniformStorageD3D::size() const
{
    return mSize;
}

}
