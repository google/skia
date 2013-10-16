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

bool meetsExpectations(const skiagm::Expectations& expectations,
                       const skiagm::GmResultDigest& digest) {
    return expectations.ignoreFailure()
        || expectations.empty()
        || expectations.match(digest);
}

}  // namespace DM
