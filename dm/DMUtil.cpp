#include "DMUtil.h"

namespace DM {

SkString underJoin(const char* a, const char* b) {
    SkString s;
    s.appendf("%s_%s", a, b);
    return s;
}

SkString png(SkString s) {
    s.appendf(".png");
    return s;
}

bool meetsExpectations(const skiagm::Expectations& expectations, const SkBitmap bitmap) {
    if (expectations.ignoreFailure() || expectations.empty()) {
        return true;
    }
    const skiagm::GmResultDigest digest(bitmap);
    return expectations.match(digest);
}

}  // namespace DM
