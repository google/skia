/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRendering_DEFINED
#define GrPathRendering_DEFINED

#include "SkPath.h"

class SkStrokeRec;
class GrPath;
class GrPathRange;
class GrGpu;

/**
 * Abstract class wrapping HW path rendering API.
 *
 * The subclasses of this class use the possible HW API to render paths (as opposed to path
 * rendering implemented in Skia on top of a "3d" HW API).
 * The subclasses hold the global state needed to render paths, including shadow of the global HW
 * API state. Similar to GrGpu.
 *
 * It is expected that the lifetimes of GrGpuXX and GrXXPathRendering are the same. The call context
 * interface (eg.  * the concrete instance of GrGpu subclass) should be provided to the instance
 * during construction.
 */
class GrPathRendering {
public:
    virtual ~GrPathRendering() { }

    enum PathTransformType {
        kNone_PathTransformType,        //!< []
        kTranslateX_PathTransformType,  //!< [kMTransX]
        kTranslateY_PathTransformType,  //!< [kMTransY]
        kTranslate_PathTransformType,   //!< [kMTransX, kMTransY]
        kAffine_PathTransformType,      //!< [kMScaleX, kMSkewX, kMTransX, kMSkewY, kMScaleY, kMTransY]

        kLast_PathTransformType = kAffine_PathTransformType
    };

    static inline int PathTransformSize(PathTransformType type) {
        switch (type) {
            case kNone_PathTransformType:
                return 0;
            case kTranslateX_PathTransformType:
            case kTranslateY_PathTransformType:
                return 1;
            case kTranslate_PathTransformType:
                return 2;
            case kAffine_PathTransformType:
                return 6;

            default:
                SkFAIL("Unknown path transform type");
                return 0;
        }
    }

    virtual GrPath* createPath(const SkPath&, const SkStrokeRec&) = 0;
    virtual GrPathRange* createPathRange(size_t size, const SkStrokeRec&) = 0;
    virtual void stencilPath(const GrPath*, SkPath::FillType) = 0;
    virtual void drawPath(const GrPath*, SkPath::FillType) = 0;
    virtual void drawPaths(const GrPathRange*, const uint32_t indices[], int count,
                           const float transforms[], PathTransformType, SkPath::FillType) = 0;
protected:
    GrPathRendering() { }

private:
    GrPathRendering& operator=(const GrPathRendering&);
};

#endif
