//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayout.cpp:
//   Implementation for block layout classes and methods.
//

#include "compiler/translator/blocklayoutHLSL.h"

#include "common/mathutil.h"
#include "common/utilities.h"

namespace sh
{

HLSLBlockEncoder::HLSLBlockEncoder(HLSLBlockEncoderStrategy strategy)
    : mEncoderStrategy(strategy),
      mTransposeMatrices(false)
{
}

void HLSLBlockEncoder::enterAggregateType()
{
    nextRegister();
}

void HLSLBlockEncoder::exitAggregateType()
{
}

void HLSLBlockEncoder::getBlockLayoutInfo(GLenum typeIn, unsigned int arraySize, bool isRowMajorMatrix, int *arrayStrideOut, int *matrixStrideOut)
{
    GLenum type = (mTransposeMatrices ? gl::TransposeMatrixType(typeIn) : typeIn);

    // We assume we are only dealing with 4 byte components (no doubles or half-words currently)
    ASSERT(gl::VariableComponentSize(gl::VariableComponentType(type)) == BytesPerComponent);

    int matrixStride = 0;
    int arrayStride = 0;

    // if variables are not to be packed, or we're about to
    // pack a matrix or array, skip to the start of the next
    // register
    if (!isPacked() ||
        gl::IsMatrixType(type) ||
        arraySize > 0)
    {
        nextRegister();
    }

    if (gl::IsMatrixType(type))
    {
        matrixStride = ComponentsPerRegister;

        if (arraySize > 0)
        {
            const int numRegisters = gl::MatrixRegisterCount(type, isRowMajorMatrix);
            arrayStride = ComponentsPerRegister * numRegisters;
        }
    }
    else if (arraySize > 0)
    {
        arrayStride = ComponentsPerRegister;
    }
    else if (isPacked())
    {
        int numComponents = gl::VariableComponentCount(type);
        if ((numComponents + (mCurrentOffset % ComponentsPerRegister)) > ComponentsPerRegister)
        {
            nextRegister();
        }
    }

    *matrixStrideOut = matrixStride;
    *arrayStrideOut = arrayStride;
}

void HLSLBlockEncoder::advanceOffset(GLenum typeIn, unsigned int arraySize, bool isRowMajorMatrix, int arrayStride, int matrixStride)
{
    GLenum type = (mTransposeMatrices ? gl::TransposeMatrixType(typeIn) : typeIn);

    if (arraySize > 0)
    {
        mCurrentOffset += arrayStride * (arraySize - 1);
    }

    if (gl::IsMatrixType(type))
    {
        ASSERT(matrixStride == ComponentsPerRegister);
        const int numRegisters = gl::MatrixRegisterCount(type, isRowMajorMatrix);
        const int numComponents = gl::MatrixComponentCount(type, isRowMajorMatrix);
        mCurrentOffset += ComponentsPerRegister * (numRegisters - 1);
        mCurrentOffset += numComponents;
    }
    else if (isPacked())
    {
        mCurrentOffset += gl::VariableComponentCount(type);
    }
    else
    {
        mCurrentOffset += ComponentsPerRegister;
    }
}

void HLSLBlockEncoder::skipRegisters(unsigned int numRegisters)
{
    mCurrentOffset += (numRegisters * ComponentsPerRegister);
}

HLSLBlockEncoder::HLSLBlockEncoderStrategy HLSLBlockEncoder::GetStrategyFor(ShShaderOutput outputType)
{
    switch (outputType)
    {
      case SH_HLSL9_OUTPUT: return ENCODE_LOOSE;
      case SH_HLSL11_OUTPUT: return ENCODE_PACKED;
      default: UNREACHABLE(); return ENCODE_PACKED;
    }
}

template <class ShaderVarType>
void HLSLVariableRegisterCount(const ShaderVarType &variable, HLSLBlockEncoder *encoder)
{
    if (variable.isStruct())
    {
        for (size_t arrayElement = 0; arrayElement < variable.elementCount(); arrayElement++)
        {
            encoder->enterAggregateType();

            for (size_t fieldIndex = 0; fieldIndex < variable.fields.size(); fieldIndex++)
            {
                HLSLVariableRegisterCount(variable.fields[fieldIndex], encoder);
            }

            encoder->exitAggregateType();
        }
    }
    else
    {
        // We operate only on varyings and uniforms, which do not have matrix layout qualifiers
        encoder->encodeType(variable.type, variable.arraySize, false);
    }
}

unsigned int HLSLVariableRegisterCount(const Varying &variable, bool transposeMatrices)
{
    HLSLBlockEncoder encoder(HLSLBlockEncoder::ENCODE_PACKED);
    encoder.setTransposeMatrices(transposeMatrices);
    HLSLVariableRegisterCount(variable, &encoder);

    const size_t registerBytes = (encoder.BytesPerComponent * encoder.ComponentsPerRegister);
    return static_cast<unsigned int>(rx::roundUp<size_t>(encoder.getBlockSize(), registerBytes) / registerBytes);
}

unsigned int HLSLVariableRegisterCount(const Uniform &variable, ShShaderOutput outputType)
{
    HLSLBlockEncoder encoder(HLSLBlockEncoder::GetStrategyFor(outputType));
    encoder.setTransposeMatrices(true);
    HLSLVariableRegisterCount(variable, &encoder);

    const size_t registerBytes = (encoder.BytesPerComponent * encoder.ComponentsPerRegister);
    return static_cast<unsigned int>(rx::roundUp<size_t>(encoder.getBlockSize(), registerBytes) / registerBytes);
}

}
