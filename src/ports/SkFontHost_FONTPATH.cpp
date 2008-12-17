/* libs/graphics/ports/SkFontHost_android.cpp
**
** Copyright 2006, The Android Open Source Project
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
#include "SkDescriptor.h"
#include "SkString.h"
#include "SkStream.h"
#include <stdio.h>

/* define this if we can use mmap() to access fonts from the filesystem */
#define SK_CAN_USE_MMAP 

#ifndef SK_FONTPATH
    #define SK_FONTPATH "the complete path for a font file"
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
    DEFAULT_FAMILY_INDEX,
    
    FAMILY_INDEX_COUNT
};

static const FontFaceRec gDefaultFaces[] = {
    { SK_FONTPATH, DEFAULT_FAMILY_INDEX, 0,  0 }
};

// This table must be in the same order as the ..._FAMILY_INDEX enum specifies
static const FontFamilyRec gFamilies[] = {
    { gDefaultFaces,   SK_ARRAY_COUNT(gDefaultFaces)  }
};

#define DEFAULT_FAMILY_INDEX            DEFAULT_FAMILY_INDEX
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
#if 0
    { "Ahem",               Ahem_FAMILY_INDEX },
    { "arial",              SANS_FAMILY_INDEX },
    { "courier",            MONO_FAMILY_INDEX },
    { "courier new",        MONO_FAMILY_INDEX },
    { "cursive",            SERIF_FAMILY_INDEX },
    { "fantasy",            SERIF_FAMILY_INDEX },
    { "georgia",            SERIF_FAMILY_INDEX },
    { "goudy",              SERIF_FAMILY_INDEX },
    { "helvetica",          SANS_FAMILY_INDEX },
    { "palatino",           SERIF_FAMILY_INDEX },
    { "tahoma",             SANS_FAMILY_INDEX },
    { "sans-serif",         SANS_FAMILY_INDEX },
    { "serif",              SERIF_FAMILY_INDEX },
    { "times",              SERIF_FAMILY_INDEX },
    { "times new roman",    SERIF_FAMILY_INDEX },
    { "verdana",            SANS_FAMILY_INDEX }
#endif
};

////////////////////////////////////////////////////////////////////////////////////////

#include "SkTSearch.h"

static bool contains_only_ascii(const char s[])
{
    for (;;)
    {
        int c = *s++;
        if (c == 0)
            break;
        if ((c >> 7) != 0)
            return false;
    }
    return true;
}

#define TRACE_FONT_NAME(code)
//#define TRACE_FONT_NAME(code)   code

const FontFamilyRec* find_family_rec(const char target[])
{
    int     index;

    //  If we're asked for a font name that contains non-ascii,
    //  1) SkStrLCSearch can't handle it
    //  2) All of our fonts are have ascii names, so...

TRACE_FONT_NAME(printf("----------------- font request <%s>", target);)

    if (contains_only_ascii(target))
    {
        // Search for the font by matching the entire name
        index = SkStrLCSearch(&gMatches[0].fLCName, SK_ARRAY_COUNT(gMatches), target, sizeof(gMatches[0]));
        if (index >= 0)
        {
            TRACE_FONT_NAME(printf(" found %d\n", index);)
            return &gFamilies[gMatches[index].fFamilyIndex];
        }
    }

    // Sniff for key words...

#if 0
    if (strstr(target, "sans") || strstr(target, "Sans"))
    {
        TRACE_FONT_NAME(printf(" found sans\n");)
        return &gFamilies[SANS_FAMILY_INDEX];
    }
    if (strstr(target, "serif") || strstr(target, "Serif"))
    {
        TRACE_FONT_NAME(printf(" found serif\n");)
        return &gFamilies[SERIF_FAMILY_INDEX];
    }
    if (strstr(target, "mono") || strstr(target, "Mono"))
    {
        TRACE_FONT_NAME(printf(" found mono\n");)
        return &gFamilies[MONO_FAMILY_INDEX];
    }
#endif

    TRACE_FONT_NAME(printf(" use default\n");)
    // we give up, just give them the default font
    return &gFamilies[DEFAULT_FAMILY_INDEX];
}

///////////////////////////////////////////////////////////////////////////////////////////////

static const FontFaceRec* get_default_face()
{
    return &gFamilies[DEFAULT_FAMILY_INDEX].fFaces[DEFAULT_FAMILY_FACE_INDEX];
}

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

    // This global const reference completely identifies the face
    const FontFaceRec& fFace;
};

static const FontFaceRec* get_typeface_rec(const SkTypeface* face)
{
    const FontFaceRec_Typeface* f = (FontFaceRec_Typeface*)face;
    return f ? &f->fFace : get_default_face();
}

