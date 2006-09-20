#include "SkColorFilter.h"
#include "SkShader.h"

void SkColorFilter::filterSpan(const SkPMColor src[], int count, SkPMColor result[])
{
    memcpy(result, src, count * sizeof(SkPMColor));
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

