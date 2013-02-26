/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Derived from chromium's skia/ext/SkFontHost_fontconfig.cpp */

#include <map>
#include <string>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "SkFontConfigInterface.h"
#include "SkFontDescriptor.h"
#include "SkFontHost.h"
#include "SkStream.h"
#include "SkTypeface.h"

extern SkFontConfigInterface* SkCreateDirectFontConfigInterface();

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

static SkFontConfigInterface* RefFCI() {
    for (;;) {
        SkFontConfigInterface* fci = SkFontConfigInterface::RefGlobal();
        if (fci) {
            return fci;
        }
        fci = SkCreateDirectFontConfigInterface();
        SkFontConfigInterface::SetGlobal(fci)->unref();
    }
}

///////////////////////////////////////////////////////////////////////////////

SK_DECLARE_STATIC_MUTEX(global_remote_font_map_lock);
static std::map<uint32_t, std::pair<uint8_t*, size_t> >* global_remote_fonts;

// Initialize the map declared above. Note that its corresponding mutex must be
// locked before calling this function.
static void AllocateGlobalRemoteFontsMapOnce() {
    if (!global_remote_fonts) {
        global_remote_fonts =
            new std::map<uint32_t, std::pair<uint8_t*, size_t> >();
    }
}

static unsigned global_next_remote_font_id;

// This is the maximum size of the font cache.
static const unsigned kFontCacheMemoryBudget = 2 * 1024 * 1024;  // 2MB

// UniqueIds are encoded as (filefaceid << 8) | style
// For system fonts, filefaceid = (fileid << 4) | face_index.
// For remote fonts, filefaceid = fileid.

static unsigned UniqueIdToFileFaceId(unsigned uniqueid)
{
    return uniqueid >> 8;
}

static SkTypeface::Style UniqueIdToStyle(unsigned uniqueid)
{
    return static_cast<SkTypeface::Style>(uniqueid & 0xff);
}

static unsigned FileFaceIdAndStyleToUniqueId(unsigned filefaceid,
                                             SkTypeface::Style style)
{
    SkASSERT((style & 0xff) == style);
    return (filefaceid << 8) | static_cast<int>(style);
}

static const unsigned kRemoteFontMask = 0x00800000u;

static bool IsRemoteFont(unsigned filefaceid)
{
    return filefaceid & kRemoteFontMask;
}

class FontConfigTypeface : public SkTypeface {
public:
    FontConfigTypeface(Style style, uint32_t id)
        : SkTypeface(style, id) {}

    virtual ~FontConfigTypeface() {
        const uint32_t id = uniqueID();
        if (IsRemoteFont(UniqueIdToFileFaceId(id))) {
            SkAutoMutexAcquire ac(global_remote_font_map_lock);
            AllocateGlobalRemoteFontsMapOnce();
            std::map<uint32_t, std::pair<uint8_t*, size_t> >::iterator iter
                = global_remote_fonts->find(id);
            if (iter != global_remote_fonts->end()) {
                sk_free(iter->second.first);  // remove the font on memory.
                global_remote_fonts->erase(iter);
            }
        }
    }
};

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       SkTypeface::Style style) {
    SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
    if (NULL == fci.get()) {
        return NULL;
    }

    SkString familyNameStr;

    if (familyFace) {
        // Given the fileid we can ask fontconfig for the familyname of the
        // font.
        const unsigned filefaceid = UniqueIdToFileFaceId(familyFace->uniqueID());
        if (!fci->getFamilyName(filefaceid, &familyNameStr)) {
            return NULL;
        }
        familyName = familyNameStr.c_str();
    }

    unsigned filefaceid;
    if (!fci->match(familyName, style, &filefaceid, &style)) {
        return NULL;
    }

    const unsigned id = FileFaceIdAndStyleToUniqueId(filefaceid, style);
    return SkNEW_ARGS(FontConfigTypeface, (style, id));
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    if (!stream) {
        return NULL;
    }
    const size_t length = stream->read(0, 0);
    if (!length) {
        return NULL;
    }
    if (length >= 1024 * 1024 * 1024) {
        return NULL;  // don't accept too large fonts (>= 1GB) for safety.
    }

    uint8_t* font = (uint8_t*)sk_malloc_throw(length);
    if (stream->read(font, length) != length) {
        sk_free(font);
        return NULL;
    }

    SkTypeface::Style style = static_cast<SkTypeface::Style>(0);
    unsigned id = 0;
    {
        SkAutoMutexAcquire ac(global_remote_font_map_lock);
        AllocateGlobalRemoteFontsMapOnce();
        id = FileFaceIdAndStyleToUniqueId(
            global_next_remote_font_id | kRemoteFontMask, style);

        if (++global_next_remote_font_id >= kRemoteFontMask)
            global_next_remote_font_id = 0;

        if (!global_remote_fonts->insert(
                std::make_pair(id, std::make_pair(font, length))).second) {
            sk_free(font);
            return NULL;
        }
    }

    return SkNEW_ARGS(FontConfigTypeface, (style, id));
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    SkTypeface* face = NULL;
    SkFILEStream* stream = SkNEW_ARGS(SkFILEStream, (path));

    if (stream->isValid()) {
        face = CreateTypefaceFromStream(stream);
    }
    stream->unref();
    return face;
}

