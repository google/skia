/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontDescriptor.h"
#include "SkFontHost.h"
#include "SkFontHost_FreeType_common.h"
#include "SkFontStream.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"

SK_DECLARE_STATIC_MUTEX(gFontConfigInterfaceMutex);
static SkFontConfigInterface* gFontConfigInterface;

SkFontConfigInterface* SkFontConfigInterface::RefGlobal() {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    return SkSafeRef(gFontConfigInterface);
}

SkFontConfigInterface* SkFontConfigInterface::SetGlobal(SkFontConfigInterface* fc) {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    SkRefCnt_SafeAssign(gFontConfigInterface, fc);
    return fc;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// convenience function to create the direct interface if none is installed.
extern SkFontConfigInterface* SkCreateDirectFontConfigInterface();

static SkFontConfigInterface* RefFCI() {
    for (;;) {
        SkFontConfigInterface* fci = SkFontConfigInterface::RefGlobal();
        if (fci) {
            return fci;
        }
        fci = SkFontConfigInterface::GetSingletonDirectInterface();
        SkFontConfigInterface::SetGlobal(fci)->unref();
    }
}

class FontConfigTypeface : public SkTypeface_FreeType {
    SkFontConfigInterface::FontIdentity fIdentity;
    SkString fFamilyName;
    SkStream* fLocalStream;

public:
    FontConfigTypeface(Style style,
                       const SkFontConfigInterface::FontIdentity& fi,
                       const SkString& familyName)
            : INHERITED(style, SkTypefaceCache::NewFontID(), false)
            , fIdentity(fi)
            , fFamilyName(familyName)
            , fLocalStream(NULL) {}

    FontConfigTypeface(Style style, SkStream* localStream)
            : INHERITED(style, SkTypefaceCache::NewFontID(), false) {
        // we default to empty fFamilyName and fIdentity
        fLocalStream = localStream;
        SkSafeRef(localStream);
    }

    virtual ~FontConfigTypeface() {
        SkSafeUnref(fLocalStream);
    }

    const SkFontConfigInterface::FontIdentity& getIdentity() const {
        return fIdentity;
    }

    const char* getFamilyName() const { return fFamilyName.c_str(); }
    SkStream*   getLocalStream() const { return fLocalStream; }

    bool isFamilyName(const char* name) const {
        return fFamilyName.equals(name);
    }

protected:
    friend class SkFontHost;    // hack until we can make public versions

    virtual int onGetTableTags(SkFontTableTag tags[]) const SK_OVERRIDE;
    virtual size_t onGetTableData(SkFontTableTag, size_t offset,
                                  size_t length, void* data) const SK_OVERRIDE;
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool*) const SK_OVERRIDE;
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE;

private:
    typedef SkTypeface_FreeType INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

struct FindRec {
    FindRec(const char* name, SkTypeface::Style style)
        : fFamilyName(name)  // don't need to make a deep copy
        , fStyle(style) {}

    const char* fFamilyName;
    SkTypeface::Style fStyle;
};

static bool find_proc(SkTypeface* face, SkTypeface::Style style, void* ctx) {
    FontConfigTypeface* fci = (FontConfigTypeface*)face;
    const FindRec* rec = (const FindRec*)ctx;

    return rec->fStyle == style && fci->isFamilyName(rec->fFamilyName);
}

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       SkTypeface::Style style) {
    SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
    if (NULL == fci.get()) {
        return NULL;
    }

    if (familyFace) {
        FontConfigTypeface* fct = (FontConfigTypeface*)familyFace;
        familyName = fct->getFamilyName();
    }

    FindRec rec(familyName, style);
    SkTypeface* face = SkTypefaceCache::FindByProcAndRef(find_proc, &rec);
    if (face) {
//        SkDebugf("found cached face <%s> <%s> %p [%d]\n", familyName, ((FontConfigTypeface*)face)->getFamilyName(), face, face->getRefCnt());
        return face;
    }

    SkFontConfigInterface::FontIdentity indentity;
    SkString                            outFamilyName;
    SkTypeface::Style                   outStyle;

    if (!fci->matchFamilyName(familyName, style,
                              &indentity, &outFamilyName, &outStyle)) {
        return NULL;
    }

    face = SkNEW_ARGS(FontConfigTypeface, (outStyle, indentity, outFamilyName));
    SkTypefaceCache::Add(face, style);
//    SkDebugf("add face <%s> <%s> %p [%d]\n", familyName, outFamilyName.c_str(), face, face->getRefCnt());
    return face;
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    if (!stream) {
        return NULL;
    }
    const size_t length = stream->getLength();
    if (!length) {
        return NULL;
    }
    if (length >= 1024 * 1024 * 1024) {
        return NULL;  // don't accept too large fonts (>= 1GB) for safety.
    }

    // TODO should the caller give us the style?
    SkTypeface::Style style = SkTypeface::kNormal;
    SkTypeface* face = SkNEW_ARGS(FontConfigTypeface, (style, stream));
    SkTypefaceCache::Add(face, style);
    return face;
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path));
    return stream.get() ? CreateTypefaceFromStream(stream) : NULL;
}

