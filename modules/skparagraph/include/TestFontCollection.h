// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/FontCollection.h"

namespace skia {
namespace textlayout {
class TestFontCollection : public FontCollection {
public:
    TestFontCollection(const std::string& resourceDir);
    ~TestFontCollection() = default;

private:
    std::string fResourceDir;
};
}  // namespace textlayout
}  // namespace skia