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
#include "SkMMapStream.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkStream.h"
#include "SkThread.h"
#include "SkTSearch.h"
#include <stdio.h>

#define FONT_CACHE_MEMORY_BUDGET    (768 * 1024)

#ifndef SK_FONT_FILE_PREFIX
    #define SK_FONT_FILE_PREFIX          "/fonts/"
#endif

SkTypeface::Style find_name_and_style(SkStream* stream, SkString* name);

static void GetFullPathForSysFonts(SkString* full, const char name[])
{
    full->set(getenv("ANDROID_ROOT"));
    full->append(SK_FONT_FILE_PREFIX);
    full->append(name);
}

///////////////////////////////////////////////////////////////////////////////

struct FamilyRec;

/*  This guy holds a mapping of a name -> family, used for looking up fonts.
    Since it is stored in a stretchy array that doesn't preserve object
    semantics, we don't use constructor/destructors, but just have explicit
    helpers to manage our internal bookkeeping.
*/
struct NameFamilyPair {
    const char* fName;      // we own this
    FamilyRec*  fFamily;    // we don't own this, we just reference it
    
    void construct(const char name[], FamilyRec* family)
    {
        fName = strdup(name);
        fFamily = family;   // we don't own this, so just record the referene
    }
    void destruct()
    {
        free((char*)fName);
        // we don't own family, so just ignore our reference
    }
};

// we use atomic_inc to grow this for each typeface we create
static int32_t gUniqueFontID;

// this is the mutex that protects these globals
static SkMutex gFamilyMutex;
static FamilyRec* gFamilyHead;
static SkTDArray<NameFamilyPair> gNameList;

struct FamilyRec {
    FamilyRec*  fNext;
    SkTypeface* fFaces[4];
    
    FamilyRec()
    {
        fNext = gFamilyHead;
        memset(fFaces, 0, sizeof(fFaces));
        gFamilyHead = this;
    }
};

static SkTypeface* find_best_face(const FamilyRec* family,
                                  SkTypeface::Style style)
{
    SkTypeface* const* faces = family->fFaces;

    if (faces[style] != NULL) { // exact match
        return faces[style];
    }
    // look for a matching bold
    style = (SkTypeface::Style)(style ^ SkTypeface::kItalic);
    if (faces[style] != NULL) {
        return faces[style];
    }
    // look for the plain
    if (faces[SkTypeface::kNormal] != NULL) {
        return faces[SkTypeface::kNormal];
    }
    // look for anything
    for (int i = 0; i < 4; i++) {
        if (faces[i] != NULL) {
            return faces[i];
        }
    }
    // should never get here, since the faces list should not be empty
    SkASSERT(!"faces list is empty");
    return NULL;
}

static FamilyRec* find_family(const SkTypeface* member)
{
    FamilyRec* curr = gFamilyHead;
    while (curr != NULL) {
        for (int i = 0; i < 4; i++) {
            if (curr->fFaces[i] == member) {
                return curr;
            }
        }
        curr = curr->fNext;
    }
    return NULL;
}

/*  Returns the matching typeface, or NULL. If a typeface is found, its refcnt
    is not modified.
 */
static SkTypeface* find_from_uniqueID(uint32_t uniqueID)
{
    FamilyRec* curr = gFamilyHead;
    while (curr != NULL) {
        for (int i = 0; i < 4; i++) {
            SkTypeface* face = curr->fFaces[i];
            if (face != NULL && face->uniqueID() == uniqueID) {
                return face;
            }
        }
        curr = curr->fNext;
    }
    return NULL;
}

/*  Remove reference to this face from its family. If the resulting family
    is empty (has no faces), return that family, otherwise return NULL
*/
static FamilyRec* remove_from_family(const SkTypeface* face)
{
    FamilyRec* family = find_family(face);
    SkASSERT(family->fFaces[face->style()] == face);
    family->fFaces[face->style()] = NULL;
    
    for (int i = 0; i < 4; i++) {
        if (family->fFaces[i] != NULL) {    // family is non-empty
            return NULL;
        }
    }
    return family;  // return the empty family
}

