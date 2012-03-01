

#include "SkPath.h"

extern void contourBounds(const SkPath& path, SkTDArray<SkRect>& boundsArray);
extern void comparePaths(const SkPath& one, const SkPath& two);
extern void comparePathsTiny(const SkPath& one, const SkPath& two);
extern void simplify(const SkPath& path, bool asFill, SkPath& simple);

