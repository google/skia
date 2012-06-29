
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkObjectParser.h"

/* TODO(chudy): Replace all std::strings with char */

std::string SkObjectParser::BitmapToString(const SkBitmap& bitmap) {
    const char* mBitmap("SkBitmap: Data unavailable");
    return mBitmap;
}

std::string SkObjectParser::BoolToString(bool doAA) {
    if (doAA) {
        return "bool doAA: True";
    } else {
        return "bool doAA: False";
    }
}

std::string SkObjectParser::IntToString(int x, const char* text) {
    std::stringstream ss;
    ss << text << x;
    return ss.str();
}

std::string SkObjectParser::IRectToString(const SkIRect& rect) {
    std::stringstream ss;
    ss << "SkIRect: ";
    ss << "L: " << rect.left() << ",";
    ss << "T: " << rect.top() << ",";
    ss << "R: " << rect.right() << ",";
    ss << "B: " << rect.bottom();
    return ss.str();
}

std::string SkObjectParser::MatrixToString(const SkMatrix& matrix) {
    std::stringstream ss;
    /* NOTE(chudy): Cleaner looking than loops. */
    /* TODO(chudy): Decide whether to remove html part in order to really
     * seperate view / model. */
    ss << "SkMatrix:<br/>(";
    ss << matrix.get(0) << "), (";
    ss << matrix.get(1) << "), (";
    ss << matrix.get(2) << "), <br/>(";
    ss << matrix.get(3) << "), (";
    ss << matrix.get(4) << "), (";
    ss << matrix.get(5) << "), <br/>(";
    ss << matrix.get(6) << "), (";
    ss << matrix.get(7) << "), (";
    ss << matrix.get(8) <<  ")";
    return ss.str();
}

std::string SkObjectParser::PaintToString(const SkPaint& paint) {
    std::stringstream ss;
    SkColor color = paint.getColor();
    ss << "SkPaint: 0x" << std::hex << std::uppercase << color;
    return ss.str();
}

std::string SkObjectParser::PathToString(const SkPath& path) {
    std::string mPath;
    std::stringstream ss;
    mPath.append("SkPath: ");

    for (int i=0; i<path.countPoints(); i++) {
        ss <<  "(" << path.getPoint(i).fX << ", " << path.getPoint(i).fY << ") ";
        mPath.append(ss.str());
        ss.str("");
    }

    return mPath;
}

std::string SkObjectParser::PointsToString(const SkPoint pts[], size_t count) {
    std::stringstream ss;
    ss << "SkPoint pts[]: ";
    for (unsigned int i = 0; i < count; i++) {
        ss << "(" << pts[i].fX << "," << pts[i].fY << ") ";
    }
    return ss.str();
}

std::string SkObjectParser::RectToString(const SkRect& rect) {
    std::string mRect("SkRect: ");
    std::stringstream ss;

    mRect.append("(");
    ss << rect.left();
    mRect.append(ss.str());

    ss.str("");
    mRect.append(", ");
    ss << rect.top();
    mRect.append(ss.str());

    ss.str("");
    mRect.append(", ");
    ss << rect.right();
    mRect.append(ss.str());

    ss.str("");
    mRect.append(", ");
    ss << rect.bottom();
    mRect.append(ss.str());
    mRect.append(")");

    return mRect;
}

std::string SkObjectParser::RegionOpToString(SkRegion::Op op) {
    std::string mOp("SkRegion::Op: ");

    if (op == SkRegion::kDifference_Op) {
        mOp.append("kDifference_Op");
    } else if (op == SkRegion::kIntersect_Op) {
        mOp.append("kIntersect_Op");
    } else if (op == SkRegion::kUnion_Op) {
        mOp.append("kUnion_Op");
    } else if (op == SkRegion::kXOR_Op) {
        mOp.append("kXOR_Op");
    } else if (op == SkRegion::kReverseDifference_Op) {
        mOp.append("kReverseDifference_Op");
    } else if (op == SkRegion::kReplace_Op) {
        mOp.append("kReplace_Op");
    } else {
        mOp.append("Unknown Type");
    }

    return mOp;
}

std::string SkObjectParser::RegionToString(const SkRegion& region) {
    return "SkRegion: Data unavailable.";
}

std::string SkObjectParser::SaveFlagsToString(SkCanvas::SaveFlags flags) {
    std::string mFlags;
    mFlags.append("SkCanvas::SaveFlags: ");

    if(flags == SkCanvas::kMatrixClip_SaveFlag) {
        mFlags.append("kMatrixClip_SaveFlag");
    } else if (flags == SkCanvas::kClip_SaveFlag) {
        mFlags.append("kClip_SaveFlag");
    } else if (flags == SkCanvas::kHasAlphaLayer_SaveFlag) {
        mFlags.append("kHasAlphaLayer_SaveFlag");
    } else if (flags == SkCanvas::kFullColorLayer_SaveFlag) {
        mFlags.append("kFullColorLayer_SaveFlag");
    } else if (flags == SkCanvas::kClipToLayer_SaveFlag) {
        mFlags.append("kClipToLayer_SaveFlag");
    } else if (flags == SkCanvas::kMatrixClip_SaveFlag) {
        mFlags.append("kMatrixClip_SaveFlag");
    } else if (flags == SkCanvas::kARGB_NoClipLayer_SaveFlag) {
        mFlags.append("kARGB_NoClipLayer_SaveFlag");
    } else if (flags == SkCanvas::kARGB_ClipLayer_SaveFlag) {
        mFlags.append("kARGB_ClipLayer_SaveFlag");
    } else {
        mFlags.append("Data Unavailable");
    }

    return mFlags;
}

std::string SkObjectParser::ScalarToString(SkScalar x, const char* text) {
    std::string mScalar;
    mScalar.append(text);

    std::stringstream ss;
    ss << x;
    mScalar.append(ss.str());
    return mScalar;
}

std::string SkObjectParser::TextToString(const void* text, size_t byteLength) {
    char result[6+byteLength];
    strcpy(result,"Text: ");
    strcat(result, (char*)text);
    return result;
}
