//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayout.h:
//   Methods and classes related to uniform layout and packing in GLSL and HLSL.
//

#ifndef COMMON_BLOCKLAYOUTHLSL_H_
#define COMMON_BLOCKLAYOUTHLSL_H_

#include <cstddef>
#include <vector>

#include "angle_gl.h"
#include "blocklayout.h"
#include <GLSLANG/ShaderLang.h>

namespace sh
{
// Block layout packed according to the D3D9 or default D3D10+ register packing rules
// See http://msdn.microsoft.com/en-us/library/windows/desktop/bb509632(v=vs.85).aspx
// The strategy should be ENCODE_LOOSE for D3D9 constant blocks, and ENCODE_PACKED
// for everything else (D3D10+ constant blocks and all attributes/varyings).

class COMPILER_EXPORT HLSLBlockEncoder : public BlockLayoutEncoder
{
  public:
    enum HLSLBlockEncoderStrategy
    {
        ENCODE_PACKED,
        ENCODE_LOOSE
    };

    HLSLBlockEncoder(HLSLBlockEncoderStrategy strategy);

    virtual void enterAggregateType();
    virtual void exitAggregateType();
    void skipRegisters(unsigned int numRegisters);

    bool isPacked() const { return mEncoderStrategy == ENCODE_PACKED; }
    void setTransposeMatrices(bool enabled) { mTransposeMatrices = enabled; }

    static HLSLBlockEncoderStrategy GetStrategyFor(ShShaderOutput outputType);

  protected:
    virtual void getBlockLayoutInfo(GLenum type, unsigned int arraySize, bool isRowMajorMatrix, int *arrayStrideOut, int *matrixStrideOut);
    virtual void advanceOffset(GLenum type, unsigned int arraySize, bool isRowMajorMatrix, int arrayStride, int matrixStride);

    HLSLBlockEncoderStrategy mEncoderStrategy;
    bool mTransposeMatrices;
};

// This method returns the number of used registers for a ShaderVariable. It is dependent on the HLSLBlockEncoder
// class to count the number of used registers in a struct (which are individually packed according to the same rules).
COMPILER_EXPORT unsigned int HLSLVariableRegisterCount(const Varying &variable, bool transposeMatrices);
COMPILER_EXPORT unsigned int HLSLVariableRegisterCount(const Uniform &variable, ShShaderOutput outputType);

}

#endif // COMMON_BLOCKLAYOUTHLSL_H_
