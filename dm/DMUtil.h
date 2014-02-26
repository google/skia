#ifndef DMUtil_DEFINED
#define DMUtil_DEFINED

#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "gm_expectations.h"

// Small free functions used in more than one place in DM.

namespace DM {

// UnderJoin("a", "b") -> "a_b"
SkString UnderJoin(const char* a, const char* b);

// Draw gm to picture.  Passes recordFlags to SkPicture::beginRecording().
void RecordPicture(skiagm::GM* gm, SkPicture* picture, uint32_t recordFlags = 0);

// Prepare bitmap to have gm or bench draw into it with this config.
// TODO(mtklein): make SkBenchmark::getSize()/GM::getISize() const.
void SetupBitmap(const SkColorType, skiagm::GM* gm, SkBitmap* bitmap);
void SetupBitmap(const SkColorType, SkBenchmark* bench, SkBitmap* bitmap);

// Draw picture to bitmap.
void DrawPicture(SkPicture* picture, SkBitmap* bitmap);

// Are these identical bitmaps?
bool BitmapsEqual(const SkBitmap& a, const SkBitmap& b);

}  // namespace DM

#endif  // DMUtil_DEFINED
