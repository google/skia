

#include "SkPath.h"

extern void contourBounds(const SkPath& path, SkTDArray<SkRect>& boundsArray);
extern bool comparePaths(const SkPath& one, const SkPath& two);
extern void comparePathsTiny(const SkPath& one, const SkPath& two);
extern void drawAsciiPaths(const SkPath& one, const SkPath& two,
        bool drawPaths);
extern void simplify(const SkPath& path, bool asFill, SkPath& simple);
extern void showPath(const SkPath& path, const char* str = NULL);
extern bool testSimplify(const SkPath& path, bool fill, SkPath& out);
