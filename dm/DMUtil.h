#ifndef DMUtil_DEFINED
#define DMUtil_DEFINED

#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "gm_expectations.h"

class SkBBHFactory;

// Small free functions used in more than one place in DM.

namespace DM {

// UnderJoin("a", "b") -> "a_b"
SkString UnderJoin(const char* a, const char* b);

// "foo_bar.skp" -> "foo-bar_skp"
SkString FileToTaskName(SkString);

// Draw gm to picture.  Passes recordFlags to SkPictureRecorder::beginRecording().
SkPicture* RecordPicture(skiagm::GM* gm,
                         uint32_t recordFlags = 0,
                         SkBBHFactory* factory = NULL);

// Allocate an empty bitmap with this size and depth.
void AllocatePixels(SkColorType, int w, int h, SkBitmap* bitmap);
// Allocate an empty bitmap the same size and depth as reference.
void AllocatePixels(const SkBitmap& reference, SkBitmap* bitmap);

// Draw picture to bitmap.
void DrawPicture(SkPicture* picture, SkBitmap* bitmap);

// What is the maximum component difference in these bitmaps?
unsigned MaxComponentDifference(const SkBitmap& a, const SkBitmap& b);

// Are these identical bitmaps?
bool BitmapsEqual(const SkBitmap& a, const SkBitmap& b);

}  // namespace DM

#endif  // DMUtil_DEFINED
