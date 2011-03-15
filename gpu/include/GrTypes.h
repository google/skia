/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrTypes_DEFINED
#define GrTypes_DEFINED

#include "GrConfig.h"

#include <memory.h>
#include <string.h>

/**
 *  Macro to round n up to the next multiple of 4, or return it unchanged if
 *  n is already a multiple of 4
 */
#define GrALIGN4(n)     (((n) + 3) >> 2 << 2)
#define GrIsALIGN4(n)   (((n) & 3) == 0)

template <typename T> const T& GrMin(const T& a, const T& b) {
	return (a < b) ? a : b;
}

template <typename T> const T& GrMax(const T& a, const T& b) {
	return (b < a) ? a : b;
}

// compile time versions of min/max
#define GR_CT_MAX(a, b) (((b) < (a)) ? (a) : (b))
#define GR_CT_MIN(a, b) (((b) < (a)) ? (b) : (a))

/**
 *  divide, rounding up
 */
static inline uint32_t GrUIDivRoundUp(uint32_t x, uint32_t y) {
    return (x + (y-1)) / y;
}
static inline size_t GrSizeDivRoundUp(size_t x, uint32_t y) {
    return (x + (y-1)) / y;
}

/**
 *  align up
 */
static inline uint32_t GrUIAlignUp(uint32_t x, uint32_t alignment) {
    return GrUIDivRoundUp(x, alignment) * alignment;
}
static inline uint32_t GrSizeAlignUp(size_t x, uint32_t alignment) {
    return GrSizeDivRoundUp(x, alignment) * alignment;
}

/**
 * amount of pad needed to align up
 */
static inline uint32_t GrUIAlignUpPad(uint32_t x, uint32_t alignment) {
    return (alignment - x % alignment) % alignment;
}
static inline size_t GrSizeAlignUpPad(size_t x, uint32_t alignment) {
    return (alignment - x % alignment) % alignment;
}

/**
 *  align down
 */
static inline uint32_t GrUIAlignDown(uint32_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}
static inline uint32_t GrSizeAlignDown(size_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}

/**
 *  Count elements in an array
 */
#define GR_ARRAY_COUNT(array)  (sizeof(array) / sizeof(array[0]))

//!< allocate a block of memory, will never return NULL
extern void* GrMalloc(size_t bytes);

//!< free block allocated by GrMalloc. ptr may be NULL
extern void GrFree(void* ptr);

