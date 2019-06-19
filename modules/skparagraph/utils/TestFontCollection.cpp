// Copyright 2019 Google LLC.
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"

namespace skia {
namespace textlayout {
TestFontCollection::TestFontCollection(const std::string& resourceDir)
  : fResourceDir(resourceDir)
  , fFontsFound(0) {
    auto fontProvider = sk_make_sp<TypefaceFontProvider>();

    SkOSFile::Iter iter(fResourceDir.c_str());
    SkString path;
    while (iter.next(&path)) {
        SkString file_path;
        file_path.printf("%s/%s", fResourceDir.c_str(), path.c_str());
        fontProvider->registerTypeface(SkTypeface::MakeFromFile(file_path.c_str()));
    }
    fFontsFound = fontProvider->countFamilies();
    this->setTestFontManager(std::move(fontProvider));
    this->disableFontFallback();
}
}  // namespace textlayout
}  // namespace skia