//
// Copyright (c) 2010-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/Uniform.h"

#include "common/utilities.h"

#include <cstring>

namespace gl
{

LinkedUniform::LinkedUniform()
    : blockIndex(-1), blockInfo(sh::BlockMemberInfo::getDefaultBlockInfo())
{
}

LinkedUniform::LinkedUniform(GLenum typeIn,
                             GLenum precisionIn,
                             const std::string &nameIn,
                             unsigned int arraySizeIn,
                             const int blockIndexIn,
                             const sh::BlockMemberInfo &blockInfoIn)
    : blockIndex(blockIndexIn), blockInfo(blockInfoIn)
{
    type      = typeIn;
    precision = precisionIn;
    name      = nameIn;
    arraySize = arraySizeIn;
}

LinkedUniform::LinkedUniform(const sh::Uniform &uniform)
    : sh::Uniform(uniform), blockIndex(-1), blockInfo(sh::BlockMemberInfo::getDefaultBlockInfo())
{
}

LinkedUniform::LinkedUniform(const LinkedUniform &uniform)
    : sh::Uniform(uniform), blockIndex(uniform.blockIndex), blockInfo(uniform.blockInfo)
{
    // This function is not intended to be called during runtime.
    ASSERT(uniform.mLazyData.empty());
}

LinkedUniform &LinkedUniform::operator=(const LinkedUniform &uniform)
{
    // This function is not intended to be called during runtime.
    ASSERT(uniform.mLazyData.empty());

    sh::Uniform::operator=(uniform);
    blockIndex           = uniform.blockIndex;
    blockInfo            = uniform.blockInfo;

    return *this;
}

LinkedUniform::~LinkedUniform()
{
}

bool LinkedUniform::isInDefaultBlock() const
{
    return blockIndex == -1;
}

size_t LinkedUniform::dataSize() const
{
    ASSERT(type != GL_STRUCT_ANGLEX);
    if (mLazyData.empty())
    {
        mLazyData.resize(VariableExternalSize(type) * elementCount());
        ASSERT(!mLazyData.empty());
    }

    return mLazyData.size();
}

uint8_t *LinkedUniform::data()
{
    if (mLazyData.empty())
    {
        // dataSize() will init the data store.
        size_t size = dataSize();
        memset(mLazyData.data(), 0, size);
    }

    return mLazyData.data();
}

const uint8_t *LinkedUniform::data() const
{
    return const_cast<LinkedUniform *>(this)->data();
}

bool LinkedUniform::isSampler() const
{
    return IsSamplerType(type);
}

bool LinkedUniform::isField() const
{
    return name.find('.') != std::string::npos;
}

size_t LinkedUniform::getElementSize() const
{
    return VariableExternalSize(type);
}

uint8_t *LinkedUniform::getDataPtrToElement(size_t elementIndex)
{
    ASSERT((!isArray() && elementIndex == 0) || (isArray() && elementIndex < arraySize));
    return data() + getElementSize() * elementIndex;
}

const uint8_t *LinkedUniform::getDataPtrToElement(size_t elementIndex) const
{
    return const_cast<LinkedUniform *>(this)->getDataPtrToElement(elementIndex);
}

UniformBlock::UniformBlock()
    : isArray(false),
      arrayElement(0),
      dataSize(0),
      vertexStaticUse(false),
      fragmentStaticUse(false),
      psRegisterIndex(GL_INVALID_INDEX),
      vsRegisterIndex(GL_INVALID_INDEX)
{
}

UniformBlock::UniformBlock(const std::string &nameIn, bool isArrayIn, unsigned int arrayElementIn)
    : name(nameIn),
      isArray(isArrayIn),
      arrayElement(arrayElementIn),
      dataSize(0),
      vertexStaticUse(false),
      fragmentStaticUse(false),
      psRegisterIndex(GL_INVALID_INDEX),
      vsRegisterIndex(GL_INVALID_INDEX)
{
}

}
