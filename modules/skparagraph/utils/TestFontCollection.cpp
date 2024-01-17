// Copyright 2019 Google LLC.
#include "modules/skparagraph/utils/TestFontCollection.h"

#include "include/core/SkStream.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "src/base/SkUTF.h"
#include "src/core/SkOSFile.h"
#include "tools/Resources.h"

#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
#include "src/ports/SkTypeface_FreeType.h"
#elif defined(SK_TYPEFACE_FACTORY_CORETEXT)
#include "src/ports/SkTypeface_mac_ct.h"
#elif defined(SK_TYPEFACE_FACTORY_DIRECTWRITE)
#include "src/ports/SkTypeface_win_dw.h"
#endif

#include <memory>
#include <utility>

namespace skia {
namespace textlayout {

TestFontCollection::TestFontCollection(const std::string& resourceDir, bool testOnly, bool loadFonts)
  : fResourceDir(resourceDir)
  , fFontsFound(0) {
    if (fDirs == resourceDir) {
      return;
    }

    fFontProvider = sk_make_sp<TypefaceFontProvider>();

    if (loadFonts) {
        SkOSFile::Iter iter(fResourceDir.c_str());
        SkString path;
        while (iter.next(&path)) {
            addFontFromFile(path.c_str());
        }
    }

    fFontsFound = fFontProvider->countFamilies();
    if (testOnly) {
        this->setTestFontManager(fFontProvider);
    } else {
        this->setAssetFontManager(fFontProvider);
    }
    this->disableFontFallback();
    fDirs = resourceDir;
}

bool TestFontCollection::addFontFromFile(const std::string& path, const std::string& familyName) {

    SkString file_path;
    file_path.printf("%s/%s", fResourceDir.c_str(), path.c_str());

    std::unique_ptr<SkStreamAsset> file = SkFILEStream::Make(file_path.c_str());
    if (!file) {
        return false;
    }
#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
    sk_sp<SkTypeface> face =
            SkTypeface_FreeType::MakeFromStream(std::move(file), SkFontArguments());
#elif defined(SK_TYPEFACE_FACTORY_CORETEXT)
    sk_sp<SkTypeface> face = SkTypeface_Mac::MakeFromStream(std::move(file), SkFontArguments());
#elif defined(SK_TYPEFACE_FACTORY_DIRECTWRITE)
    sk_sp<SkTypeface> face = DWriteFontTypeface::MakeFromStream(std::move(file), SkFontArguments());
#else
    sk_sp<SkTypeface> face = nullptr;
#endif
    if (familyName.empty()) {
        fFontProvider->registerTypeface(std::move(face));
    } else {
        fFontProvider->registerTypeface(std::move(face), SkString(familyName.c_str()));
    }

    return true;
}
}  // namespace textlayout
}  // namespace skia
