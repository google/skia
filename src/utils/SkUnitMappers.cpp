#include "SkUnitMappers.h"

SkDiscreteMapper::SkDiscreteMapper(int segments)
{
    if (segments < 2)
    {
        fSegments = 0;
        fScale = 0;
    }
    else
    {
        if (segments > 0xFFFF)
            segments = 0xFFFF;
        fSegments = segments;
        fScale = SK_Fract1 / (segments - 1);
    }
}

uint16_t SkDiscreteMapper::mapUnit16(uint16_t input)
{
    SkFixed x = input * fSegments >> 16;
    x = x * fScale >> 14;
    x += x << 15 >> 31; // map 0x10000 to 0xFFFF
    return SkToU16(x);
}

SkDiscreteMapper::SkDiscreteMapper(SkFlattenableReadBuffer& rb)
    : SkUnitMapper(rb)
{
    fSegments = rb.readU32();
    fScale = rb.readU32();
}

SkFlattenable::Factory SkDiscreteMapper::getFactory()
{
    return Create;
}

SkFlattenable* SkDiscreteMapper::Create(SkFlattenableReadBuffer& rb)
{
    return SkNEW_ARGS(SkDiscreteMapper, (rb));
}

void SkDiscreteMapper::flatten(SkFlattenableWriteBuffer& wb)
{
    this->INHERITED::flatten(wb);

    wb.write32(fSegments);
    wb.write32(fScale);
}

///////////////////////////////////////////////////////////////////////////////

uint16_t SkCosineMapper::mapUnit16(uint16_t input)
{
    /*  we want to call cosine(input * pi/2) treating input as [0...1)
        however, the straight multitply would overflow 32bits since input is
        16bits and pi/2 is 17bits, so we shift down our pi const before we mul
    */
    SkFixed rads = (unsigned)(input * (SK_FixedPI >> 2)) >> 15;
    SkFixed x = SkFixedCos(rads);
    x += x << 15 >> 31; // map 0x10000 to 0xFFFF
    return SkToU16(x);
}

SkCosineMapper::SkCosineMapper(SkFlattenableReadBuffer& rb)
    : SkUnitMapper(rb)
{
}

SkFlattenable::Factory SkCosineMapper::getFactory()
{
    return Create;
}

SkFlattenable* SkCosineMapper::Create(SkFlattenableReadBuffer& rb)
{
    return SkNEW_ARGS(SkCosineMapper, (rb));
}

