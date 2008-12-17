/* libs/graphics/sgl/SkColorFilter.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkColorFilter.h"
#include "SkShader.h"

void SkColorFilter::filterSpan16(const uint16_t s[], int count, uint16_t d[])
{
    SkASSERT(this->getFlags() & SkColorFilter::kHasFilter16_Flag);
    SkASSERT(!"missing implementation of SkColorFilter::filterSpan16");

    if (d != s)
        memcpy(d, s, count * sizeof(uint16_t));
}

//////////////////////////////////////////////////////////////////////////////

SkFilterShader::SkFilterShader(SkShader* shader, SkColorFilter* filter)
{
    fShader = shader;   shader->ref();
    fFilter = filter;   filter->ref();
}

SkFilterShader::SkFilterShader(SkFlattenableReadBuffer& buffer) :
    INHERITED(buffer)
{
    fShader = static_cast<SkShader*>(buffer.readFlattenable());
    fFilter = static_cast<SkColorFilter*>(buffer.readFlattenable());
}

SkFilterShader::~SkFilterShader()
{
    fFilter->unref();
    fShader->unref();
}

void SkFilterShader::beginSession()
{
    this->INHERITED::beginSession();
    fShader->beginSession();
}

void SkFilterShader::endSession()
{
    fShader->endSession();
    this->INHERITED::endSession();
}

void SkFilterShader::flatten(SkFlattenableWriteBuffer& buffer)
{
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fShader);
    buffer.writeFlattenable(fFilter);
}

uint32_t SkFilterShader::getFlags()
{
    uint32_t shaderF = fShader->getFlags();
    uint32_t filterF = fFilter->getFlags();
    
    // if the filter doesn't support 16bit, clear the matching bit in the shader
    if (!(filterF & SkColorFilter::kHasFilter16_Flag))
        shaderF &= ~SkShader::kHasSpan16_Flag;
    
    // if the filter might change alpha, clear the opaque flag in the shader
    if (!(filterF & SkColorFilter::kAlphaUnchanged_Flag))
        shaderF &= ~(SkShader::kOpaqueAlpha_Flag | SkShader::kHasSpan16_Flag);
    
    return shaderF;
}

bool SkFilterShader::setContext(const SkBitmap& device,
                                const SkPaint& paint,
                                const SkMatrix& matrix)
{
    return  this->INHERITED::setContext(device, paint, matrix) &&
            fShader->setContext(device, paint, matrix);
}

void SkFilterShader::shadeSpan(int x, int y, SkPMColor result[], int count)
{
    fShader->shadeSpan(x, y, result, count);
    fFilter->filterSpan(result, count, result);
}

void SkFilterShader::shadeSpan16(int x, int y, uint16_t result[], int count)
{
    SkASSERT(fShader->getFlags() & SkShader::kHasSpan16_Flag);
    SkASSERT(fFilter->getFlags() & SkColorFilter::kHasFilter16_Flag);

    fShader->shadeSpan16(x, y, result, count);
    fFilter->filterSpan16(result, count, result);
}

