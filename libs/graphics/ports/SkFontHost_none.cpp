#include "SkFontHost.h"

SkTypeface* SkFontHost::CreateTypeface( const SkTypeface* familyFace,
                                        const char familyName[],
                                        SkTypeface::Style style)
{
    SkASSERT(!"SkFontHost::CreateTypeface unimplemented");
    return nil;
}

uint32_t SkFontHost::FlattenTypeface(const SkTypeface* tface, void* buffer)
{
    SkASSERT(!"SkFontHost::FlattenTypeface unimplemented");
    return 0;
}

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc)
{
    SkASSERT(!"SkFontHost::CreateScalarContext unimplemented");
    return nil;
}

SkFontHost::ScalerContextID SkFontHost::FindScalerContextIDForUnichar(int32_t unichar)
{
    SkASSERT(!"SkFontHost::FindScalerContextIDForUnichar unimplemented");
    return kMissing_ScalerContextID;
}

SkScalerContext* SkFontHost::CreateScalerContextFromID(SkFontHost::ScalerContextID id, const SkScalerContext::Rec& rec)
{
    SkASSERT(!"SkFontHost::CreateScalerContextFromID unimplemented");
    return nil;
}
