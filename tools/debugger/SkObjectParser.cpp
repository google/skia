/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkObjectParser.h"
#include "SkData.h"
#include "SkFontDescriptor.h"
#include "SkImage.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkStringUtils.h"
#include "SkTypeface.h"
#include "SkUtils.h"
#include "SkClipOpPriv.h"

/* TODO(chudy): Replace all std::strings with char */

SkString* SkObjectParser::BitmapToString(const SkBitmap& bitmap) {
    SkString* mBitmap = new SkString("SkBitmap: ");
    mBitmap->append("W: ");
    mBitmap->appendS32(bitmap.width());
    mBitmap->append(" H: ");
    mBitmap->appendS32(bitmap.height());

    const char* gColorTypeStrings[] = {
        "None", "A8", "565", "4444", "RGBA", "BGRA",
        "G8", "RGBAf16"
    };
    static_assert(kLastEnum_SkColorType + 1 == SK_ARRAY_COUNT(gColorTypeStrings),
                  "colortype names do not match colortype enum");

    mBitmap->append(" ColorType: ");
    mBitmap->append(gColorTypeStrings[bitmap.colorType()]);

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

SkString* SkObjectParser::ImageToString(const SkImage* image) {
    SkString* str = new SkString("SkImage: ");
    if (!image) {
        return str;
    }

    str->append("W: ");
    str->appendS32(image->width());
    str->append(" H: ");
    str->appendS32(image->height());

    if (image->isOpaque()) {
        str->append(" opaque");
    } else {
        str->append(" not-opaque");
    }

    str->append(" uniqueID: ");
    str->appendS32(image->uniqueID());

    return str;
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
    SkString* str = new SkString("SkMatrix: ");
#ifndef SK_IGNORE_TO_STRING
    matrix.toString(str);
#endif
    return str;
}

SkString* SkObjectParser::PaintToString(const SkPaint& paint) {
    SkString* str = new SkString;
#ifndef SK_IGNORE_TO_STRING
    paint.toString(str);
#endif
    return str;
}

SkString* SkObjectParser::PathToString(const SkPath& path) {
    SkString* mPath = new SkString;

    mPath->appendf("Path (%d) (", path.getGenerationID());

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

    if (path.isRect(nullptr)) {
        mPath->append("isRect, ");
    } else {
        mPath->append("isNotRect, ");
    }

    if (path.isOval(nullptr)) {
        mPath->append("isOval, ");
    } else {
        mPath->append("isNotOval, ");
    }

    SkRRect rrect;
    if (path.isRRect(&rrect)) {
        mPath->append("isRRect, ");
    } else {
        mPath->append("isNotRRect, ");
    }

    mPath->appendS32(path.countVerbs());
    mPath->append("V, ");
    mPath->appendS32(path.countPoints());
    mPath->append("P): ");

    static const char* gVerbStrings[] = {
        "Move", "Line", "Quad", "Conic", "Cubic", "Close", "Done"
    };
    static const int gPtsPerVerb[] = { 1, 1, 2, 2, 3, 0, 0 };
    static const int gPtOffsetPerVerb[] = { 0, 1, 1, 1, 1, 0, 0 };
    SkASSERT(SkPath::kDone_Verb == 6);

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
            mPath->append(")");
        }

        if (SkPath::kConic_Verb == verb) {
            mPath->append("(");
            mPath->appendScalar(iter.conicWeight());
            mPath->append(")");
        }

        mPath->append(" ");
    }

    SkString* boundStr = SkObjectParser::RectToString(path.getBounds(), "    Bound: ");

    if (boundStr) {
        mPath->append(*boundStr);
        delete boundStr;
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

    if (nullptr == title) {
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

    if (nullptr == title) {
        mRRect->append("SkRRect (");
        if (rrect.isEmpty()) {
            mRRect->append("empty");
        } else if (rrect.isRect()) {
            mRRect->append("rect");
        } else if (rrect.isOval()) {
            mRRect->append("oval");
        } else if (rrect.isSimple()) {
            mRRect->append("simple");
        } else if (rrect.isNinePatch()) {
            mRRect->append("nine-patch");
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

SkString* SkObjectParser::ClipOpToString(SkClipOp op) {
    SkString* mOp = new SkString("SkRegion::Op: ");
    if (op == kDifference_SkClipOp) {
        mOp->append("kDifference_Op");
    } else if (op == kIntersect_SkClipOp) {
        mOp->append("kIntersect_Op");
    } else if (op == kUnion_SkClipOp) {
        mOp->append("kUnion_Op");
    } else if (op == kXOR_SkClipOp) {
        mOp->append("kXOR_Op");
    } else if (op == kReverseDifference_SkClipOp) {
        mOp->append("kReverseDifference_Op");
    } else if (op == kReplace_SkClipOp) {
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

SkString* SkObjectParser::SaveLayerFlagsToString(SkCanvas::SaveLayerFlags saveLayerFlags) {
    SkString* mFlags = new SkString("SkCanvas::SaveFlags: ");
    if (saveLayerFlags & SkCanvas::kIsOpaque_SaveLayerFlag) {
        mFlags->append("kIsOpaque_SaveLayerFlag ");
    }
    if (saveLayerFlags & SkCanvas::kPreserveLCDText_SaveLayerFlag) {
        mFlags->append("kPreserveLCDText_SaveLayerFlag ");
    }
    return mFlags;
}

SkString* SkObjectParser::ScalarToString(SkScalar x, const char* text) {
    SkString* mScalar = new SkString(text);
    mScalar->append(" ");
    mScalar->appendScalar(x);
    return mScalar;
}

SkString* SkObjectParser::TextToString(const void* text, size_t byteLength,
                                       SkPaint::TextEncoding encoding) {

    SkString* decodedText = new SkString();
    switch (encoding) {
        case SkPaint::kUTF8_TextEncoding: {
            decodedText->append("UTF-8: ");
            decodedText->append((const char*)text, byteLength);
            break;
        }
        case SkPaint::kUTF16_TextEncoding: {
            decodedText->append("UTF-16: ");
            size_t sizeNeeded = SkUTF16_ToUTF8((uint16_t*)text,
                                                SkToS32(byteLength / 2),
                                                nullptr);
            SkAutoSTMalloc<0x100, char> utf8(sizeNeeded);
            SkUTF16_ToUTF8((uint16_t*)text, SkToS32(byteLength / 2), utf8);
            decodedText->append(utf8, sizeNeeded);
            break;
        }
        case SkPaint::kUTF32_TextEncoding: {
            decodedText->append("UTF-32: ");
            const SkUnichar* begin = (const SkUnichar*)text;
            const SkUnichar* end = (const SkUnichar*)((const char*)text + byteLength);
            for (const SkUnichar* unichar = begin; unichar < end; ++unichar) {
                decodedText->appendUnichar(*unichar);
            }
            break;
        }
        case SkPaint::kGlyphID_TextEncoding: {
            decodedText->append("GlyphID: ");
            const uint16_t* begin = (const uint16_t*)text;
            const uint16_t* end = (const uint16_t*)((const char*)text + byteLength);
            for (const uint16_t* glyph = begin; glyph < end; ++glyph) {
                decodedText->append("0x");
                decodedText->appendHex(*glyph);
                decodedText->append(" ");
            }
            break;
        }
        default:
            decodedText->append("Unknown text encoding.");
            break;
    }

    return decodedText;
}

SkString* SkObjectParser::LatticeToString(const SkCanvas::Lattice& lattice) {
    SkString* mLattice = new SkString;
    mLattice->append("Lattice: ");
    mLattice->append("(X: ");
    mLattice->appendS32(lattice.fXCount);
    mLattice->append(", Y:");
    mLattice->appendS32(lattice.fYCount);
    mLattice->append(", Bounds:");
    if (nullptr != lattice.fBounds) {
        mLattice->append(*IRectToString(*lattice.fBounds));
    } else {
        mLattice->append("null");
    }
    mLattice->append(")");
    return mLattice;
}
