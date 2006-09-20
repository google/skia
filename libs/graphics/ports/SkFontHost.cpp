#include "SkFontHost.h"
#include "SkDescriptor.h"
#include "SkString.h"
#include <stdio.h>

#ifdef ANDROID
    #define kFontFilePrefix          "/fonts/"
#else
    // this is for our test apps that look directly into the src directory for the fonts
    #define kFontFilePrefix          "../fonts/"
#endif

struct FontFaceRec {
    const char* fFileName;    
    uint8_t     fFamilyIndex;
    SkBool8     fBold;
    SkBool8     fItalic;

    static const FontFaceRec& FindFace(const FontFaceRec rec[], int count, int isBold, int isItalic);
};

struct FontFamilyRec {
    const FontFaceRec*  fFaces;
    int                 fFaceCount;
};

// export for scalercontext subclass using freetype
void FontFaceRec_getFileName(const FontFaceRec* face, SkString* name);
void FontFaceRec_getFileName(const FontFaceRec* face, SkString* name)
{
    name->set(getenv("ANDROID_ROOT"));
    name->append(kFontFilePrefix);
    name->append(face->fFileName);
}

const FontFaceRec& FontFaceRec::FindFace(const FontFaceRec rec[], int count, int isBold, int isItalic)
{
    SkASSERT(count > 0);
    
    int i;

    // look for an exact match
    for (i = 0; i < count; i++) {
        if (rec[i].fBold == isBold && rec[i].fItalic == isItalic)
            return rec[i];
    }
    // look for a match in the bold field
    for (i = 0; i < count; i++) {
        if (rec[i].fBold == isBold)
            return rec[i];
    }
    // look for a normal/regular face
    for (i = 0; i < count; i++) {
        if (!rec[i].fBold && !rec[i].fItalic)
            return rec[i];
    }
    // give up
    return rec[0];
}

enum {
    SANS_FAMILY_INDEX,
    CJK_FAMILY_INDEX,
    MONO_FAMILY_INDEX,
    SERIF_FAMILY_INDEX,
    DROID_FAMILY_INDEX,
	DROID_MORE_FAMILY_INDEX,
	
    FAMILY_INDEX_COUNT
};



static const FontFaceRec gSansFaces[] = {
//    { "DejaVuSansCondensed.ttf",        SANS_FAMILY_INDEX, 0,  0 },
//    { "DejaVuSansCondensed-Bold.ttf",   SANS_FAMILY_INDEX, 1,  0 }
    { "DroidSans_bd100.ttf",        SANS_FAMILY_INDEX, 0,  0 },
    { "DroidSans-Bold_bd100.ttf",   SANS_FAMILY_INDEX, 1,  0 }
};

static const FontFaceRec gCJKFaces[] = {
    { "HeiTBig5_J.otf", CJK_FAMILY_INDEX, 0,  0 }
};

static const FontFaceRec gSerifFaces[] = {
    { "DejaVuSerifCondensed.ttf",               SERIF_FAMILY_INDEX, 0,  0 },
    { "DejaVuSerifCondensed-Bold.ttf",          SERIF_FAMILY_INDEX, 1,  0 },
    { "DejaVuSerifCondensed-Oblique.ttf",       SERIF_FAMILY_INDEX, 0,  1 },
    { "DejaVuSerifCondensed-BoldOblique.ttf",   SERIF_FAMILY_INDEX, 1,  1 }
};

static const FontFaceRec gMonoFaces[] = {
    { "DejaVuSansMono.ttf", MONO_FAMILY_INDEX, 0,  0 }
};

static const FontFaceRec gDroidFaces[] = {
	{ "DroidSansLight.ttf",  DROID_FAMILY_INDEX, 0,  0 },
    { "DroidSans.ttf",        DROID_FAMILY_INDEX, 1,  0 },
    { "DroidSansDemiBold.ttf",  DROID_FAMILY_INDEX, 0,  1 },
	{ "DroidSans-SemiBold.ttf",   DROID_FAMILY_INDEX, 1,  1 }
};

static const FontFaceRec gDroidMoreFaces[] = {
    { "DroidSans-Bold.ttf",   DROID_MORE_FAMILY_INDEX, 0,  0 },
	{ "DroidSansBlack.ttf",  DROID_MORE_FAMILY_INDEX, 1,  0 },
	{ "DroidSerifScotch.ttf", DROID_MORE_FAMILY_INDEX, 0,  1 }
};

// This table must be in the same order as the ..._FAMILY_INDEX enum specifies
static const FontFamilyRec gFamilies[] = {
    { gSansFaces,     SK_ARRAY_COUNT(gSansFaces)  },
    { gCJKFaces,      SK_ARRAY_COUNT(gCJKFaces)   },
    { gMonoFaces,     SK_ARRAY_COUNT(gMonoFaces)  },
    { gSerifFaces,    SK_ARRAY_COUNT(gSerifFaces) },
	{ gDroidFaces,    SK_ARRAY_COUNT(gDroidFaces) },
	{ gDroidMoreFaces,    SK_ARRAY_COUNT(gDroidMoreFaces) }
};

