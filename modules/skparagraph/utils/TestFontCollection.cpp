// Copyright 2019 Google LLC.
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace skia {
namespace textlayout {

TestFontCollection::TestFontCollection(const std::string& resourceDir)
  : fResourceDir(resourceDir)
  , fFontsFound(0) {
    if (fDirs == resourceDir) {
      return;
    }

    fFontProvider = sk_make_sp<TypefaceFontProvider>();

    SkOSFile::Iter iter(fResourceDir.c_str());
    SkString path;
    while (iter.next(&path)) {
        SkString file_path;
        file_path.printf("%s/%s", fResourceDir.c_str(), path.c_str());
        // fonts from data are faster (skips file overhead), so we use them here for testing
        auto data = SkData::MakeFromFileName(file_path.c_str());
        if (data) {
            fFontProvider->registerTypeface(SkTypeface::MakeFromData(data));
        }
    }

    fFontsFound = fFontProvider->countFamilies();
    this->setTestFontManager(fFontProvider);
    this->disableFontFallback();
    fDirs = resourceDir;
}
}  // namespace textlayout
}  // namespace skia