///////////////////////////////////////////////////////////////////////////////

SkStream* FontConfigTypeface::onOpenStream(int* ttcIndex) const {
    SkStream* stream = this->getLocalStream();
    if (stream) {
        // TODO: fix issue 1176.
        // As of now open_stream will return a stream and unwind it, but the
        // SkStream is not thread safe, and if two threads use the stream they
        // may collide and print preview for example could still fail,
        // or there could be some failures in rendering if this stream is used
        // there.
        stream->rewind();
        stream->ref();
        // should have been provided by CreateFromStream()
        *ttcIndex = 0;
    } else {
        SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
        if (NULL == fci.get()) {
            return NULL;
        }
        stream = fci->openStream(this->getIdentity());
        *ttcIndex = this->getIdentity().fTTCIndex;
    }
    return stream;
}

int FontConfigTypeface::onGetTableTags(SkFontTableTag tags[]) const {
    int ttcIndex;
    SkAutoTUnref<SkStream> stream(this->openStream(&ttcIndex));
    return stream.get()
                ? SkFontStream::GetTableTags(stream, ttcIndex, tags)
                : 0;
}

size_t FontConfigTypeface::onGetTableData(SkFontTableTag tag, size_t offset,
                                  size_t length, void* data) const {
    int ttcIndex;
    SkAutoTUnref<SkStream> stream(this->openStream(&ttcIndex));
    return stream.get()
                ? SkFontStream::GetTableData(stream, ttcIndex,
                                             tag, offset, length, data)
                : 0;
}

void FontConfigTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                             bool* isLocalStream) const {
    desc->setFamilyName(this->getFamilyName());
    *isLocalStream = SkToBool(this->getLocalStream());
}

///////////////////////////////////////////////////////////////////////////////

// look for the last substring after a '/' and return that, or return null.
static const char* find_just_name(const char* str) {
    const char* last = strrchr(str, '/');
    return last ? last + 1 : NULL;
}

static bool is_lower(char c) {
    return c >= 'a' && c <= 'z';
}

#include "SkFontMgr.h"
#include <fontconfig/fontconfig.h>

static int get_int(FcPattern* pattern, const char field[]) {
    int value;
    if (FcPatternGetInteger(pattern, field, 0, &value) != FcResultMatch) {
        value = SK_MinS32;
    }
    return value;
}

static const char* get_name(FcPattern* pattern, const char field[]) {
    const char* name;
    if (FcPatternGetString(pattern, field, 0, (FcChar8**)&name) != FcResultMatch) {
        name = "";
    }
    return name;
}

static bool valid_pattern(FcPattern* pattern) {
    FcBool is_scalable;
    if (FcPatternGetBool(pattern, FC_SCALABLE, 0, &is_scalable) != FcResultMatch || !is_scalable) {
        return false;
    }

    // fontconfig can also return fonts which are unreadable
    const char* c_filename = get_name(pattern, FC_FILE);
    if (0 == *c_filename) {
        return false;
    }
    if (access(c_filename, R_OK) != 0) {
        return false;
    }
    return true;
}

static bool match_name(FcPattern* pattern, const char family_name[]) {
    return !strcasecmp(family_name, get_name(pattern, FC_FAMILY));
}

