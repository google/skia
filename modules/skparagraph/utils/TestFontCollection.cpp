// Copyright 2019 Google LLC.
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace skia {
namespace textlayout {

sk_sp<TypefaceFontProvider> TestFontCollection::fFontProvider = nullptr;
std::set<std::string> TestFontCollection::fDirs;

TestFontCollection::TestFontCollection(const std::string& resourceDir)
  : fResourceDir(resourceDir)
  , fFontsFound(0) {
/*
    ParagraphImpl::setChecker([&](ParagraphImpl* impl, const char* method, bool result) {
        if (std::strcmp(method, "findParagraph") == 0) {
            if (result) {
                if (impl != nullptr) {
                    impl->printKeyValue("found paragraph:", true);
                }
            } else {
                if (impl != nullptr) {
                    impl->printKeyValue("not found paragraph:", false);
                }
            }
        } else if (std::strcmp(method, "addParagraph") == 0) {
            if (impl != nullptr) {
                impl->printKeyValue("added paragraph:", true);
            }
        } else {
            SkASSERT(false);
        }
    });
*/
    if (!fDirs.count(resourceDir)) {
        if (fFontProvider == nullptr) {
            fFontProvider = sk_make_sp<TypefaceFontProvider>();
        }

        SkOSFile::Iter iter(fResourceDir.c_str());
        SkString path;
        while (iter.next(&path)) {
            SkString file_path;
            file_path.printf("%s/%s", fResourceDir.c_str(), path.c_str());
            fFontProvider->registerTypeface(SkTypeface::MakeFromFile(file_path.c_str()));
        }
    }

    fFontsFound = fFontProvider->countFamilies();
    this->setTestFontManager(fFontProvider);
    this->disableFontFallback();
    fDirs.insert(resourceDir);
}
}  // namespace textlayout
}  // namespace skia