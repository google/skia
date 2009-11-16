#ifndef SkLineClipper_DEFINED
#define SkLineClipper_DEFINED

#include "SkRect.h"
#include "SkPoint.h"

class SkLineClipper {
public:
    enum {
        kMaxPoints = 4
    };

    /*  Clip the line pts[0]...pts[1] against clip, ignoring segments that
        lie completely above or below the clip. For portions to the left or
        right, turn those into vertical line segments that are aligned to the
        edge of the clip.
         
        Return the number of line segments that result, and store the end-points
        of those segments sequentially in lines as follows:
            1st segment: lines[0]..lines[1]
            2nd segment: lines[1]..lines[2]
            3rd segment: lines[2]..lines[3]
     */
    static int ClipLine(const SkPoint pts[2], const SkRect& clip,
                        SkPoint lines[kMaxPoints]);
};

#endif

