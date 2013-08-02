
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkFontHost.h"
#include "SkFontHost_FreeType_common.h"
#include "SkFontDescriptor.h"
#include "SkDescriptor.h"
#include "SkOSFile.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkStream.h"
#include "SkThread.h"
#include "SkTSearch.h"

#ifndef SK_FONT_FILE_PREFIX
    #define SK_FONT_FILE_PREFIX "/usr/share/fonts/truetype/"
#endif
#ifndef SK_FONT_FILE_DIR_SEPERATOR
    #define SK_FONT_FILE_DIR_SEPERATOR "/"
#endif

bool find_name_and_attributes(SkStream* stream, SkString* name,
                              SkTypeface::Style* style, bool* isFixedPitch);

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
SK_DECLARE_STATIC_MUTEX(gFamilyMutex);
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
    SkDEBUGFAIL("faces list is empty");
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
    SkDEBUGFAIL("Yikes, couldn't find family in our list to remove/delete");
}

static const char* find_family_name(const SkTypeface* familyMember) {
    const FamilyRec* familyRec = find_family(familyMember);
    for (int i = 0; i < gNameList.count(); i++) {
        if (gNameList[i].fFamily == familyRec) {
            return gNameList[i].fName;
        }
    }
    return NULL;
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

class FamilyTypeface : public SkTypeface_FreeType {
public:
    FamilyTypeface(Style style, bool sysFont, FamilyRec* family, bool isFixedPitch)
    : INHERITED(style, sk_atomic_inc(&gUniqueFontID) + 1, isFixedPitch) {
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

    virtual const char* getUniqueString() const = 0;

protected:
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool*) const SK_OVERRIDE;
    virtual SkTypeface* onRefMatchingStyle(Style styleBits) const SK_OVERRIDE;

private:
    FamilyRec*  fFamilyRec; // we don't own this, just point to it
    bool        fIsSysFont;

    typedef SkTypeface_FreeType INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

/* This subclass is just a place holder for when we have no fonts available.
    It exists so that our globals (e.g. gFamilyHead) that expect *something*
    will not be null.
 */
class EmptyTypeface : public FamilyTypeface {
public:
    EmptyTypeface() : INHERITED(SkTypeface::kNormal, true, NULL, false) {}

    virtual const char* getUniqueString() SK_OVERRIDE const { return NULL; }

protected:
    virtual SkStream* onOpenStream(int*) const SK_OVERRIDE { return NULL; }

private:
    typedef FamilyTypeface INHERITED;
};

class StreamTypeface : public FamilyTypeface {
public:
    StreamTypeface(Style style, bool sysFont, FamilyRec* family,
                   SkStream* stream, bool isFixedPitch)
    : INHERITED(style, sysFont, family, isFixedPitch) {
        stream->ref();
        fStream = stream;
    }
    virtual ~StreamTypeface() {
        fStream->unref();
    }

    virtual const char* getUniqueString() const SK_OVERRIDE { return NULL; }

protected:
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        *ttcIndex = 0;
        fStream->ref();
        return fStream;
    }

private:
    SkStream* fStream;

    typedef FamilyTypeface INHERITED;
};

class FileTypeface : public FamilyTypeface {
public:
    FileTypeface(Style style, bool sysFont, FamilyRec* family,
                 const char path[], bool isFixedPitch)
        : INHERITED(style, sysFont, family, isFixedPitch) {
        fPath.set(path);
    }

    virtual const char* getUniqueString() const SK_OVERRIDE {
        const char* str = strrchr(fPath.c_str(), '/');
        if (str) {
            str += 1;   // skip the '/'
        }
        return str;
    }

protected:
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        *ttcIndex = 0;
        return SkStream::NewFromFile(fPath.c_str());
    }

private:
    SkString fPath;

    typedef FamilyTypeface INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool get_name_and_style(const char path[], SkString* name,
                               SkTypeface::Style* style, bool* isFixedPitch) {
    SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path));
    if (stream.get()) {
        return find_name_and_attributes(stream, name, style, isFixedPitch);
    } else {
        SkDebugf("---- failed to open <%s> as a font\n", path);
        return false;
    }
}

// these globals are assigned (once) by load_system_fonts()
static SkTypeface* gFallBackTypeface;
static FamilyRec* gDefaultFamily;
static SkTypeface* gDefaultNormal;

static void load_directory_fonts(const SkString& directory, unsigned int* count) {
    SkOSFile::Iter  iter(directory.c_str(), ".ttf");
    SkString        name;

    while (iter.next(&name, false)) {
        SkString filename(directory);
        filename.append(name);

        bool isFixedPitch;
        SkString realname;
        SkTypeface::Style style = SkTypeface::kNormal; // avoid uninitialized warning

        if (!get_name_and_style(filename.c_str(), &realname, &style, &isFixedPitch)) {
            SkDebugf("------ can't load <%s> as a font\n", filename.c_str());
            continue;
        }

        FamilyRec* family = find_familyrec(realname.c_str());
        if (family && family->fFaces[style]) {
            continue;
        }

        // this constructor puts us into the global gFamilyHead llist
        FamilyTypeface* tf = SkNEW_ARGS(FileTypeface,
                                        (style,
                                         true,  // system-font (cannot delete)
                                         family, // what family to join
                                         filename.c_str(),
                                         isFixedPitch) // filename
                                        );

        if (NULL == family) {
            add_name(realname.c_str(), tf->getFamily());
        }
        *count += 1;
    }

    SkOSFile::Iter  dirIter(directory.c_str());
    while (dirIter.next(&name, true)) {
        if (name.startsWith(".")) {
            continue;
        }
        SkString dirname(directory);
        dirname.append(name);
        dirname.append(SK_FONT_FILE_DIR_SEPERATOR);
        load_directory_fonts(dirname, count);
    }
}

static void load_system_fonts() {
    // check if we've already be called
    if (NULL != gDefaultNormal) {
        return;
    }

    SkString baseDirectory(SK_FONT_FILE_PREFIX);
    unsigned int count = 0;
    load_directory_fonts(baseDirectory, &count);

    if (0 == count) {
        SkNEW(EmptyTypeface);
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
}

///////////////////////////////////////////////////////////////////////////////

void FamilyTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                         bool* isLocalStream) const {
    desc->setFamilyName(find_family_name(this));
    desc->setFontFileName(this->getUniqueString());
    *isLocalStream = !this->isSysFont();
}

static SkTypeface* create_typeface(const SkTypeface* familyFace,
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

    SkSafeRef(tf);
    return tf;
}

SkTypeface* FamilyTypeface::onRefMatchingStyle(Style style) const {
    return create_typeface(this, NULL, style);
}

///////////////////////////////////////////////////////////////////////////////

#ifndef SK_FONTHOST_USES_FONTMGR

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       SkTypeface::Style style) {
    return create_typeface(familyFace, NULL, style);
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    if (NULL == stream || stream->getLength() <= 0) {
        SkDELETE(stream);
        return NULL;
    }

    bool isFixedPitch;
    SkTypeface::Style style;
    if (find_name_and_attributes(stream, NULL, &style, &isFixedPitch)) {
        return SkNEW_ARGS(StreamTypeface, (style, false, NULL, stream, isFixedPitch));
    } else {
        return NULL;
    }
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path));
    return stream.get() ? CreateTypefaceFromStream(stream) : NULL;
}

#endif

///////////////////////////////////////////////////////////////////////////////

#include "SkFontMgr.h"

SkFontMgr* SkFontMgr::Factory() {
    // todo
    return NULL;
}
