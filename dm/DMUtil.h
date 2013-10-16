#ifndef DMUtil_DEFINED
#define DMUtil_DEFINED

#include "SkBitmap.h"
#include "SkString.h"
#include "gm_expectations.h"

// Small free functions used in more than one place in DM.

namespace DM {

// underJoin("a", "b") -> "a_b"
SkString underJoin(const char* a, const char* b);

// png("a") -> "a.png"
SkString png(SkString s);

// Roughly, expectations.match(GmResultDigest(bitmap)), but calculates the digest lazily.
bool meetsExpectations(const skiagm::Expectations& expectations, const SkBitmap bitmap);

}  // namespace DM

#endif  // DMUtil_DEFINED
