#include "SkPath.h"

void contourBounds(const SkPath& path, SkTDArray<SkRect>& boundsArray);
void simplify(const SkPath& path, bool asFill, SkPath& simple);
void simplifyx(const SkPath& path, SkPath& simple);

// FIXME: remove this section once debugging is complete
extern const bool gRunTestsInOneThread;
#ifdef SK_DEBUG
extern int gDebugMaxWindSum;
extern int gDebugMaxWindValue;
#endif
