/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKOBJECTPARSER_H_
#define SKOBJECTPARSER_H_

#include "SkCanvas.h"
#include "SkString.h"

/** \class SkObjectParser

    The ObjectParser is used to return string information about parameters
    in each draw command.
 */
class SkObjectParser {
public:

    /**
        Returns a string about a bitmap's bounds and colortype.
        @param bitmap  SkBitmap
    */
    static SkString* BitmapToString(const SkBitmap& bitmap);

    /**
        Returns a string about a image
        @param image   SkImage
    */
    static SkString* ImageToString(const SkImage* image);

    /**
        Returns a string representation of a boolean.
        @param doAA  boolean
     */
    static SkString* BoolToString(bool doAA);

    /**
        Returns a string representation of the text pointer passed in.
     */
    static SkString* CustomTextToString(const char* text);

    /**
        Returns a string representation of an integer with the text parameter
        at the front of the string.
        @param x  integer
        @param text
     */
    static SkString* IntToString(int x, const char* text);
    /**
        Returns a string representation of the SkIRects coordinates.
        @param rect  SkIRect
     */
    static SkString* IRectToString(const SkIRect& rect);

    /**
        Returns a string representation of an SkMatrix's contents
        @param matrix  SkMatrix
     */
    static SkString* MatrixToString(const SkMatrix& matrix);

    /**
        Returns a string representation of an SkPaint's color
        @param paint  SkPaint
     */
    static SkString* PaintToString(const SkPaint& paint);

    /**
        Returns a string representation of a SkPath's points.
        @param path  SkPath
     */
    static SkString* PathToString(const SkPath& path);

    /**
        Returns a string representation of the points in the point array.
        @param pts[]  Array of SkPoints
        @param count
     */
    static SkString* PointsToString(const SkPoint pts[], size_t count);

    /**
        Returns a string representation of the SkCanvas PointMode enum.
     */
    static SkString* PointModeToString(SkCanvas::PointMode mode);

    /**
        Returns a string representation of the SkRects coordinates.
        @param rect  SkRect
     */
    static SkString* RectToString(const SkRect& rect, const char* title = nullptr);

    /**
        Returns a string representation of an SkRRect.
        @param rrect  SkRRect
     */
    static SkString* RRectToString(const SkRRect& rrect, const char* title = nullptr);

    /**
        Returns a string representation of the SkRegion enum.
        @param op  SkRegion::op enum
     */
    static SkString* RegionOpToString(SkRegion::Op op);

    /**
        Returns a string representation of the SkRegion.
        @param region  SkRegion
     */
    static SkString* RegionToString(const SkRegion& region);

    /**
        Returns a string representation of the SkCanvas::SaveLayerFlags enum.
        @param flags  SkCanvas::SaveLayerFlags enum
     */
    static SkString* SaveLayerFlagsToString(uint32_t saveLayerFlags);

    /**
        Returns a string representation of an SkScalar with the text parameter
        at the front of the string.
        @param x  SkScalar
        @param text
     */
    static SkString* ScalarToString(SkScalar x, const char* text);

    /**
        Returns a string representation of the char pointer passed in.
        @param text  const void* that will be cast to a char*
     */
    static SkString* TextToString(const void* text, size_t byteLength,
                                  SkPaint::TextEncoding encoding);
};

#endif