static uint32_t ptr2uint32(const void* p)
{
    // cast so we avoid warnings on 64bit machines that a ptr difference
    // which might be 64bits is being trucated from 64 to 32
    return (uint32_t)((char*)p - (char*)0);
}

uint32_t SkFontHost::TypefaceHash(const SkTypeface* face)
{
    // just use our address as the hash value
    return ptr2uint32(get_typeface_rec(face));
}

bool SkFontHost::TypefaceEqual(const SkTypeface* facea, const SkTypeface* faceb)
{
    return get_typeface_rec(facea) == get_typeface_rec(faceb);
}

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
    if (familyFace != NULL && &((FontFaceRec_Typeface*)familyFace)->fFace == &face)
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
       face = get_default_face();

    size_t  size = sizeof(face);
    if (buffer)
        memcpy(buffer, &face, size);
    return size;
}

void SkFontHost::GetDescriptorKeyString(const SkDescriptor* desc, SkString* key)
{
    key->set(SK_FONTPATH);
}

#ifdef SK_CAN_USE_MMAP
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

class SkMMAPStream : public SkMemoryStream {
public:
    SkMMAPStream(const char filename[]);
    virtual ~SkMMAPStream();

    virtual void setMemory(const void* data, size_t length);
private:
    int     fFildes;
    void*   fAddr;
    size_t  fSize;
    
    void closeMMap();
    
    typedef SkMemoryStream INHERITED;
};

SkMMAPStream::SkMMAPStream(const char filename[])
{
    fFildes = -1;   // initialize to failure case

    int fildes = open(filename, O_RDONLY);
    if (fildes < 0)
    {
        SkDEBUGF(("---- failed to open(%s) for mmap stream error=%d\n", filename, errno));
        return;
    }

    off_t size = lseek(fildes, 0, SEEK_END);    // find the file size
    if (size == -1)
    {
        SkDEBUGF(("---- failed to lseek(%s) for mmap stream error=%d\n", filename, errno));
        close(fildes);
        return;
    }
    (void)lseek(fildes, 0, SEEK_SET);   // restore file offset to beginning

    void* addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fildes, 0);
    if (MAP_FAILED == addr)
    {
        SkDEBUGF(("---- failed to mmap(%s) for mmap stream error=%d\n", filename, errno));
        close(fildes);
        return;
    }

    this->INHERITED::setMemory(addr, size);

    fFildes = fildes;
    fAddr = addr;
    fSize = size;
}

SkMMAPStream::~SkMMAPStream()
{
    this->closeMMap();
}

void SkMMAPStream::setMemory(const void* data, size_t length)
{
    this->closeMMap();
    this->INHERITED::setMemory(data, length);
}

void SkMMAPStream::closeMMap()
{
    if (fFildes >= 0)
    {
        munmap(fAddr, fSize);
        close(fFildes);
        fFildes = -1;
    }
}

#endif

SkStream* SkFontHost::OpenDescriptorStream(const SkDescriptor* desc, const char keyString[])
{
    // our key string IS our filename, so we can ignore desc
    SkStream* strm;

#ifdef SK_CAN_USE_MMAP
    strm = new SkMMAPStream(keyString);
    if (strm->getLength() > 0)
        return strm;

    // strm not valid
    delete strm;
    // fall through to FILEStream attempt
#endif

    strm = new SkFILEStream(keyString);
    if (strm->getLength() > 0)
        return strm;

    // strm not valid
    delete strm;
    return NULL;
}

SkScalerContext* SkFontHost::CreateFallbackScalerContext(const SkScalerContext::Rec& rec)
{
    const FontFaceRec* face = get_default_face();

    SkAutoDescriptor    ad(sizeof(rec) + sizeof(face) + SkDescriptor::ComputeOverhead(2));
    SkDescriptor*       desc = ad.getDesc();

    desc->init();
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    desc->addEntry(kTypeface_SkDescriptorTag, sizeof(face), &face);
    desc->computeChecksum();

    return SkFontHost::CreateScalerContext(desc);
}

size_t SkFontHost::ShouldPurgeFontCache(size_t sizeAllocatedSoFar)
{
    return 0;   // nothing to do (change me if you want to limit the font cache)
}

int SkFontHost::ComputeGammaFlag(const SkPaint& paint)
{
    return 0;
}

void SkFontHost::GetGammaTables(const uint8_t* tables[2])
{
    tables[0] = NULL;   // black gamma (e.g. exp=1.4)
    tables[1] = NULL;   // white gamma (e.g. exp= 1/1.4)
}

