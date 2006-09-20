#include "SkShader.h"
#include "SkColorFilter.h"
#include "SkShaderExtras.h"

//////////////////////////////////////////////////////////////////////////////////////
    
SkComposeShader::SkComposeShader(SkShader* sA, SkShader* sB, SkColorCombine* combine)
{
    fShaderA = sA;      sA->ref();
    fShaderB = sB;      sB->ref();
    fCombine = combine; combine->ref();
}

SkComposeShader::~SkComposeShader()
{
    fCombine->unref();
    fShaderB->unref();
    fShaderA->unref();
}

bool SkComposeShader::setContext(const SkBitmap& device,
                                 const SkPaint& paint,
                                 const SkMatrix& matrix)
{
    return  this->INHERITED::setContext(device, paint, matrix) &&
            fShaderA->setContext(device, paint, matrix) &&
            fShaderA->setContext(device, paint, matrix);
}

void SkComposeShader::shadeSpan(int x, int y, SkPMColor result[], int count)
{
    SkShader*       shaderA = fShaderA;
    SkShader*       shaderB = fShaderB;
    SkColorCombine* combine = fCombine;
    
    SkPMColor       tmp[COUNT];
    do {
        int n = count;
        if (n > COUNT)
            n = COUNT;
        
        shaderA->shadeSpan(x, y, tmp, n);
        shaderB->shadeSpan(x, y, result, n);
        combine->combineSpan(tmp, result, n, result);
        
        result += n;
        x += n;
        count -= n;
    } while (count > 0);
}
    
