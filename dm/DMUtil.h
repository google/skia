#ifndef DMUtil_DEFINED
#define DMUtil_DEFINED

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkString.h"
#include "gm.h"

class SkBBHFactory;

// Small free functions used in more than one place in DM.

namespace DM {

// UnderJoin("a", "b") -> "a_b"
SkString UnderJoin(const char* a, const char* b);

// "foo_bar.skp" -> "foo-bar_skp"
SkString FileToTaskName(SkString);

// Draw gm to picture.
SkPicture* RecordPicture(skiagm::GM* gm, SkBBHFactory* factory = NULL);

// Allocate an empty bitmap with this size and depth.
void AllocatePixels(SkColorType, int w, int h, SkBitmap* bitmap);
// Allocate an empty bitmap the same size and depth as reference.
void AllocatePixels(const SkBitmap& reference, SkBitmap* bitmap);

// Draw picture to bitmap.
void DrawPicture(const SkPicture& picture, SkBitmap* bitmap);

// What is the maximum component difference in these bitmaps?
unsigned MaxComponentDifference(const SkBitmap& a, const SkBitmap& b);

// Are these identical bitmaps?
bool BitmapsEqual(const SkBitmap& a, const SkBitmap& b);

// Hook to modify canvas using global flag values (e.g. --matrix).
void CanvasPreflight(SkCanvas*);

}  // namespace DM

#endif  // DMUtil_DEFINED
