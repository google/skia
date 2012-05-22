#include "SkPath.h"

void contourBounds(const SkPath& path, SkTDArray<SkRect>& boundsArray);
void simplify(const SkPath& path, bool asFill, SkPath& simple);
void simplifyx(const SkPath& path, bool asFill, SkPath& simple);

extern const bool gRunTestsInOneThread; // FIXME: remove once debugging is complete