
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkObjectParser.h"
#include "SkRRect.h"
#include "SkTypeface.h"
#include "SkStream.h"
#include "SkData.h"
#include "SkFontDescriptor.h"

/* TODO(chudy): Replace all std::strings with char */

SkString* SkObjectParser::BitmapToString(const SkBitmap& bitmap) {
    SkString* mBitmap = new SkString("SkBitmap: ");
    mBitmap->append("W: ");
    mBitmap->appendS32(bitmap.width());
    mBitmap->append(" H: ");
    mBitmap->appendS32(bitmap.height());

    const char* gConfigStrings[] = {
        "None", "A1", "A8", "Index8", "RGB565", "ARGB4444", "ARGB8888", "RLE8"
    };
    SkASSERT(SkBitmap::kConfigCount == 8);

    mBitmap->append(" Config: ");
    mBitmap->append(gConfigStrings[bitmap.getConfig()]);

    if (bitmap.isOpaque()) {
        mBitmap->append(" opaque");
    } else {
        mBitmap->append(" not-opaque");
    }

    if (bitmap.isImmutable()) {
        mBitmap->append(" immutable");
    } else {
        mBitmap->append(" not-immutable");
    }

    if (bitmap.isVolatile()) {
        mBitmap->append(" volatile");
    } else {
        mBitmap->append(" not-volatile");
    }

    mBitmap->append(" genID: ");
    mBitmap->appendS32(bitmap.getGenerationID());

    return mBitmap;
}

SkString* SkObjectParser::BoolToString(bool doAA) {
    SkString* mBool = new SkString("Bool doAA: ");
    if (doAA) {
        mBool->append("True");
    } else {
        mBool->append("False");
    }
    return mBool;
}

SkString* SkObjectParser::CustomTextToString(const char* text) {
    SkString* mText = new SkString(text);
    return mText;
}

SkString* SkObjectParser::IntToString(int x, const char* text) {
    SkString* mInt = new SkString(text);
    mInt->append(" ");
    mInt->appendScalar(SkIntToScalar(x));
    return mInt;
}

SkString* SkObjectParser::IRectToString(const SkIRect& rect) {
    SkString* mRect = new SkString("SkIRect: ");
    mRect->append("L: ");
    mRect->appendS32(rect.left());
    mRect->append(", T: ");
    mRect->appendS32(rect.top());
    mRect->append(", R: ");
    mRect->appendS32(rect.right());
    mRect->append(", B: ");
    mRect->appendS32(rect.bottom());
    return mRect;
}

SkString* SkObjectParser::MatrixToString(const SkMatrix& matrix) {
    SkString* mMatrix = new SkString("SkMatrix: (");
    for (int i = 0; i < 8; i++) {
        mMatrix->appendScalar(matrix.get(i));
        mMatrix->append("), (");
    }
    mMatrix->appendScalar(matrix.get(8));
    mMatrix->append(")");
    return mMatrix;
}

SkString* SkObjectParser::PaintToString(const SkPaint& paint) {
    SkColor color = paint.getColor();
    SkString* mPaint = new SkString("<dl><dt>SkPaint:</dt><dd><dl><dt>Color:</dt><dd>0x");
    mPaint->appendHex(color);
    mPaint->append("</dd>");
    
    SkTypeface *typeface = paint.getTypeface();
    if (typeface) {
        SkDynamicMemoryWStream ostream;
        typeface->serialize(&ostream);
        SkData *data = SkAutoTUnref<SkData>(ostream.copyToData());
    
        SkMemoryStream stream(data);
        SkFontDescriptor descriptor(&stream);
    
        mPaint->append("<dt>Font Family Name:</dt><dd>");
        mPaint->append(descriptor.getFamilyName());
        mPaint->append("</dd><dt>Font Full Name:</dt><dd>");
        mPaint->append(descriptor.getFullName());
        mPaint->append("</dd><dt>Font PS Name:</dt><dd>");
        mPaint->append(descriptor.getPostscriptName());
        mPaint->append("</dd><dt>Font File Name:</dt><dd>");
        mPaint->append(descriptor.getFontFileName());
        mPaint->append("</dd></dl></dl>");
    }

    return mPaint;
}

