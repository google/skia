// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"

namespace skia {
namespace textlayout {
class TestFontCollection : public FontCollection {
public:
    TestFontCollection(const std::string& resourceDir, bool testOnly = false, bool loadFonts = true);
    ~TestFontCollection() = default;

    size_t fontsFound() const { return fFontsFound; }
    bool addFontFromFile(const std::string& path, const std::string& familyName = "");

private:
    std::string fResourceDir;
    size_t fFontsFound;
    sk_sp<TypefaceFontProvider> fFontProvider;
    std::string fDirs;
};
}  // namespace textlayout
}  // namespace skia
