
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKOBJECTPARSER_H_
#define SKOBJECTPARSER_H_

#include <iostream>
#include <sstream>
#include "SkCanvas.h"
/** \class SkObjectParser

    The ObjectParser is used to return string information about parameters
    in each draw command.
    TODO(chudy): Change std::string to SkString
 */
class SkObjectParser {
public:

    /**
        Returns a string about a bitmaps bounds and config.
        @param bitmap  SkBitmap
    */
    static std::string BitmapToString(const SkBitmap& bitmap);

    /**
        Returns a string representation of a boolean.
        @param doAA  boolean
     */
    static std::string BoolToString(bool doAA);

    /**
        Returns a string representation of an integer with the text parameter
        at the front of the string.
        @param x  integer
        @param text
     */
    static std::string IntToString(int x, const char* text);
    /**
        Returns a string representation of the SkIRects coordinates.
        @param rect  SkIRect
     */
    static std::string IRectToString(const SkIRect& rect);

    /**
        Returns a string representation of an SkMatrix's contents
        @param matrix  SkMatrix
     */
    static std::string MatrixToString(const SkMatrix& matrix);

    /**
        Returns a string representation of an SkPaint's color
        @param paint  SkPaint
     */
    static std::string PaintToString(const SkPaint& paint);

    /**
        Returns a string representation of a SkPath's points.
        @param path  SkPath
     */
    static std::string PathToString(const SkPath& path);

    /**
        Returns a string representation of the points in the point array.
        @param pts[]  Array of SkPoints
        @param count
     */
    static std::string PointsToString(const SkPoint pts[], size_t count);

    /**
        Returns a string representation of the SkRects coordinates.
        @param rect  SkRect
     */
    static std::string RectToString(const SkRect& rect);

    /**
        Returns a string representation of the SkRegion enum.
        @param op  SkRegion::op enum
     */
    static std::string RegionOpToString(SkRegion::Op op);

    /**
        Returns a string representation of the SkRegion.
        @param region  SkRegion
     */
    static std::string RegionToString(const SkRegion& region);

    /**
        Returns a string representation of the SkCanvas::SaveFlags enum.
        @param flags  SkCanvas::SaveFlags enum
     */
    static std::string SaveFlagsToString(SkCanvas::SaveFlags flags);

    /**
        Returns a string representation of an SkScalar with the text parameter
        at the front of the string.
        @param x  SkScalar
        @param text
     */
    static std::string ScalarToString(SkScalar x, const char* text);

    /**
        Returns a string representation of the char pointer passed in.
        @param text  const void* that will be cast to a char*
     */
    static std::string TextToString(const void* text, size_t byteLength);
};

#endif