// maybe we should make FamilyRec be doubly-linked
static void detach_and_delete_family(FamilyRec* family)
{
    FamilyRec* curr = gFamilyHead;
    FamilyRec* prev = NULL;

    while (curr != NULL) {
        FamilyRec* next = curr->fNext;
        if (curr == family) {
            if (prev == NULL) {
                gFamilyHead = next;
            } else {
                prev->fNext = next;
            }
            SkDELETE(family);
            return;
        }
        prev = curr;
        curr = next;
    }
    SkASSERT(!"Yikes, couldn't find family in our list to remove/delete");
}

static SkTypeface* find_typeface(const char name[], SkTypeface::Style style)
{
    NameFamilyPair* list = gNameList.begin();
    int             count = gNameList.count();
    
    int index = SkStrLCSearch(&list[0].fName, count, name, sizeof(list[0]));

    if (index >= 0) {
        return find_best_face(list[index].fFamily, style);
    }
    return NULL;
}

static SkTypeface* find_typeface(const SkTypeface* familyMember,
                                 SkTypeface::Style style)
{
    const FamilyRec* family = find_family(familyMember);
    return family ? find_best_face(family, style) : NULL;
}

static void add_name(const char name[], FamilyRec* family)
{
    SkAutoAsciiToLC tolc(name);
    name = tolc.lc();

    NameFamilyPair* list = gNameList.begin();
    int             count = gNameList.count();
    
    int index = SkStrLCSearch(&list[0].fName, count, name, sizeof(list[0]));

    if (index < 0) {
        list = gNameList.insert(~index);
        list->construct(name, family);
    }
}

