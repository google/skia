/* libs/graphics/sgl/SkColorFilter.cpp
**
** Copyright 2006, Google Inc.
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

void SkColorFilter::filterSpan(const SkPMColor src[], int count, SkPMColor result[])
{
    if (result != src) {
        memcpy(result, src, count * sizeof(SkPMColor));
    }
}

//////////////////////////////////////////////////////////////////////////////////////

SkFilterShader::SkFilterShader(SkShader* shader, SkColorFilter* filter)
{
    fShader = shader;   shader->ref();
    fFilter = filter;   filter->ref();
}

SkFilterShader::~SkFilterShader()
{
    fFilter->unref();
    fShader->unref();
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

