#include "SkTypeface.h"
#include "SkFontHost.h"

static const SkTypeface* resolve_null_typeface(const SkTypeface* face)
{
    if (NULL == face) {
        face = SkFontHost::FindTypeface(NULL, NULL, SkTypeface::kNormal);
        SkASSERT(face);
    }
    return face;
}

uint32_t SkTypeface::UniqueID(const SkTypeface* face)
{
    return resolve_null_typeface(face)->uniqueID();
}

bool SkTypeface::Equal(const SkTypeface* facea, const SkTypeface* faceb)
{
    return resolve_null_typeface(facea)->uniqueID() ==
           resolve_null_typeface(faceb)->uniqueID();
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkTypeface::Create(const char name[], Style style)
{
    SkTypeface* face = SkFontHost::FindTypeface(NULL, name, style);
    face->ref();
    return face;
}

SkTypeface* SkTypeface::CreateFromTypeface(const SkTypeface* family, Style s)
{
    family = resolve_null_typeface(family);
    SkTypeface* face = SkFontHost::FindTypeface(family, NULL, s);
    face->ref();
    return face;
}

SkTypeface* SkTypeface::CreateFromStream(SkStream* stream)
{
    return SkFontHost::CreateTypeface(stream);
}

#include "SkMMapStream.h"
SkTypeface* SkTypeface::CreateFromFile(const char path[])
{
    return SkFontHost::CreateTypeface(SkNEW_ARGS(SkMMAPStream, (path)));
}

///////////////////////////////////////////////////////////////////////////////

void SkTypeface::serialize(SkWStream* stream) const {
    SkFontHost::Serialize(this, stream);
}

SkTypeface* SkTypeface::Deserialize(SkStream* stream) {
    SkTypeface* face = SkFontHost::Deserialize(stream);
    face->ref();
    return face;
}


