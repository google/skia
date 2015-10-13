//
// Copyright (c) 2010-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_UNIFORM_H_
#define LIBANGLE_UNIFORM_H_

#include <string>
#include <vector>

#include "angle_gl.h"
#include "common/debug.h"
#include "common/MemoryBuffer.h"
#include "compiler/translator/blocklayout.h"
#include "libANGLE/angletypes.h"

namespace gl
{

// Helper struct representing a single shader uniform
struct LinkedUniform : public sh::Uniform
{
    LinkedUniform();
    LinkedUniform(GLenum type, GLenum precision, const std::string &name, unsigned int arraySize, const int blockIndex, const sh::BlockMemberInfo &blockInfo);
    LinkedUniform(const sh::Uniform &uniform);
    LinkedUniform(const LinkedUniform &uniform);
    LinkedUniform &operator=(const LinkedUniform &uniform);
    ~LinkedUniform();

    size_t dataSize() const;
    uint8_t *data();
    const uint8_t *data() const;
    bool isSampler() const;
    bool isInDefaultBlock() const;
    bool isField() const;
    size_t getElementSize() const;
    uint8_t *getDataPtrToElement(size_t elementIndex);
    const uint8_t *getDataPtrToElement(size_t elementIndex) const;

    int blockIndex;
    sh::BlockMemberInfo blockInfo;

  private:
    mutable rx::MemoryBuffer mLazyData;
};

// Helper struct representing a single shader uniform block
struct UniformBlock
{
    UniformBlock();
    UniformBlock(const std::string &nameIn, bool isArrayIn, unsigned int arrayElementIn);
    UniformBlock(const UniformBlock &other) = default;
    UniformBlock &operator=(const UniformBlock &other) = default;

    std::string name;
    bool isArray;
    unsigned int arrayElement;
    unsigned int dataSize;

    bool vertexStaticUse;
    bool fragmentStaticUse;

    std::vector<unsigned int> memberUniformIndexes;

    // TODO(jmadill): Make D3D-only.
    unsigned int psRegisterIndex;
    unsigned int vsRegisterIndex;
};

}

#endif   // LIBANGLE_UNIFORM_H_