static inline void Gr_bzero(void* dst, size_t size) {
    memset(dst, 0, size);
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return the number of leading zeros in n
 */
extern int Gr_clz(uint32_t n);

/**
 *  Return true if n is a power of 2
 */
static inline bool GrIsPow2(unsigned n) {
    return n && 0 == (n & (n - 1));
}

/**
 *  Return the next power of 2 >= n.
 */
static inline uint32_t GrNextPow2(uint32_t n) {
    return n ? (1 << (32 - Gr_clz(n - 1))) : 1;
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  16.16 fixed point type
 */
typedef int32_t GrFixed;

#if GR_DEBUG

static inline int16_t GrToS16(intptr_t x) {
    GrAssert((int16_t)x == x);
    return (int16_t)x;
}

#else

#define GrToS16(x)  x

#endif

///////////////////////////////////////////////////////////////////////////////

/**
 *  Use to cast a ptr to a different type, and maintain strict-aliasing
 */
template <typename Dst, typename Src> Dst GrTCast(Src src) {
    union {
        Src src;
        Dst dst;
    } data;
    data.src = src;
    return data.dst;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Type used to describe format of vertices in arrays
 * Values are defined in GrDrawTarget
 */
typedef uint16_t GrVertexLayout;

/**
* Geometric primitives used for drawing.
*/
enum GrPrimitiveType {
    kTriangles_PrimitiveType,
    kTriangleStrip_PrimitiveType,
    kTriangleFan_PrimitiveType,
    kPoints_PrimitiveType,
    kLines_PrimitiveType,
    kLineStrip_PrimitiveType
};

/**
 * Coeffecients for alpha-blending.
 */
enum GrBlendCoeff {
    kZero_BlendCoeff,    //<! 0
    kOne_BlendCoeff,     //<! 1
    kSC_BlendCoeff,      //<! src color
    kISC_BlendCoeff,     //<! one minus src color
    kDC_BlendCoeff,      //<! dst color
    kIDC_BlendCoeff,     //<! one minus dst color
    kSA_BlendCoeff,      //<! src alpha
    kISA_BlendCoeff,     //<! one minus src alpha
    kDA_BlendCoeff,      //<! dst alpha
    kIDA_BlendCoeff,     //<! one minus dst alpha
    kConstC_BlendCoeff,  //<! constant color
    kIConstC_BlendCoeff, //<! one minus constant color
    kConstA_BlendCoeff,  //<! constant color alpha
    kIConstA_BlendCoeff, //<! one minus constant color alpha

    kBlendCoeffCount
};

/**
 *  Formats for masks, used by the font cache.
 *  Important that these are 0-based.
 */
enum GrMaskFormat {
    kA8_GrMaskFormat,   //!< 1-byte per pixel
    kA565_GrMaskFormat  //!< 2-bytes per pixel
};
#define kCount_GrMaskFormats    2

/**
 *  Return the number of bytes-per-pixel for the specified mask format.
 */
static inline int GrMaskFormatBytesPerPixel(GrMaskFormat format) {
    GrAssert((unsigned)format <= 1);
    return (int)format + 1;
}

/**
 * Set Operations used to construct clips.
 */
enum GrSetOp {
    kReplace_SetOp,
    kIntersect_SetOp,
    kUnion_SetOp,
    kXor_SetOp,
    kDifference_SetOp,
    kReverseDifference_SetOp,
};

/**
 * Clips are composed from these objects.
 */
enum GrClipType {
    kRect_ClipType,
    kPath_ClipType
};

/**
 * Commands used to describe a path. Each command
 * is accompanied by some number of points.
 */
enum GrPathCmd {
    kMove_PathCmd,      //!< Starts a new subpath at
                        //   at the returned point
                        // 1 point
    kLine_PathCmd,      //!< Adds a line segment
                        // 2 points
    kQuadratic_PathCmd, //!< Adds a quadratic segment
                        // 3 points
    kCubic_PathCmd,     //!< Adds a cubic segment
                        // 4 points
    kClose_PathCmd,     //!< Closes the current subpath
                        //   by connecting a line to the
                        //   starting point.
                        // 0 points
    kEnd_PathCmd        //!< Indicates the end of the last subpath
                        //   when iterating
                        // 0 points.
};

/**
 * Gets the number of points associated with a path command.
 */
static int inline NumPathCmdPoints(GrPathCmd cmd) {
    static const int gNumPoints[] = {
        1, 2, 3, 4, 0, 0
    };
    return gNumPoints[cmd];
}

/**
 * Path filling rules
 */
enum GrPathFill {
    kWinding_PathFill,
    kEvenOdd_PathFill,
    kInverseWinding_PathFill,
    kInverseEvenOdd_PathFill,
    kHairLine_PathFill,

    kPathFillCount
};

static inline GrPathFill NonInvertedFill(GrPathFill fill) {
    static const GrPathFill gNonInvertedFills[] = {
        kWinding_PathFill, // kWinding_PathFill
        kEvenOdd_PathFill, // kEvenOdd_PathFill
        kWinding_PathFill, // kInverseWinding_PathFill
        kEvenOdd_PathFill, // kInverseEvenOdd_PathFill
        kHairLine_PathFill,// kHairLine_PathFill
    };
    GR_STATIC_ASSERT(0 == kWinding_PathFill);
    GR_STATIC_ASSERT(1 == kEvenOdd_PathFill);
    GR_STATIC_ASSERT(2 == kInverseWinding_PathFill);
    GR_STATIC_ASSERT(3 == kInverseEvenOdd_PathFill);
    GR_STATIC_ASSERT(4 == kHairLine_PathFill);
    GR_STATIC_ASSERT(5 == kPathFillCount);
    return gNonInvertedFills[fill];
}

static inline bool IsFillInverted(GrPathFill fill) {
    static const bool gIsFillInverted[] = {
        false, // kWinding_PathFill
        false, // kEvenOdd_PathFill
        true,  // kInverseWinding_PathFill
        true,  // kInverseEvenOdd_PathFill
        false, // kHairLine_PathFill
    };
    GR_STATIC_ASSERT(0 == kWinding_PathFill);
    GR_STATIC_ASSERT(1 == kEvenOdd_PathFill);
    GR_STATIC_ASSERT(2 == kInverseWinding_PathFill);
    GR_STATIC_ASSERT(3 == kInverseEvenOdd_PathFill);
    GR_STATIC_ASSERT(4 == kHairLine_PathFill);
    GR_STATIC_ASSERT(5 == kPathFillCount);
    return gIsFillInverted[fill];
}

/**
 * Hints provided about a path's convexity (or lack thereof).
 */
enum GrConvexHint {
    kNone_ConvexHint,                         //<! No hint about convexity
                                              //   of the path
    kConvex_ConvexHint,                       //<! Path is one convex piece
    kNonOverlappingConvexPieces_ConvexHint,   //<! Multiple convex pieces,
                                              //   pieces are known to be
                                              //   disjoint
    kSameWindingConvexPieces_ConvexHint,      //<! Multiple convex pieces,
                                              //   may or may not intersect,
                                              //   either all wind cw or all
                                              //   wind ccw.
    kConcave_ConvexHint                       //<! Path is known to be
                                              //   concave
};

///////////////////////////////////////////////////////////////////////////////

// this is included only to make it easy to use this debugging facility
#include "GrInstanceCounter.h"

#endif
