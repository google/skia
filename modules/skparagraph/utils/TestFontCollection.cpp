// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"

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

    auto data = SkData::MakeFromFileName(file_path.c_str());
    if (!data) {
        return false;
    }
    if (familyName.empty()) {
        fFontProvider->registerTypeface(SkTypeface::MakeFromData(data));
    } else {
        fFontProvider->registerTypeface(SkTypeface::MakeFromData(data), SkString(familyName.c_str()));
    }

    return true;
}
}  // namespace textlayout
}  // namespace skia
