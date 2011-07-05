/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef SkEmptyShader_DEFINED
#define SkEmptyShader_DEFINED

#include "SkShader.h"

/**
 *  \class SkEmptyShader
 *  A Shader that always draws nothing.
 */
class SK_API SkEmptyShader : public SkShader {
public:
    SkEmptyShader();

    virtual uint32_t getFlags();
    virtual uint8_t getSpan16Alpha() const;
    virtual bool setContext(const SkBitmap& device, const SkPaint& paint,
                            const SkMatrix& matrix);
    virtual void shadeSpan(int x, int y, SkPMColor span[], int count);
    virtual void shadeSpan16(int x, int y, uint16_t span[], int count);
    virtual void shadeSpanAlpha(int x, int y, uint8_t alpha[], int count);

protected:
    SkEmptyShader(SkFlattenableReadBuffer&);
    virtual Factory getFactory();
    virtual void flatten(SkFlattenableWriteBuffer&);

private:
    typedef SkShader INHERITED;
};

#endif
