
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

static void add_flag_to_string(SkString* string, bool flag, const char* flagStr, bool* needSeparator) {
    if (flag) {
        if (*needSeparator) {
            string->append("|");
        }
        string->append(flagStr);
        *needSeparator = true;
    }
}

SkString* SkObjectParser::PaintToString(const SkPaint& paint) {
    SkString* mPaint = new SkString("<dl><dt>SkPaint:</dt><dd><dl>");

    SkTypeface* typeface = paint.getTypeface();
    if (NULL != typeface) {
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
        mPaint->append("</dd>");
    }

    mPaint->append("<dt>TextSize:</dt><dd>");
    mPaint->appendScalar(paint.getTextSize());
    mPaint->append("</dd>");

    mPaint->append("<dt>TextScaleX:</dt><dd>");
    mPaint->appendScalar(paint.getTextScaleX());
    mPaint->append("</dd>");

    mPaint->append("<dt>TextSkewX:</dt><dd>");
    mPaint->appendScalar(paint.getTextSkewX());
    mPaint->append("</dd>");

    SkPathEffect* pathEffect = paint.getPathEffect();
    if (NULL != pathEffect) {
        mPaint->append("<dt>PathEffect:</dt><dd>");
        mPaint->append("</dd>");
    }

    SkShader* shader = paint.getShader();
    if (NULL != shader) {
        mPaint->append("<dt>Shader:</dt><dd>");
        mPaint->append("</dd>");
    }

    SkXfermode* xfer = paint.getXfermode();
    if (NULL != xfer) {
        mPaint->append("<dt>Xfermode:</dt><dd>");
        mPaint->append("</dd>");
    }

    SkMaskFilter* maskFilter = paint.getMaskFilter();
    if (NULL != maskFilter) {
        mPaint->append("<dt>MaskFilter:</dt><dd>");
        mPaint->append("</dd>");
    }

    SkColorFilter* colorFilter = paint.getColorFilter();
    if (NULL != colorFilter) {
        mPaint->append("<dt>ColorFilter:</dt><dd>");
        mPaint->append("</dd>");
    }

    SkRasterizer* rasterizer = paint.getRasterizer();
    if (NULL != rasterizer) {
        mPaint->append("<dt>Rasterizer:</dt><dd>");
        mPaint->append("</dd>");
    }

    SkDrawLooper* looper = paint.getLooper();
    if (NULL != looper) {
        mPaint->append("<dt>DrawLooper:</dt><dd>");
        mPaint->append("</dd>");
    }

    SkImageFilter* imageFilter = paint.getImageFilter();
    if (NULL != imageFilter) {
        mPaint->append("<dt>ImageFilter:</dt><dd>");
        mPaint->append("</dd>");
    }

    SkAnnotation* annotation = paint.getAnnotation();
    if (NULL != annotation) {
        mPaint->append("<dt>Annotation:</dt><dd>");
        mPaint->append("</dd>");
    }

    mPaint->append("<dt>Color:</dt><dd>0x");
    SkColor color = paint.getColor();
    mPaint->appendHex(color);
    mPaint->append("</dd>");

    mPaint->append("<dt>Stroke Width:</dt><dd>");
    mPaint->appendScalar(paint.getStrokeWidth());
    mPaint->append("</dd>");

    mPaint->append("<dt>Stroke Miter:</dt><dd>");
    mPaint->appendScalar(paint.getStrokeMiter());
    mPaint->append("</dd>");

    mPaint->append("<dt>Flags:</dt><dd>(");
    if (paint.getFlags()) {
        bool needSeparator = false;
        add_flag_to_string(mPaint, paint.isAntiAlias(), "AntiAlias", &needSeparator);
        add_flag_to_string(mPaint, paint.isFilterBitmap(), "FilterBitmap", &needSeparator);
        add_flag_to_string(mPaint, paint.isDither(), "Dither", &needSeparator);
        add_flag_to_string(mPaint, paint.isUnderlineText(), "UnderlineText", &needSeparator);
        add_flag_to_string(mPaint, paint.isStrikeThruText(), "StrikeThruText", &needSeparator);
        add_flag_to_string(mPaint, paint.isFakeBoldText(), "FakeBoldText", &needSeparator);
        add_flag_to_string(mPaint, paint.isLinearText(), "LinearText", &needSeparator);
        add_flag_to_string(mPaint, paint.isSubpixelText(), "SubpixelText", &needSeparator);
        add_flag_to_string(mPaint, paint.isDevKernText(), "DevKernText", &needSeparator);
        add_flag_to_string(mPaint, paint.isLCDRenderText(), "LCDRenderText", &needSeparator);
        add_flag_to_string(mPaint, paint.isEmbeddedBitmapText(), 
                           "EmbeddedBitmapText", &needSeparator);
        add_flag_to_string(mPaint, paint.isAutohinted(), "Autohinted", &needSeparator);
        add_flag_to_string(mPaint, paint.isVerticalText(), "VerticalText", &needSeparator);
        add_flag_to_string(mPaint, SkToBool(paint.getFlags() & SkPaint::kGenA8FromLCD_Flag), 
                           "GenA8FromLCD", &needSeparator);
    } else {
        mPaint->append("None");
    }
    mPaint->append(")</dd>");

    mPaint->append("<dt>TextAlign:</dt><dd>");
    static const char* gTextAlignStrings[SkPaint::kAlignCount] = { "Left", "Center", "Right" };
    mPaint->append(gTextAlignStrings[paint.getTextAlign()]);
    mPaint->append("</dd>");

    mPaint->append("<dt>CapType:</dt><dd>");
    static const char* gStrokeCapStrings[SkPaint::kCapCount] = { "Butt", "Round", "Square" };
    mPaint->append(gStrokeCapStrings[paint.getStrokeCap()]);
    mPaint->append("</dd>");

    mPaint->append("<dt>JoinType:</dt><dd>");
    static const char* gJoinStrings[SkPaint::kJoinCount] = { "Miter", "Round", "Bevel" };
    mPaint->append(gJoinStrings[paint.getStrokeJoin()]);
    mPaint->append("</dd>");

    mPaint->append("<dt>Style:</dt><dd>");
    static const char* gStyleStrings[SkPaint::kStyleCount] = { "Fill", "Stroke", "StrokeAndFill" };
    mPaint->append(gStyleStrings[paint.getStyle()]);
    mPaint->append("</dd>");

    mPaint->append("<dt>TextEncoding:</dt><dd>");
    static const char* gTextEncodingStrings[] = { "UTF8", "UTF16", "UTF32", "GlyphID" };
    mPaint->append(gTextEncodingStrings[paint.getTextEncoding()]);
    mPaint->append("</dd>");

    mPaint->append("<dt>Hinting:</dt><dd>");
    static const char* gHintingStrings[] = { "None", "Slight", "Normal", "Full" };
    mPaint->append(gHintingStrings[paint.getHinting()]);
    mPaint->append("</dd>");

    mPaint->append("</dd></dl></dl>");
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
