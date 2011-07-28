
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkUnitMappers_DEFINED
#define SkUnitMappers_DEFINED

#include "SkUnitMapper.h"

/** This discretizes the range [0...1) into N discret values.
*/
class SkDiscreteMapper : public SkUnitMapper {
public:
    SkDiscreteMapper(int segments);
    // override from SkUnitMapper
    virtual uint16_t mapUnit16(uint16_t x);

protected:
    SkDiscreteMapper(SkFlattenableReadBuffer& );
    // overrides from SkFlattenable
    virtual void flatten(SkFlattenableWriteBuffer& );
    virtual Factory getFactory();
private:
    int     fSegments;
    SkFract fScale;    // computed from fSegments

    static SkFlattenable* Create(SkFlattenableReadBuffer& buffer);
    
    typedef SkUnitMapper INHERITED;
};

/** This returns cos(x), to simulate lighting a sphere, where 0 maps to the
    center of the sphere, and 1 maps to the edge.
*/
class SkCosineMapper : public SkUnitMapper {
public:
    SkCosineMapper() {}
    // override from SkUnitMapper
    virtual uint16_t mapUnit16(uint16_t x);

protected:
    SkCosineMapper(SkFlattenableReadBuffer&);
    // overrides from SkFlattenable
    virtual Factory getFactory();

private:
    static SkFlattenable* Create(SkFlattenableReadBuffer&);

    typedef SkUnitMapper INHERITED;
};

#endif

