/*
 *  CAUTION: EXPERIMENTAL CODE
 *
 *  This code is not to be used and will not be supported
 *  if it fails on you. DO NOT USE!
 *
 */

#ifndef SkPathUtils_DEFINED
#define SkPathUtils_DEFINED

#include "SkPath.h"

/*
 * The following methods return the boundary path given a 1-bit bitmap, specified
 * by width/height and stride. The bits are interpreted as 1 being "in" the path,
 * and 0 being "out". The bits are interpreted as MSB on the left, and LSB on the right.
 */

class SK_API SkPathUtils {
public:
    /**
       This variation iterates the binary data sequentially (as in scanline fashion)
       and will add each run of 1's to the path as a rectangular path. Upon parsing
       all binary data the path is simplified using the PathOps::Simplify() method.
    */
    static void BitsToPath_Path(SkPath* path, const char* bitmap,
                            int w, int h, int rowBytes);

    /**
       This variation utilizes the SkRegion class to generate paths, adding
       each run of 1's to the SkRegion as an SkIRect. Upon parsing the entirety
       of the binary the SkRegion is converted to a Path via getBoundaryPath().
    */
    static void BitsToPath_Region(SkPath* path, const char* bitmap,
                                   int w, int h, int rowBytes);

};

#endif
