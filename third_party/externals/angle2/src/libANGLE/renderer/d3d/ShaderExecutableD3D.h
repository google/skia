//
// Copyright (c) 2012-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderExecutable.h: Defines a class to contain D3D shader executable
// implementation details.

#ifndef LIBANGLE_RENDERER_D3D_SHADEREXECUTABLED3D_H_
#define LIBANGLE_RENDERER_D3D_SHADEREXECUTABLED3D_H_

#include "common/debug.h"

#include <vector>
#include <cstdint>

namespace rx
{

class ShaderExecutableD3D : angle::NonCopyable
{
  public:
    ShaderExecutableD3D(const void *function, size_t length);
    virtual ~ShaderExecutableD3D();

    const uint8_t *getFunction() const;

    size_t getLength() const;

    const std::string &getDebugInfo() const;

    void appendDebugInfo(const std::string &info);

  private:
    std::vector<uint8_t> mFunctionBuffer;
    std::string mDebugInfo;
};

class UniformStorageD3D : angle::NonCopyable
{
  public:
    UniformStorageD3D(size_t initialSize);
    virtual ~UniformStorageD3D();

    size_t size() const;

  private:
    size_t mSize;
};

}

#endif // LIBANGLE_RENDERER_D3D_SHADEREXECUTABLED3D_H_
