
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkObjectParser.h"

/* TODO(chudy): Replace all std::strings with char */

SkString* SkObjectParser::BitmapToString(const SkBitmap& bitmap) {
    SkString* mBitmap = new SkString("SkBitmap: Data unavailable");
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
    mRect->appendScalar(rect.left());
    mRect->append(", T: ");
    mRect->appendScalar(rect.top());
    mRect->append(", R: ");
    mRect->appendScalar(rect.right());
    mRect->append(", B: ");
    mRect->appendScalar(rect.bottom());
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
    SkString* mPaint = new SkString("SkPaint: 0x");
    mPaint->appendHex(color);
    return mPaint;
}

SkString* SkObjectParser::PathToString(const SkPath& path) {
    SkString* mPath = new SkString("SkPath: ");
    for (int i = 0; i < path.countPoints(); i++) {
        mPath->append("(");
        mPath->appendScalar(path.getPoint(i).fX);
        mPath->append(", ");
        mPath->appendScalar(path.getPoint(i).fY);
        mPath->append(") ");
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

SkString* SkObjectParser::RectToString(const SkRect& rect) {
    SkString* mRect = new SkString("SkRect: ");
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
    char result[6+byteLength];
    strcpy(result,"Text: ");
    strcat(result, (char*)text);
    SkString* mText = new SkString(result);
    return mText;
}