SkString* SkObjectParser::PathToString(const SkPath& path) {
    SkString* mPath = new SkString("Path (");

    static const char* gFillStrings[] = {
        "Winding", "EvenOdd", "InverseWinding", "InverseEvenOdd"
    };

    mPath->append(gFillStrings[path.getFillType()]);
    mPath->append(", ");

    static const char* gConvexityStrings[] = {
        "Unknown", "Convex", "Concave"
    };
    SkASSERT(SkPath::kConcave_Convexity == 2);

    mPath->append(gConvexityStrings[path.getConvexity()]);
    mPath->append(", ");

    if (path.isRect(NULL)) {
        mPath->append("isRect, ");
    } else {
        mPath->append("isNotRect, ");
    }

    mPath->appendS32(path.countVerbs());
    mPath->append("V, ");
    mPath->appendS32(path.countPoints());
    mPath->append("P): ");

    static const char* gVerbStrings[] = {
        "Move", "Line", "Quad", "Cubic", "Close", "Done"
    };
    static const int gPtsPerVerb[] = { 1, 1, 2, 3, 0, 0 };
    static const int gPtOffsetPerVerb[] = { 0, 1, 1, 1, 0, 0 };
    SkASSERT(SkPath::kDone_Verb == 5);

    SkPath::Iter iter(const_cast<SkPath&>(path), false);
    SkPath::Verb verb;
    SkPoint points[4];

    for(verb = iter.next(points, false);
        verb != SkPath::kDone_Verb;
        verb = iter.next(points, false)) {

        mPath->append(gVerbStrings[verb]);
        mPath->append(" ");

        for (int i = 0; i < gPtsPerVerb[verb]; ++i) {
            mPath->append("(");
            mPath->appendScalar(points[gPtOffsetPerVerb[verb]+i].fX);
            mPath->append(", ");
            mPath->appendScalar(points[gPtOffsetPerVerb[verb]+i].fY);
            mPath->append(") ");
        }
    }

    SkString* boundStr = SkObjectParser::RectToString(path.getBounds(), "    Bound: ");

    if (NULL != boundStr) {
        mPath->append(*boundStr);
        SkDELETE(boundStr);
    }

    return mPath;
}

SkString* SkObjectParser::PointsToString(const SkPoint pts[], size_t count) {
    SkString* mPoints = new SkString("SkPoints pts[]: ");
    for (unsigned int i = 0; i < count; i++) {
        mPoints->append("(");
        mPoints->appendScalar(pts[i].fX);
        mPoints->append(",");
        mPoints->appendScalar(pts[i].fY);
        mPoints->append(")");
    }
    return mPoints;
}

SkString* SkObjectParser::PointModeToString(SkCanvas::PointMode mode) {
    SkString* mMode = new SkString("SkCanvas::PointMode: ");
    if (mode == SkCanvas::kPoints_PointMode) {
        mMode->append("kPoints_PointMode");
    } else if (mode == SkCanvas::kLines_PointMode) {
        mMode->append("kLines_Mode");
    } else if (mode == SkCanvas::kPolygon_PointMode) {
        mMode->append("kPolygon_PointMode");
    }
    return mMode;
}

SkString* SkObjectParser::RectToString(const SkRect& rect, const char* title) {

    SkString* mRect = new SkString;

    if (NULL == title) {
        mRect->append("SkRect: ");
    } else {
        mRect->append(title);
    }
    mRect->append("(");
    mRect->appendScalar(rect.left());
    mRect->append(", ");
    mRect->appendScalar(rect.top());
    mRect->append(", ");
    mRect->appendScalar(rect.right());
    mRect->append(", ");
    mRect->appendScalar(rect.bottom());
    mRect->append(")");
    return mRect;
}

