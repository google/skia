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
#include "SkOSFile.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkStream.h"
#include "SkThread.h"
#include "SkTSearch.h"
#include <stdio.h>

#define FONT_CACHE_MEMORY_BUDGET    (1 * 1024 * 1024)

#ifndef SK_FONT_FILE_PREFIX
    #define SK_FONT_FILE_PREFIX      "/usr/share/fonts/truetype/msttcorefonts/"
#endif

SkTypeface::Style find_name_and_style(SkStream* stream, SkString* name);

static void GetFullPathForSysFonts(SkString* full, const char name[])
{
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
                                  SkTypeface::Style style) {
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

static FamilyRec* find_family(const SkTypeface* member) {
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

static bool valid_uniqueID(uint32_t uniqueID) {
    FamilyRec* curr = gFamilyHead;
    while (curr != NULL) {
        for (int i = 0; i < 4; i++) {
            SkTypeface* face = curr->fFaces[i];
            if (face != NULL && face->uniqueID() == uniqueID) {
                return true;
            }
        }
        curr = curr->fNext;
    }
    return false;
}

/*  Remove reference to this face from its family. If the resulting family
 is empty (has no faces), return that family, otherwise return NULL
 */
static FamilyRec* remove_from_family(const SkTypeface* face) {
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
static void detach_and_delete_family(FamilyRec* family) {
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

static FamilyRec* find_familyrec(const char name[]) {
    const NameFamilyPair* list = gNameList.begin();    
    int index = SkStrLCSearch(&list[0].fName, gNameList.count(), name,
                              sizeof(list[0]));
    return index >= 0 ? list[index].fFamily : NULL;
}

static SkTypeface* find_typeface(const char name[], SkTypeface::Style style) {
    FamilyRec* rec = find_familyrec(name);
    return rec ? find_best_face(rec, style) : NULL;
}

static SkTypeface* find_typeface(const SkTypeface* familyMember,
                                 SkTypeface::Style style) {
    const FamilyRec* family = find_family(familyMember);
    return family ? find_best_face(family, style) : NULL;
}

static void add_name(const char name[], FamilyRec* family) {
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

static void remove_from_names(FamilyRec* emptyFamily) {
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
    FamilyTypeface(Style style, bool sysFont, FamilyRec* family)
    : SkTypeface(style, sk_atomic_inc(&gUniqueFontID) + 1) {
        fIsSysFont = sysFont;
        
        SkAutoMutexAcquire  ac(gFamilyMutex);
        
        if (NULL == family) {
            family = SkNEW(FamilyRec);
        }
        family->fFaces[style] = this;
        fFamilyRec = family;    // just record it so we can return it if asked
    }
    
    virtual ~FamilyTypeface() {
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
    FamilyRec* getFamily() const { return fFamilyRec; }
    
    virtual SkStream* openStream() = 0;
    virtual void closeStream(SkStream*) = 0;
    virtual const char* getUniqueString() const = 0;
    
private:
    FamilyRec*  fFamilyRec; // we don't own this, just point to it
    bool        fIsSysFont;
    
    typedef SkTypeface INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class StreamTypeface : public FamilyTypeface {
public:
    StreamTypeface(Style style, bool sysFont, FamilyRec* family,
                   SkStream* stream)
    : INHERITED(style, sysFont, family) {
        fStream = stream;
    }
    virtual ~StreamTypeface() {
        SkDELETE(fStream);
    }
    
    // overrides
    virtual SkStream* openStream() { return fStream; }
    virtual void closeStream(SkStream*) {}
    virtual const char* getUniqueString() const { return NULL; }
    
private:
    SkStream* fStream;
    
    typedef FamilyTypeface INHERITED;
};

class FileTypeface : public FamilyTypeface {
public:
    FileTypeface(Style style, bool sysFont, FamilyRec* family,
                 const char path[])
        : INHERITED(style, sysFont, family) {
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
    virtual void closeStream(SkStream* stream)
    {
        SkDELETE(stream);
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
                               SkTypeface::Style* style) {    
    SkMMAPStream stream(path);
    if (stream.getLength() > 0) {
        *style = find_name_and_style(&stream, name);
        return true;
    }
    else {
        SkFILEStream stream(path);
        if (stream.getLength() > 0) {
            *style = find_name_and_style(&stream, name);
            return true;
        }
    }
    
    SkDebugf("---- failed to open <%s> as a font\n", path);
    return false;
}

// these globals are assigned (once) by load_system_fonts()
static SkTypeface* gFallBackTypeface;
static FamilyRec* gDefaultFamily;
static SkTypeface* gDefaultNormal;

static void load_system_fonts() {
    // check if we've already be called
    if (NULL != gDefaultNormal) {
        return;
    }
    
    SkOSFile::Iter    iter(SK_FONT_FILE_PREFIX, ".ttf");
    SkString          name;

    while (iter.next(&name, false)) {
        SkString filename;
        GetFullPathForSysFonts(&filename, name.c_str());
//    while (filename.size() == 0) { filename.set("/usr/share/fonts/truetype/msttcorefonts/Arial.ttf");

        SkString realname;
        SkTypeface::Style style;
        
        if (!get_name_and_style(filename.c_str(), &realname, &style)) {
            SkDebugf("------ can't load <%s> as a font\n", filename.c_str());
            continue;
        }
        
//        SkDebugf("font: <%s> %d <%s>\n", realname.c_str(), style, filename.c_str());
        
        FamilyRec* family = find_familyrec(realname.c_str());
        // this constructor puts us into the global gFamilyHead llist
        FamilyTypeface* tf = SkNEW_ARGS(FileTypeface,
                                        (style,
                                         true,  // system-font (cannot delete)
                                         family, // what family to join
                                         filename.c_str()) // filename
                                        );

        if (NULL == family) {
            add_name(realname.c_str(), tf->getFamily());
        }
    }
    
    // do this after all fonts are loaded. This is our default font, and it
    // acts as a sentinel so we only execute load_system_fonts() once
    static const char* gDefaultNames[] = {
        "Arial", "Verdana", "Times New Roman", NULL
    };
    const char** names = gDefaultNames;
    while (*names) {
        SkTypeface* tf = find_typeface(*names++, SkTypeface::kNormal);
        if (tf) {
            gDefaultNormal = tf;
            break;
        }
    }
    // check if we found *something*
    if (NULL == gDefaultNormal) {
        if (NULL == gFamilyHead) {
            sk_throw();
        }
        for (int i = 0; i < 4; i++) {
            if ((gDefaultNormal = gFamilyHead->fFaces[i]) != NULL) {
                break;
            }
        }
    }
    if (NULL == gDefaultNormal) {
        sk_throw();
    }
    gFallBackTypeface = gDefaultNormal;    
    gDefaultFamily = find_family(gDefaultNormal);

//    SkDebugf("---- default %p head %p family %p\n", gDefaultNormal, gFamilyHead, gDefaultFamily);
}

///////////////////////////////////////////////////////////////////////////////

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
#if 0
    const char* name = ((FamilyTypeface*)face)->getUniqueString();
    
    stream->write8((uint8_t)face->getStyle());
    
    if (NULL == name || 0 == *name) {
        stream->writePackedUInt(0);
        //        SkDebugf("--- fonthost serialize null\n");
    } else {
        uint32_t len = strlen(name);
        stream->writePackedUInt(len);
        stream->write(name, len);
        //      SkDebugf("--- fonthost serialize <%s> %d\n", name, face->getStyle());
    }
#endif
    sk_throw();
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
#if 0
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
                        return SkFontHost::CreateTypeface(NULL, rec[j].fNames[0],
                                                          (SkTypeface::Style)style);
                    }
                }
            }
        }
    }
    return SkFontHost::CreateTypeface(NULL, NULL, (SkTypeface::Style)style);
#endif
    sk_throw();
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       SkTypeface::Style style) {
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
    
    return tf;
}

SkTypeface* SkFontHost::ValidFontID(uint32_t fontID) {
    SkAutoMutexAcquire  ac(gFamilyMutex);
    
    return valid_uniqueID(fontID);
}

SkStream* SkFontHost::OpenStream(uint32_t fontID) {
    FamilyTypeface* tf = (FamilyTypeface*)SkFontHost::ResolveTypeface(fontID);
    SkStream* stream = tf ? tf->openStream() : NULL;
    
    if (NULL == stream || stream->getLength() == 0) {
        delete stream;
        stream = NULL;
    }
    return stream;
}

void SkFontHost::CloseStream(uint32_t fontID, SkStream* stream) {
    FamilyTypeface* tf = (FamilyTypeface*)SkFontHost::ResolveTypeface(fontID);
    if (NULL != tf) {
        tf->closeStream(stream);
    }
}

SkScalerContext* SkFontHost::CreateFallbackScalerContext(
                                            const SkScalerContext::Rec& rec) {
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

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    if (NULL == stream || stream->getLength() <= 0) {
        SkDELETE(stream);
        return NULL;
    }
    
    SkString name;
    SkTypeface::Style style = find_name_and_style(stream, &name);
    
    return SkNEW_ARGS(StreamTypeface, (style, false, NULL, stream));
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    SkTypeface* face = NULL;
    SkFILEStream* stream = SkNEW_ARGS(SkFILEStream, (path));

    if (stream->isValid()) {
        return CreateTypeface(stream);
    }
    stream->unref();
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

size_t SkFontHost::ShouldPurgeFontCache(size_t sizeAllocatedSoFar) {
    if (sizeAllocatedSoFar > FONT_CACHE_MEMORY_BUDGET)
        return sizeAllocatedSoFar - FONT_CACHE_MEMORY_BUDGET;
    else
        return 0;   // nothing to do
}

