#include "SkTypeface.h"
#include "SkFontHost.h"

uint32_t SkTypeface::UniqueID(const SkTypeface* face) {
    if (face) {
        return face->uniqueID();
    }

    // We cache the default fontID, assuming it will not change during a boot
    // The initial value of 0 is fine, since a typeface's uniqueID should not
    // be zero.
    static uint32_t gDefaultFontID;
    
    if (0 == gDefaultFontID) {
        SkTypeface* defaultFace = SkFontHost::CreateTypeface(NULL, NULL,
                                                    SkTypeface::kNormal);
        SkASSERT(defaultFace);
        gDefaultFontID = defaultFace->uniqueID();
        defaultFace->unref();
    }
    return gDefaultFontID;
}

bool SkTypeface::Equal(const SkTypeface* facea, const SkTypeface* faceb) {
    return SkTypeface::UniqueID(facea) == SkTypeface::UniqueID(faceb);
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkTypeface::CreateFromName(const char name[], Style style) {
    return SkFontHost::CreateTypeface(NULL, name, style);
}

SkTypeface* SkTypeface::CreateFromTypeface(const SkTypeface* family, Style s) {
    return SkFontHost::CreateTypeface(family, NULL, s);
}

SkTypeface* SkTypeface::CreateFromStream(SkStream* stream) {
    return SkFontHost::CreateTypefaceFromStream(stream);
}

SkTypeface* SkTypeface::CreateFromFile(const char path[]) {
    return SkFontHost::CreateTypefaceFromFile(path);
}

///////////////////////////////////////////////////////////////////////////////

void SkTypeface::serialize(SkWStream* stream) const {
    SkFontHost::Serialize(this, stream);
}

SkTypeface* SkTypeface::Deserialize(SkStream* stream) {
    return SkFontHost::Deserialize(stream);
}


