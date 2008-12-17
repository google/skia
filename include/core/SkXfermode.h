/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkXfermode_DEFINED
#define SkXfermode_DEFINED

#include "SkFlattenable.h"
#include "SkColor.h"

/** \class SkXfermode

    SkXfermode is the base class for objects that are called to implement custom
    "transfer-modes" in the drawing pipeline. The static function Create(Modes)
    can be called to return an instance of any of the predefined subclasses as
    specified in the Modes enum. When an SkXfermode is assigned to an SkPaint,
    then objects drawn with that paint have the xfermode applied.
*/
class SkXfermode : public SkFlattenable {
public:
    SkXfermode() {}

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]);
    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]);
    virtual void xfer4444(uint16_t dst[], const SkPMColor src[], int count,
                          const SkAlpha aa[]);
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]);
    
    enum Coeff {
        kZero_Coeff,
        kOne_Coeff,
        kSC_Coeff,
        kISC_Coeff,
        kDC_Coeff,
        kIDC_Coeff,
        kSA_Coeff,
        kISA_Coeff,
        kDA_Coeff,
        kIDA_Coeff,
        
        kCoeffCount
    };
    virtual bool asCoeff(Coeff* src, Coeff* dst);

protected:
    SkXfermode(SkFlattenableReadBuffer& rb) : SkFlattenable(rb) {}
    
    /** The default implementation of xfer32/xfer16/xferA8 in turn call this
        method, 1 color at a time (upscaled to a SkPMColor). The default
        implmentation of this method just returns dst. If performance is
        important, your subclass should override xfer32/xfer16/xferA8 directly.
        
        This method will not be called directly by the client, so it need not
        be implemented if your subclass has overridden xfer32/xfer16/xferA8
    */
    virtual SkPMColor xferColor(SkPMColor src, SkPMColor dst);

private:
    typedef SkFlattenable INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

/** \class SkProcXfermode

    SkProcXfermode is a xfermode that applies the specified proc to its colors.
    This class is not exported to java.
*/
class SkProcXfermode : public SkXfermode {
public:
    SkProcXfermode(SkXfermodeProc proc) : fProc(proc) {}

    // overrides from SkXfermode
    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]);
    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]);
    virtual void xfer4444(uint16_t dst[], const SkPMColor src[], int count,
                          const SkAlpha aa[]);
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]);

    // overrides from SkFlattenable
    virtual Factory getFactory() { return CreateProc; }
    virtual void    flatten(SkFlattenableWriteBuffer&);

protected:
    SkProcXfermode(SkFlattenableReadBuffer&);

private:
    SkXfermodeProc  fProc;
    
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkProcXfermode, (buffer)); }

    typedef SkXfermode INHERITED;
};

#endif

