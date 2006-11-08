/* libs/graphics/effects/SkShaderExtras.cpp
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

#include "SkShader.h"
#include "SkColorFilter.h"
#include "SkShaderExtras.h"

//////////////////////////////////////////////////////////////////////////////////////
    
SkComposeShader::SkComposeShader(SkShader* sA, SkShader* sB, SkColorComposer* composer)
{
    fShaderA = sA;          sA->ref();
    fShaderB = sB;          sB->ref();
    fComposer = composer;   composer->ref();
}

SkComposeShader::~SkComposeShader()
{
    fComposer->unref();
    fShaderB->unref();
    fShaderA->unref();
}

bool SkComposeShader::setContext(const SkBitmap& device,
                                 const SkPaint& paint,
                                 const SkMatrix& matrix)
{
    if (!this->INHERITED::setContext(device, paint, matrix))
        return false;

    // we preconcat our localMatrix (if any) with the device matrix
    // before calling our sub-shaders

    const SkMatrix* localM = this->getLocalMatrix();
    SkMatrix        storage;
    const SkMatrix* tmpM;
    
    if (NULL == localM)
        tmpM = &matrix;
    else
    {
        storage.setConcat(matrix, *localM);
        tmpM = &storage;
    }

    return  fShaderA->setContext(device, paint, *tmpM) &&
            fShaderB->setContext(device, paint, *tmpM);
}

void SkComposeShader::shadeSpan(int x, int y, SkPMColor result[], int count)
{
    SkShader*       shaderA = fShaderA;
    SkShader*       shaderB = fShaderB;
    SkColorComposer* composer = fComposer;
    
    SkPMColor       tmp[COUNT];
    do {
        int n = count;
        if (n > COUNT)
            n = COUNT;
        
        shaderA->shadeSpan(x, y, tmp, n);
        shaderB->shadeSpan(x, y, result, n);
        composer->composeSpan(tmp, result, n, result);
        
        result += n;
        x += n;
        count -= n;
    } while (count > 0);
}
    
