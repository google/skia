/* include/graphics/SkShaderExtras.h
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

#ifndef SkShaderExtras_DEFINED
#define SkShaderExtras_DEFINED

#include "SkShader.h"

class SkColorComposer :  public SkRefCnt {
public:
    /** Called with two scanlines of color. The implementation writes out its combination of
        those into the result[] scaline.
    */
    virtual void composeSpan(const SkPMColor srcA[], const SkPMColor srcB[], int count, SkPMColor result[]) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////

class SkComposeShader : public SkShader {
public:
    SkComposeShader(SkShader* sA, SkShader* sB, SkColorComposer* composer);
    virtual ~SkComposeShader();
    
    // override
    virtual bool setContext(const SkBitmap& device, const SkPaint& paint, const SkMatrix& matrix);
    virtual void shadeSpan(int x, int y, SkPMColor result[], int count);

private:
    enum {
        COUNT = 32
    };
    SkShader*           fShaderA;
    SkShader*           fShaderB;
    SkColorComposer*    fComposer;

    typedef SkShader INHERITED;
};

#endif