SkString* SkObjectParser::RRectToString(const SkRRect& rrect, const char* title) {

    SkString* mRRect = new SkString;

    if (NULL == title) {
        mRRect->append("SkRRect (");
        if (rrect.isEmpty()) {
            mRRect->append("empty");
        } else if (rrect.isRect()) {
            mRRect->append("rect");
        } else if (rrect.isOval()) {
            mRRect->append("oval");
        } else if (rrect.isSimple()) {
            mRRect->append("simple");
        } else {
            SkASSERT(rrect.isComplex());
            mRRect->append("complex");
        }
        mRRect->append("): ");
    } else {
        mRRect->append(title);
    }
    mRRect->append("(");
    mRRect->appendScalar(rrect.rect().left());
    mRRect->append(", ");
    mRRect->appendScalar(rrect.rect().top());
    mRRect->append(", ");
    mRRect->appendScalar(rrect.rect().right());
    mRRect->append(", ");
    mRRect->appendScalar(rrect.rect().bottom());
    mRRect->append(") radii: (");
    for (int i = 0; i < 4; ++i) {
        const SkVector& radii = rrect.radii((SkRRect::Corner) i);
        mRRect->appendScalar(radii.fX);
        mRRect->append(", ");
        mRRect->appendScalar(radii.fY);
        if (i < 3) {
            mRRect->append(", ");
        }
    }
    mRRect->append(")");
    return mRRect;
}

SkString* SkObjectParser::RegionOpToString(SkRegion::Op op) {
    SkString* mOp = new SkString("SkRegion::Op: ");
    if (op == SkRegion::kDifference_Op) {
        mOp->append("kDifference_Op");
    } else if (op == SkRegion::kIntersect_Op) {
        mOp->append("kIntersect_Op");
    } else if (op == SkRegion::kUnion_Op) {
        mOp->append("kUnion_Op");
    } else if (op == SkRegion::kXOR_Op) {
        mOp->append("kXOR_Op");
    } else if (op == SkRegion::kReverseDifference_Op) {
        mOp->append("kReverseDifference_Op");
    } else if (op == SkRegion::kReplace_Op) {
        mOp->append("kReplace_Op");
    } else {
        mOp->append("Unknown Type");
    }
    return mOp;
}

SkString* SkObjectParser::RegionToString(const SkRegion& region) {
    SkString* mRegion = new SkString("SkRegion: Data unavailable.");
    return mRegion;
}

SkString* SkObjectParser::SaveFlagsToString(SkCanvas::SaveFlags flags) {
    SkString* mFlags = new SkString("SkCanvas::SaveFlags: ");
    if(flags == SkCanvas::kMatrixClip_SaveFlag) {
        mFlags->append("kMatrixClip_SaveFlag");
    } else if (flags == SkCanvas::kClip_SaveFlag) {
        mFlags->append("kClip_SaveFlag");
    } else if (flags == SkCanvas::kHasAlphaLayer_SaveFlag) {
        mFlags->append("kHasAlphaLayer_SaveFlag");
    } else if (flags == SkCanvas::kFullColorLayer_SaveFlag) {
        mFlags->append("kFullColorLayer_SaveFlag");
    } else if (flags == SkCanvas::kClipToLayer_SaveFlag) {
        mFlags->append("kClipToLayer_SaveFlag");
    } else if (flags == SkCanvas::kMatrixClip_SaveFlag) {
        mFlags->append("kMatrixClip_SaveFlag");
    } else if (flags == SkCanvas::kARGB_NoClipLayer_SaveFlag) {
        mFlags->append("kARGB_NoClipLayer_SaveFlag");
    } else if (flags == SkCanvas::kARGB_ClipLayer_SaveFlag) {
        mFlags->append("kARGB_ClipLayer_SaveFlag");
    } else {
        mFlags->append("Data Unavailable");
    }
    return mFlags;
}

SkString* SkObjectParser::ScalarToString(SkScalar x, const char* text) {
    SkString* mScalar = new SkString(text);
    mScalar->append(" ");
    mScalar->appendScalar(x);
    return mScalar;
}

SkString* SkObjectParser::TextToString(const void* text, size_t byteLength) {
    SkString* mText = new SkString(6+byteLength+1);
    mText->append("Text: ");
    mText->append((char*) text, byteLength);
    return mText;
}