static void remove_from_names(FamilyRec* emptyFamily)
{
#ifdef SK_DEBUG
    for (int i = 0; i < 4; i++) {
        SkASSERT(emptyFamily->fFaces[i] == NULL);
    }
#endif

    SkTDArray<NameFamilyPair>& list = gNameList;
    
    // must go backwards when removing
    for (int i = list.count() - 1; i >= 0; --i) {
        NameFamilyPair* pair = &list[i];
        if (pair->fFamily == emptyFamily) {
            pair->destruct();
            list.remove(i);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

class FamilyTypeface : public SkTypeface {
public:
    FamilyTypeface(Style style, bool sysFont, SkTypeface* familyMember)
        : SkTypeface(style, sk_atomic_inc(&gUniqueFontID) + 1)
    {
        fIsSysFont = sysFont;
        
        SkAutoMutexAcquire  ac(gFamilyMutex);
        
        FamilyRec* rec = NULL;
        if (familyMember) {
            rec = find_family(familyMember);
            SkASSERT(rec);
        } else {
            rec = SkNEW(FamilyRec);
        }
        rec->fFaces[style] = this;
    }
    
    virtual ~FamilyTypeface()
    {
        SkAutoMutexAcquire  ac(gFamilyMutex);
        
        // remove us from our family. If the family is now empty, we return
        // that and then remove that family from the name list
        FamilyRec* family = remove_from_family(this);
        if (NULL != family) {
            remove_from_names(family);
            detach_and_delete_family(family);
        }
    }
    
    bool isSysFont() const { return fIsSysFont; }
    
    virtual SkStream* openStream() = 0;
    virtual const char* getUniqueString() const = 0;
    
private:
    bool    fIsSysFont;
    
    typedef SkTypeface INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class StreamTypeface : public FamilyTypeface {
public:
    StreamTypeface(Style style, bool sysFont, SkTypeface* familyMember,
                   SkStream* stream)
        : INHERITED(style, sysFont, familyMember)
    {
        SkASSERT(stream);
        stream->ref();
        fStream = stream;
    }
    virtual ~StreamTypeface() {
        fStream->unref();
    }
    
    // overrides
    virtual SkStream* openStream() {
        // we just ref our existing stream, since the caller will call unref()
        // when they are through
        fStream->ref();
        return fStream;
    }
    virtual const char* getUniqueString() const { return NULL; }

private:
    SkStream* fStream;
    
    typedef FamilyTypeface INHERITED;
};

class FileTypeface : public FamilyTypeface {
public:
    FileTypeface(Style style, bool sysFont, SkTypeface* familyMember,
                 const char path[])
    : INHERITED(style, sysFont, familyMember)
    {
        SkString fullpath;
        
        if (sysFont) {
            GetFullPathForSysFonts(&fullpath, path);
            path = fullpath.c_str();
        }
        fPath.set(path);
    }
    
    // overrides
    virtual SkStream* openStream()
    {
        SkStream* stream = SkNEW_ARGS(SkMMAPStream, (fPath.c_str()));

        // check for failure
        if (stream->getLength() <= 0) {
            SkDELETE(stream);
            // maybe MMAP isn't supported. try FILE
            stream = SkNEW_ARGS(SkFILEStream, (fPath.c_str()));
            if (stream->getLength() <= 0) {
                SkDELETE(stream);
                stream = NULL;
            }
        }
        return stream;
    }
    virtual const char* getUniqueString() const {
        const char* str = strrchr(fPath.c_str(), '/');
        if (str) {
            str += 1;   // skip the '/'
        }
        return str;
    }

private:
    SkString fPath;
    
    typedef FamilyTypeface INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool get_name_and_style(const char path[], SkString* name,
                               SkTypeface::Style* style)
{
    SkString        fullpath;
    GetFullPathForSysFonts(&fullpath, path);

    SkMMAPStream stream(fullpath.c_str());
    if (stream.getLength() > 0) {
        *style = find_name_and_style(&stream, name);
        return true;
    }
    else {
        SkFILEStream stream(fullpath.c_str());
        if (stream.getLength() > 0) {
            *style = find_name_and_style(&stream, name);
            return true;
        }
    }

    SkDebugf("---- failed to open <%s> as a font\n", fullpath.c_str());
    return false;
}

struct FontInitRec {
    const char*         fFileName;
    const char* const*  fNames;     // null-terminated list
};

static const char* gSansNames[] = {
    "sans-serif", "arial", "helvetica", "tahoma", "verdana", NULL
};

static const char* gSerifNames[] = {
    "serif", "times", "times new roman", "palatino", "georgia", "baskerville",
    "goudy", "fantasy", "cursive", "ITC Stone Serif", NULL
};

static const char* gMonoNames[] = {
    "monospace", "courier", "courier new", "monaco", NULL
};

static const char* gFBNames[] = { NULL };

/*  Fonts must be grouped by family, with the first font in a family having the
    list of names (even if that list is empty), and the following members having
    null for the list. The names list must be NULL-terminated
*/
static const FontInitRec gSystemFonts[] = {
    { "DroidSans.ttf",              gSansNames  },
    { "DroidSans-Bold.ttf",         NULL        },
    { "DroidSerif-Regular.ttf",     gSerifNames },
    { "DroidSerif-Bold.ttf",        NULL        },
    { "DroidSerif-Italic.ttf",      NULL        },
    { "DroidSerif-BoldItalic.ttf",  NULL        },
    { "DroidSansMono.ttf",          gMonoNames  },
#ifdef NO_FALLBACK_FONT
    { "DroidSans.ttf",              gFBNames    }
#else
    { "DroidSansFallback.ttf",      gFBNames    }
#endif
};

#define DEFAULT_NAMES   gSansNames

// these globals are assigned (once) by load_system_fonts()
static SkTypeface* gFallBackTypeface;
static FamilyRec* gDefaultFamily;
static SkTypeface* gDefaultNormal;

static void load_system_fonts()
{
    // check if we've already be called
    if (NULL != gDefaultNormal) {
        return;
    }

    const FontInitRec* rec = gSystemFonts;
    SkTypeface* firstInFamily = NULL;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gSystemFonts); i++) {
        // if we're the first in a new family, clear firstInFamily
        if (rec[i].fNames != NULL) {
            firstInFamily = NULL;
        }
        
        SkString name;
        SkTypeface::Style style;
        
        if (!get_name_and_style(rec[i].fFileName, &name, &style)) {
            SkDebugf("------ can't load <%s> as a font\n", rec[i].fFileName);
            continue;
        }

        SkTypeface* tf = SkNEW_ARGS(FileTypeface,
                                    (style,
                                     true,  // system-font (cannot delete)
                                     firstInFamily, // what family to join
                                     rec[i].fFileName) // filename
                                    );

        if (rec[i].fNames != NULL) {
            firstInFamily = tf;
            const char* const* names = rec[i].fNames;

            // record the fallback if this is it
            if (names == gFBNames) {
                gFallBackTypeface = tf;
            }
            // record the default family if this is it
            if (names == DEFAULT_NAMES) {
                gDefaultFamily = find_family(tf);
            }
            // add the names to map to this family
            FamilyRec* family = find_family(tf);
            while (*names) {
                add_name(*names, family);
                names += 1;
            }
        }
        
    }

    // do this after all fonts are loaded. This is our default font, and it
    // acts as a sentinel so we only execute load_system_fonts() once
    gDefaultNormal = find_best_face(gDefaultFamily, SkTypeface::kNormal);
}

///////////////////////////////////////////////////////////////////////////////

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
    const char* name = ((FamilyTypeface*)face)->getUniqueString();

    stream->write8((uint8_t)face->style());

    if (NULL == name || 0 == *name) {
        stream->writePackedUInt(0);
//        SkDebugf("--- fonthost serialize null\n");
    } else {
        uint32_t len = strlen(name);
        stream->writePackedUInt(len);
        stream->write(name, len);
//      SkDebugf("--- fonthost serialize <%s> %d\n", name, face->style());
    }
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    load_system_fonts();

    int style = stream->readU8();

    int len = stream->readPackedUInt();
    if (len > 0) {
        SkString str;
        str.resize(len);
        stream->read(str.writable_str(), len);
        
        const FontInitRec* rec = gSystemFonts;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gSystemFonts); i++) {
            if (strcmp(rec[i].fFileName, str.c_str()) == 0) {
                // backup until we hit the fNames
                for (int j = i; j >= 0; --j) {
                    if (rec[j].fNames != NULL) {
                        return SkFontHost::CreateTypeface(NULL,
                                    rec[j].fNames[0], (SkTypeface::Style)style);
                    }
                }
            }
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       SkTypeface::Style style)
{
    load_system_fonts();

    SkAutoMutexAcquire  ac(gFamilyMutex);
    
    // clip to legal style bits
    style = (SkTypeface::Style)(style & SkTypeface::kBoldItalic);
    
    SkTypeface* tf = NULL;

    if (NULL != familyFace) {
        tf = find_typeface(familyFace, style);
    } else if (NULL != familyName) {
//        SkDebugf("======= familyName <%s>\n", familyName);
        tf = find_typeface(familyName, style);
    }

    if (NULL == tf) {
        tf = find_best_face(gDefaultFamily, style);
    }

    // we ref(), since the symantic is to return a new instance
    tf->ref();
    return tf;
}

bool SkFontHost::ValidFontID(uint32_t fontID)
{
    SkAutoMutexAcquire  ac(gFamilyMutex);
    
    return find_from_uniqueID(fontID) != NULL;
}

SkStream* SkFontHost::OpenStream(uint32_t fontID)
{
    SkAutoMutexAcquire  ac(gFamilyMutex);
    
    FamilyTypeface* tf = (FamilyTypeface*)find_from_uniqueID(fontID);
    SkStream* stream = tf ? tf->openStream() : NULL;

    if (stream && stream->getLength() == 0) {
        stream->unref();
        stream = NULL;
    }
    return stream;
}

SkScalerContext* SkFontHost::CreateFallbackScalerContext(
                                                const SkScalerContext::Rec& rec)
{
    load_system_fonts();

    SkAutoDescriptor    ad(sizeof(rec) + SkDescriptor::ComputeOverhead(1));
    SkDescriptor*       desc = ad.getDesc();
    
    desc->init();
    SkScalerContext::Rec* newRec =
        (SkScalerContext::Rec*)desc->addEntry(kRec_SkDescriptorTag,
                                              sizeof(rec), &rec);
    newRec->fFontID = gFallBackTypeface->uniqueID();
    desc->computeChecksum();
    
    return SkFontHost::CreateScalerContext(desc);
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream)
{
    if (NULL == stream || stream->getLength() <= 0) {
        return NULL;
    }
    
    SkString name;
    SkTypeface::Style style = find_name_and_style(stream, &name);

    return SkNEW_ARGS(StreamTypeface, (style, false, NULL, stream));
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[])
{
    SkStream* stream = SkNEW_ARGS(SkMMAPStream, (path));
    SkTypeface* face = SkFontHost::CreateTypefaceFromStream(stream);
    // since we created the stream, we let go of our ref() here
    stream->unref();
    return face;
}

///////////////////////////////////////////////////////////////////////////////

size_t SkFontHost::ShouldPurgeFontCache(size_t sizeAllocatedSoFar)
{
    if (sizeAllocatedSoFar > FONT_CACHE_MEMORY_BUDGET)
        return sizeAllocatedSoFar - FONT_CACHE_MEMORY_BUDGET;
    else
        return 0;   // nothing to do
}

