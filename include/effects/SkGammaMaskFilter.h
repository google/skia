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

#ifndef SkGammaMaskFilter_DEFINED
#define SkGammaMaskFilter_DEFINED

#include "SkMaskFilter.h"
#include "SkScalar.h"

/** \class SkGammaMaskFilter
 
    Applies a table lookup on each of the alpha values in the mask.
    An arbitrary table can be assigned, or a gamma (pow) table is computed
    based on the specified exponent.
 */
class SkGammaMaskFilter : public SkMaskFilter {
public:
    SkGammaMaskFilter();
    SkGammaMaskFilter(SkScalar gamma);
    SkGammaMaskFilter(const uint8_t table[256]);
    virtual ~SkGammaMaskFilter();

    void setGamma(SkScalar gamma);
    void setGammaTable(const uint8_t table[256]);

    // overrides from SkMaskFilter
    virtual SkMask::Format getFormat();
    virtual bool filterMask(SkMask*, const SkMask&, const SkMatrix&, SkIPoint*);
    
    // overrides from SkFlattenable
    virtual void flatten(SkFlattenableWriteBuffer& wb);
    virtual Factory getFactory();

protected:
    SkGammaMaskFilter(SkFlattenableReadBuffer& rb);
    static SkFlattenable* Factory(SkFlattenableReadBuffer&);

private:
    uint8_t fTable[256];
    
    typedef SkMaskFilter INHERITED;
};

#endif