#define DEFAULT_FAMILY_INDEX            SANS_FAMILY_INDEX
#define DEFAULT_FAMILY_FACE_INDEX       0

////////////////////////////////////////////////////////////////////////////////////////

/* map common "web" font names to our font list */

struct FontFamilyMatchRec {
    const char* fLCName;
    int         fFamilyIndex;
};

/*  This is a table of synonyms for collapsing font names
    down to their pseudo-equivalents (i.e. in terms of fonts
    we actually have.)
    Keep this sorted by the first field so we can do a binary search.
    If this gets big, we could switch to a hash...
*/
static const FontFamilyMatchRec gMatches[] = {
    { "arial",              SANS_FAMILY_INDEX },
    { "courier",            MONO_FAMILY_INDEX },
    { "courier new",        MONO_FAMILY_INDEX },
    { "cursive",            SERIF_FAMILY_INDEX },
    { "dejavu mono",        MONO_FAMILY_INDEX },
    { "dejavu sans",        SANS_FAMILY_INDEX },
    { "dejavu serif",       SANS_FAMILY_INDEX },
	{ "droid more",         DROID_MORE_FAMILY_INDEX },
	{ "droid sans",         SANS_FAMILY_INDEX },
    { "fantasy",            SERIF_FAMILY_INDEX },
    { "goudy",              SERIF_FAMILY_INDEX },
    { "helvetica",          SANS_FAMILY_INDEX },
    { "lucida grande",      SANS_FAMILY_INDEX },
    { "lucida grande",      MONO_FAMILY_INDEX },
    { "palatino",           SERIF_FAMILY_INDEX },
    { "tahoma",             SANS_FAMILY_INDEX },
    { "sans-serif",         SANS_FAMILY_INDEX },
    { "serif",              SERIF_FAMILY_INDEX },
    { "times",              SERIF_FAMILY_INDEX },
    { "times new roman",    SERIF_FAMILY_INDEX },
    { "verdana",            SANS_FAMILY_INDEX }
};

////////////////////////////////////////////////////////////////////////////////////////

#include "SkTSearch.h"

const FontFamilyRec* find_family_rec(const char target[])
{
    size_t  targetLen = strlen(target);
    int     index;

    // Search for the font by matching the entire name
    index = SkStrLCSearch(&gMatches[0].fLCName, SK_ARRAY_COUNT(gMatches), target, targetLen, sizeof(gMatches[0]));
    if (index >= 0)
        return &gFamilies[gMatches[index].fFamilyIndex];

    // Sniff for key words...
    // If we converted target to lower-case, these would be simpler/faster...

    if (strstr(target, "sans") || strstr(target, "Sans"))
        return &gFamilies[SANS_FAMILY_INDEX];

    if (strstr(target, "serif") || strstr(target, "Serif"))
        return &gFamilies[SERIF_FAMILY_INDEX];

    if (strstr(target, "mono") || strstr(target, "Mono"))
        return &gFamilies[MONO_FAMILY_INDEX];

    // we give up, just give them the default font
    return &gFamilies[DEFAULT_FAMILY_INDEX];
}

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
    const FontFamilyRec* family;
    
    if (familyFace)
        family = &gFamilies[((FontFaceRec_Typeface*)familyFace)->fFace.fFamilyIndex];
    else if (familyName)
        family = find_family_rec(familyName);
    else
        family = &gFamilies[DEFAULT_FAMILY_INDEX];

    const FontFaceRec& face = FontFaceRec::FindFace(family->fFaces, family->fFaceCount,
                                                    (style & SkTypeface::kBold) != 0,
                                                    (style & SkTypeface::kItalic) != 0);

    // if we're returning our input parameter, no need to create a new instance
    if (familyFace != nil && &((FontFaceRec_Typeface*)familyFace)->fFace == &face)
    {
        familyFace->ref();
        return (SkTypeface*)familyFace;
    }
    return SkNEW_ARGS(FontFaceRec_Typeface, (face));
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
    // when we have more than one fall-back font, will have to do a search based on unichar
    // to know what index to return (0 means no fall-back can help)
    
    // we return the family index+1 to indicate succes, or 0 for failure (no fall-back font)
    return (ScalerContextID)(CJK_FAMILY_INDEX + 1);
}

SkScalerContext* SkFontHost::CreateScalerContextFromID(ScalerContextID id, const SkScalerContext::Rec& rec)
{
    SkASSERT((int)id > 0 && (int)id < FAMILY_INDEX_COUNT);

    const FontFaceRec* face = &gFamilies[(int)id - 1].fFaces[0];

	SkAutoDescriptor	ad(sizeof(rec) + sizeof(face) + SkDescriptor::ComputeOverhead(2));
	SkDescriptor*		desc = ad.getDesc();

	desc->init();
	desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    desc->addEntry(kTypeface_SkDescriptorTag, sizeof(face), &face);
	desc->computeChecksum();

    return SkFontHost::CreateScalerContext(desc);
}

