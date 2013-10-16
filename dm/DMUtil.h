#ifndef DMUtil_DEFINED
#define DMUtil_DEFINED

#include "SkString.h"
#include "gm_expectations.h"

// Small free functions used in more than one place in DM.

namespace DM {

// underJoin("a", "b") -> "a_b"
SkString underJoin(const char* a, const char* b);

// png("a") -> "a.png"
SkString png(SkString s);

// Roughly, expectations.match(digest), but only does it if we're not ignoring the result.
bool meetsExpectations(const skiagm::Expectations& expectations,
                       const skiagm::GmResultDigest& digest);

}  // namespace DM

#endif  // DMUtil_DEFINED
