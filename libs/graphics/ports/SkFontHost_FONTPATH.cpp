/* libs/graphics/ports/SkFontHost_FONTPATH.cpp
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

#include "SkFontHost.h"
#include "SkString.h"
#include <stdio.h>

//#define SK_FONTPATH "the complete file name for our font"

struct FontFaceRec {
    const char* fFileName;    
    uint8_t     fFamilyIndex;
    SkBool8     fBold;
    SkBool8     fItalic;
};

struct FontFamilyRec {
    const char*         fLCName;
    const FontFaceRec*  fFaces;
    int                 fFaceCount;
};

// export for scalercontext subclass using freetype
void FontFaceRec_getFileName(const FontFaceRec* face, SkString* name);
void FontFaceRec_getFileName(const FontFaceRec* face, SkString* name)
{
    name->set(SK_FONTPATH);
}

static const FontFaceRec gArialFaces[] = {
    { SK_FONTPATH,        0, 0,  0 }
};

static const FontFamilyRec gFamilies[] = {
    { "font",  gArialFaces, SK_ARRAY_COUNT(gArialFaces) },
};

#define DEFAULT_FAMILY_INDEX            0
#define DEFAULT_FAMILY_FACE_INDEX       0

///////////////////////////////////////////////////////////////////////////////////////////////

class FontFaceRec_Typeface : public SkTypeface {
public:
    FontFaceRec_Typeface(const FontFaceRec& face) : fFace(face)
    {
        int style = 0;
        if (face.fBold)
            style |= SkTypeface::kBold;
        if (face.fItalic)
            style |= SkTypeface::kItalic;
        this->setStyle((SkTypeface::Style)style);
    }

    const FontFaceRec& fFace;
};

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace, const char familyName[], SkTypeface::Style style)
{
    return SkNEW_ARGS(FontFaceRec_Typeface, (gFamilies[DEFAULT_FAMILY_INDEX].fFaces[DEFAULT_FAMILY_FACE_INDEX]));
}

uint32_t SkFontHost::FlattenTypeface(const SkTypeface* tface, void* buffer)
{
    const FontFaceRec* face;
    
    if (tface)
        face = &((const FontFaceRec_Typeface*)tface)->fFace;
    else
       face = &gFamilies[DEFAULT_FAMILY_INDEX].fFaces[DEFAULT_FAMILY_FACE_INDEX];

    size_t  size = sizeof(face);
    if (buffer)
        memcpy(buffer, &face, size);
    return size;
}

SkFontHost::ScalerContextID SkFontHost::FindScalerContextIDForUnichar(int32_t unichar)
{
    return kMissing_ScalerContextID;   // don't do any font-chaining when we only have 1 font
}

SkScalerContext* SkFontHost::CreateScalerContextFromID(ScalerContextID, const SkScalerContext::Rec&)
{
    SkASSERT(!"Should not be called, since we always return kMissing_ScalerContextID");
    return NULL;
}