static FcPattern** MatchFont(FcFontSet* font_set,
                             const char post_config_family[],
                             int* count) {
  // Older versions of fontconfig have a bug where they cannot select
  // only scalable fonts so we have to manually filter the results.
  
    FcPattern** iter = font_set->fonts;
    FcPattern** stop = iter + font_set->nfont;
    // find the first good match
    for (; iter < stop; ++iter) {
        if (valid_pattern(*iter)) {
            break;
        }
    }

    if (iter == stop || !match_name(*iter, post_config_family)) {
        return NULL;
    }

    FcPattern** firstIter = iter++;
    for (; iter < stop; ++iter) {
        if (!valid_pattern(*iter) || !match_name(*iter, post_config_family)) {
            break;
        }
    }
    
    *count = iter - firstIter;
    return firstIter;
}

class SkFontStyleSet_FC : public SkFontStyleSet {
public:
    SkFontStyleSet_FC(FcPattern** matches, int count);
    virtual ~SkFontStyleSet_FC();

    virtual int count() SK_OVERRIDE { return fRecCount; }
    virtual void getStyle(int index, SkFontStyle*, SkString* style) SK_OVERRIDE;
    virtual SkTypeface* createTypeface(int index) SK_OVERRIDE;
    virtual SkTypeface* matchStyle(const SkFontStyle& pattern) SK_OVERRIDE;
    
private:
    struct Rec {
        SkString    fStyleName;
        SkString    fFileName;
        SkFontStyle fStyle;
    };
    Rec* fRecs;
    int  fRecCount;
};

static int map_range(int value,
                     int old_min, int old_max, int new_min, int new_max) {
    SkASSERT(old_min < old_max);
    SkASSERT(new_min < new_max);    
    return new_min + SkMulDiv(value - old_min,
                              new_max - new_min, old_max - old_min);
}

static SkFontStyle make_fontconfig_style(FcPattern* match) {
    int weight = get_int(match, FC_WEIGHT);
    int width = get_int(match, FC_WIDTH);
    int slant = get_int(match, FC_SLANT);
//    SkDebugf("old weight %d new weight %d\n", weight, map_range(weight, 0, 80, 0, 400));
   
    // fontconfig weight seems to be 0..200 or so, so we remap it here
    weight = map_range(weight, 0, 80, 0, 400);
    width = map_range(width, 0, 200, 0, 9);
    return SkFontStyle(weight, width, slant > 0 ? SkFontStyle::kItalic_Slant
                                                : SkFontStyle::kUpright_Slant);
}

SkFontStyleSet_FC::SkFontStyleSet_FC(FcPattern** matches, int count) {
    fRecCount = count;
    fRecs = SkNEW_ARRAY(Rec, count);
    for (int i = 0; i < count; ++i) {
        fRecs[i].fStyleName.set(get_name(matches[i], FC_STYLE));
        fRecs[i].fFileName.set(get_name(matches[i], FC_FILE));
        fRecs[i].fStyle = make_fontconfig_style(matches[i]);
        
//        SkDebugf("%s [%d %d %d]\n", fRecs[i].fFileName.c_str(), fRecs[i].fStyle.weight(), fRecs[i].fStyle.width(), fRecs[i].fStyle.isItalic());
    }
}

SkFontStyleSet_FC::~SkFontStyleSet_FC() {
    SkDELETE_ARRAY(fRecs);
}

void SkFontStyleSet_FC::getStyle(int index, SkFontStyle* style,
                                 SkString* styleName) {
    SkASSERT((unsigned)index < (unsigned)fRecCount);
    if (style) {
        *style = fRecs[index].fStyle;
    }
    if (styleName) {
        *styleName = fRecs[index].fStyleName;
    }
}

SkTypeface* SkFontStyleSet_FC::createTypeface(int index) {
    return NULL;
}

SkTypeface* SkFontStyleSet_FC::matchStyle(const SkFontStyle& pattern) {
    return NULL;
}

static bool find_name(const SkTDArray<const char*>& array, const char* name) {
    for (int i = 0; i < array.count(); ++i) {
        if (0 == strcmp(array[i], name)) {
            return true;
        }
    }
    return false;
}

class SkFontMgr_fontconfig : public SkFontMgr {
    SkAutoTUnref<SkFontConfigInterface> fFCI;

    int fFamilyCount;
    SkString* fFamilyNames;

