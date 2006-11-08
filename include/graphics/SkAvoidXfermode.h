/* include/graphics/SkAvoidXfermode.h
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

#ifndef SkAvoidXfermode_DEFINED
#define SkAvoidXfermode_DEFINED

#include "SkXfermode.h"

/** \class SkAvoidXfermode

    This xfermode will draw the src everywhere except on top of the specified
    color.
*/
class SkAvoidXfermode : public SkXfermode {
public:
    /** This xfermode will draw the src everywhere except on top of the specified
        color.
        @param opColor  the color to avoid (or to target if reverse is true);
        @param tolerance    How closely we compare a pixel to the opColor.
                            0 - we only avoid on an exact match
                            255 - maximum gradation (blending) based on how similar
                            the pixel is to our opColor.
        @param reverse      true means we target the opColor rather than avoid it.
    */
    SkAvoidXfermode(SkColor opColor, U8CPU tolerance, bool reverse);

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count, const SkAlpha aa[]);
    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count, const SkAlpha aa[]);
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count, const SkAlpha aa[]);

private:
    SkColor     fOpColor;
    uint32_t    fDistMul;   // x.14
    bool        fReverse;
};

#endif
