#include "SkLineClipper.h"

// return X coordinate of intersection with horizontal line at Y
static SkScalar sect_with_horizontal(const SkPoint src[2], SkScalar Y) {
    SkScalar dy = src[1].fY - src[0].fY;
    if (SkScalarNearlyZero(dy)) {
        return SkScalarAve(src[0].fX, src[1].fX);
    } else {
        return src[0].fX + SkScalarMulDiv(Y - src[0].fY, src[1].fX - src[0].fX,
                                          dy);
    }
}

// return Y coordinate of intersection with vertical line at X
static SkScalar sect_with_vertical(const SkPoint src[2], SkScalar X) {
    SkScalar dx = src[1].fX - src[0].fX;
    if (SkScalarNearlyZero(dx)) {
        return SkScalarAve(src[0].fY, src[1].fY);
    } else {
        return src[0].fY + SkScalarMulDiv(X - src[0].fX, src[1].fY - src[0].fY,
                                          dx);
    }
}

int SkLineClipper::ClipLine(const SkPoint pts[], const SkRect& clip,
                            SkPoint lines[]) {
    int index0, index1;

    if (pts[0].fY < pts[1].fY) {
        index0 = 0;
        index1 = 1;
    } else {
        index0 = 1;
        index1 = 0;
    }

    // Check if we're completely clipped out in Y (above or below

    if (pts[index1].fY <= clip.fTop) {  // we're above the clip
        return 0;
    }
    if (pts[index0].fY >= clip.fBottom) {  // we're below the clip
        return 0;
    }
    
    // Chop in Y to produce a single segment, stored in tmp[0..1]

    SkPoint tmp[2];
    memcpy(tmp, pts, sizeof(tmp));

    // now compute intersections
    if (pts[index0].fY < clip.fTop) {
        tmp[index0].set(sect_with_horizontal(pts, clip.fTop), clip.fTop);
    }
    if (tmp[index1].fY > clip.fBottom) {
        tmp[index1].set(sect_with_horizontal(pts, clip.fBottom), clip.fBottom);
    }

    // Chop it into 1..3 segments that are wholly within the clip in X.

    // temp storage for up to 3 segments
    SkPoint resultStorage[kMaxPoints];
    SkPoint* result;    // points to our results, either tmp or resultStorage
    int lineCount = 1;

    if (pts[0].fX < pts[1].fX) {
        index0 = 0;
        index1 = 1;
    } else {
        index0 = 1;
        index1 = 0;
    }
    
    if (tmp[index1].fX <= clip.fLeft) {  // wholly to the left
        tmp[0].fX = tmp[1].fX = clip.fLeft;
        result = tmp;
    } else if (tmp[index0].fX >= clip.fRight) {    // wholly to the right
        tmp[0].fX = tmp[1].fX = clip.fRight;
        result = tmp;
    } else {
        result = resultStorage;
        SkPoint* r = result;
        
        if (tmp[index0].fX < clip.fLeft) {
            r->set(clip.fLeft, tmp[index0].fY);
            r += 1;
            r->set(clip.fLeft, sect_with_vertical(pts, clip.fLeft));
        } else {
            *r = tmp[index0];
        }
        r += 1;

        if (tmp[index1].fX > clip.fRight) {
            r->set(clip.fRight, sect_with_vertical(pts, clip.fRight));
            r += 1;
            r->set(clip.fRight, tmp[index1].fY);
        } else {
            *r = tmp[index1];
        }

        lineCount = r - result;
    }

    // Now copy the results into the caller's lines[] parameter
    if (0 == index1) {
        // copy the pts in reverse order to maintain winding order
        for (int i = 0; i <= lineCount; i++) {
            lines[lineCount - i] = result[i];
        }
    } else {
        memcpy(lines, result, (lineCount + 1) * sizeof(SkPoint));
    }
    return lineCount;
}