    void init() {
        if (fFamilyCount >= 0) {
            return;
        }

        FcPattern* pat = FcPatternCreate();
        FcObjectSet* os = FcObjectSetBuild (FC_FAMILY, (char *) 0);
        FcFontSet* fs = FcFontList(NULL, pat, os);

        SkTDArray<const char*> familyNames;
        familyNames.setReserve(fs->nfont);
        for (int i=0; fs && i < fs->nfont; ++i) {
            FcPattern* match = fs->fonts[i];
            const char* famName = get_name(match, FC_FAMILY);
            if (!find_name(familyNames, famName)) {
                *familyNames.append() = famName;
            }
//            SkDebugf("[%d %d] %s %s %x\n", i, fs->nfont, get_name(match, FC_FILE),
//                     get_name(match, FC_FAMILY), get_name(match, FC_FAMILY));
        }
        
        fFamilyCount = familyNames.count();
        fFamilyNames = SkNEW_ARRAY(SkString, fFamilyCount);
        for (int i = 0; i < fFamilyCount; ++i) {
            fFamilyNames[i].set(familyNames[i]);
        }

        if (fs) FcFontSetDestroy(fs);
        FcPatternDestroy(pat);
    }

public:
    SkFontMgr_fontconfig(SkFontConfigInterface* fci) : fFCI(fci) {
        fFamilyCount = -1;
    }
    
    virtual ~SkFontMgr_fontconfig() {
        SkDELETE_ARRAY(fFamilyNames);
    }

protected:
    virtual int onCountFamilies() { this->init(); return fFamilyCount; }

    virtual void onGetFamilyName(int index, SkString* familyName) {
        this->init();
        SkASSERT((unsigned)index < (unsigned)fFamilyCount);
        *familyName = fFamilyNames[index];
    }

    virtual SkFontStyleSet* onCreateStyleSet(int index) {
        this->init();
        SkASSERT((unsigned)index < (unsigned)fFamilyCount);
        return this->onMatchFamily(fFamilyNames[index].c_str());
    }

    virtual SkFontStyleSet* onMatchFamily(const char familyName[]) {
        this->init();

        FcPattern* pattern = FcPatternCreate();

        FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)familyName);
#if 0
        FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);
#endif
        FcConfigSubstitute(NULL, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);

        const char* post_config_family = get_name(pattern, FC_FAMILY);

        FcResult result;
        FcFontSet* font_set = FcFontSort(0, pattern, 0, 0, &result);
        if (!font_set) {
            FcPatternDestroy(pattern);
            return NULL;
        }

        int count;
        FcPattern** match = MatchFont(font_set, post_config_family, &count);
        if (!match) {
            FcPatternDestroy(pattern);
            FcFontSetDestroy(font_set);
            return NULL;
        }

        FcPatternDestroy(pattern);

        SkTDArray<FcPattern*> trimmedMatches;
        for (int i = 0; i < count; ++i) {
            const char* justName = find_just_name(get_name(match[i], FC_FILE));
            if (!is_lower(*justName)) {
                *trimmedMatches.append() = match[i];
            }
            #if 0
            printf("[%d:%d] %s %s [%d %d %d]\n", i, count,
                   get_name(match[i], FC_STYLE), get_name(match[i], FC_FILE),
                   get_int(match[i], FC_WEIGHT), get_int(match[i], FC_WIDTH),
                   get_int(match[i], FC_SLANT));
            #endif
        }

        SkFontStyleSet_FC* sset = SkNEW_ARGS(SkFontStyleSet_FC,
                                             (trimmedMatches.begin(),
                                              trimmedMatches.count()));
        return sset;
    }

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle&) { return NULL; }
    virtual SkTypeface* onMatchFaceStyle(const SkTypeface*,
                                         const SkFontStyle&) { return NULL; }

    virtual SkTypeface* onCreateFromData(SkData*, int ttcIndex) { return NULL; }
    virtual SkTypeface* onCreateFromStream(SkStream*, int ttcIndex) {
        return NULL;
    }
    virtual SkTypeface* onCreateFromFile(const char path[], int ttcIndex) {
        return NULL;
    }
};

SkFontMgr* SkFontMgr::Factory() {
    SkFontConfigInterface* fci = RefFCI();
    return fci ? SkNEW_ARGS(SkFontMgr_fontconfig, (fci)) : NULL;
}


