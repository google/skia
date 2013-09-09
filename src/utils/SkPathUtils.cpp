/*
 *  CAUTION: EXPERIMENTAL CODE
 *
 *  This code is not to be used and will not be supported
 *  if it fails on you. DO NOT USE!
 *
 */

#include "SkPathUtils.h"

#include "SkPath.h"
#include "SkPathOps.h" // this can't be found, how do I link it?
#include "SkRegion.h"

typedef void (*line2path)(SkPath*, const char*, int, int);
#define SQRT_2 1.41421356237f
#define ON  0xFF000000 // black pixel
#define OFF 0x00000000 // transparent pixel

// assumes stride is in bytes
/*
static void FillRandomBits( int chars, char* bits ){
    SkTime time;
    SkRandom rand = SkRandom( time.GetMSecs() );

    for (int i = 0; i < chars; ++i){
        bits[i] = rand.nextU();
    }
}OA
*/

static int GetBit( const char* buffer, int x ) {
    int byte = x >> 3;
    int bit = x & 7;

    return buffer[byte] & (128 >> bit);
}

/*
static void Line2path_pixel(SkPath* path, const char* line,
                            int lineIdx, int width) {
    for (int i = 0; i < width; ++i) {
        // simply makes every ON pixel into a rect path
        if (GetBit(line,i)) {
            path->addRect(SkRect::MakeXYWH(i, lineIdx, 1, 1),
                          SkPath::kCW_Direction);
        }
    }
}

static void Line2path_pixelCircle(SkPath* path, const char* line,
                                  int lineIdx, int width) {
    for (int i = 0; i < width; ++i) {
        // simply makes every ON pixel into a circle path
        if (GetBit(line,i)) {
            path->addCircle(i + SK_ScalarHalf,
                            lineIdx + SK_ScalarHalf,
                            SkFloatToScalar(SQRT_2 / 2.0f));
        }
    }
}
*/

static void Line2path_span(SkPath* path, const char* line,
                           int lineIdx, int width) {
    bool inRun = 0;
    int start = 1;

    for (int i = 0; i < width; ++i) {
        int curPixel = GetBit(line,i);

        if ( (curPixel!=0) != inRun ) { // if transition
            if (curPixel) { // if transition on
                inRun = 1;
                start = i; // mark beginning of span
            }else { // if transition off add the span as a path
                inRun = 0;
                path->addRect(SkRect::MakeXYWH(SkIntToScalar(start), SkIntToScalar(lineIdx),
                                               SkIntToScalar(i-start), SK_Scalar1),
                              SkPath::kCW_Direction);
            }
        }
    }

    if (inRun==1) { // close any open spans
        int end = 0;
        if ( GetBit(line,width-1) ) ++end;
        path->addRect(SkRect::MakeXYWH(SkIntToScalar(start), SkIntToScalar(lineIdx),
                                       SkIntToScalar(width - 1 + end - start), SK_Scalar1),
                      SkPath::kCW_Direction);
    } else if ( GetBit(line, width - 1) ) { // if last pixel on add
        path->addRect(SkRect::MakeXYWH(width - SK_Scalar1, SkIntToScalar(lineIdx),
                                       SK_Scalar1, SK_Scalar1),
                      SkPath::kCW_Direction);
    }
}

void SkPathUtils::BitsToPath_Path(SkPath* path,
                        const char* bitmap,
                        int w, int h, int stride) {
    // loop for every line in bitmap
    for (int i = 0; i < h; ++i) {
        // fn ptr handles each line separately
        //l2p_fn(path, &bitmap[i*stride], i, w);
        Line2path_span(path, &bitmap[i*stride], i, w);
    }
    Simplify(*path, path); // simplify resulting path.
}

void SkPathUtils::BitsToPath_Region(SkPath* path,
                               const char* bitmap,
                               int w, int h, int stride) {
    SkRegion region;

    // loop for each line
    for (int y = 0; y < h; ++y){
        bool inRun = 0;
        int start = 1;
        const char* line = &bitmap[y * stride];

        // loop for each pixel
        for (int i = 0; i < w; ++i) {
            int curPixel = GetBit(line,i);

            if ( (curPixel!=0) != inRun ) { // if transition
                if (curPixel) { // if transition on
                    inRun = 1;
                    start = i; // mark beginning of span
                }else { // if transition off add the span as a path
                    inRun = 0;
                    //add here
                    region.op(SkIRect::MakeXYWH(start, y, i-start, 1),
                              SkRegion::kUnion_Op );
                }
            }
        }
        if (inRun==1) { // close any open spans
            int end = 0;
            if ( GetBit(line,w-1) ) ++end;
            // add the thing here
            region.op(SkIRect::MakeXYWH(start, y, w-1-start+end, 1),
                      SkRegion::kUnion_Op );

        } else if ( GetBit(line,w-1) ) { // if last pixel on add rect
            // add the thing here
            region.op(SkIRect::MakeXYWH(w-1, y, 1, 1),
                      SkRegion::kUnion_Op );
        }
    }
    // convert region to path
    region.getBoundaryPath(path);
}
