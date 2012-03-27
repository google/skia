

#include "ShapeOps.h"

extern bool comparePaths(const SkPath& one, const SkPath& two);
extern void comparePathsTiny(const SkPath& one, const SkPath& two);
extern bool drawAsciiPaths(const SkPath& one, const SkPath& two,
        bool drawPaths);
extern void showPath(const SkPath& path, const char* str = NULL);
extern bool testSimplify(const SkPath& path, bool fill, SkPath& out);