uint32_t SkFontHost::NextLogicalFont(SkFontID curr, SkFontID orig) {
    // We don't handle font fallback.
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

// Serialize, Deserialize need to be compatible across platforms, hence the use
// of SkFontDescriptor.

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
    SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
    SkFontDescriptor desc(face->style());
    SkString familyName;

    const unsigned filefaceid = UniqueIdToFileFaceId(face->uniqueID());
    if (fci.get() && fci->getFamilyName(filefaceid, &familyName)) {
        desc.setFamilyName(familyName.c_str());
    } else {
        desc.setFamilyName("sans-serif");
    }

//SkDebugf("Serialize:  <%s>\n", desc.getFamilyName());
    // would also like other names (see SkFontDescriptor.h)

    desc.serialize(stream);

    // by convention, we also write out the actual sfnt data, preceeded by
    // a packed-length. For now we skip that, so we just write the zero.
    stream->writePackedUInt(0);
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    SkFontDescriptor descriptor(stream);
    const char* familyName = descriptor.getFamilyName();
    const SkTypeface::Style style = descriptor.getStyle();

    const uint32_t customFontDataLength = stream->readPackedUInt();
    if (customFontDataLength > 0) {
#if 0 // need to support inline data...

        // generate a new stream to store the custom typeface
        SkMemoryStream* fontStream = new SkMemoryStream(customFontDataLength - 1);
        stream->read((void*)fontStream->getMemoryBase(), customFontDataLength - 1);

        SkTypeface* face = CreateTypefaceFromStream(fontStream);

        fontStream->unref();
        return face;
#else
        stream->skip(customFontDataLength);
        return NULL;
#endif
    } else {
//SkDebugf("Deserialize:<%s> %d\n", familyName, style);
        return SkFontHost::CreateTypeface(NULL, familyName, style);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkStream* SkFontHost::OpenStream(uint32_t id) {
    const unsigned filefaceid = UniqueIdToFileFaceId(id);

    if (IsRemoteFont(filefaceid)) {
      // remote font
      SkAutoMutexAcquire ac(global_remote_font_map_lock);
      AllocateGlobalRemoteFontsMapOnce();
      std::map<uint32_t, std::pair<uint8_t*, size_t> >::const_iterator iter
          = global_remote_fonts->find(id);
      if (iter == global_remote_fonts->end())
          return NULL;
      return SkNEW_ARGS(
          SkMemoryStream, (iter->second.first, iter->second.second));
    }

    // system font

    SkAutoTUnref<SkFontConfigInterface> fci(RefFCI());
    if (NULL == fci.get()) {
        return NULL;
    }
    return fci->openStream(filefaceid);
}

size_t SkFontHost::GetFileName(SkFontID fontID, char path[], size_t length,
                               int32_t* index) {
    const unsigned filefaceid = UniqueIdToFileFaceId(fontID);

    if (IsRemoteFont(filefaceid))
        return 0;

    if (index) {
        *index = filefaceid & 0xfu;
        // 1 is a bogus return value.
        // We had better change the signature of this function in Skia
        // to return bool to indicate success/failure and have another
        // out param for fileName length.
        if (!path)
          return 1;
    }

    if (path)
        SkASSERT(!"SkFontHost::GetFileName does not support the font path "
                  "retrieval.");

    return 0;
}
