/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFShader_DEFINED
#define SkPDFShader_DEFINED

#include "SkBitmapKey.h"
#include "SkPDFTypes.h"
#include "SkShader.h"

class SkPDFCanon;
class SkPDFDocument;
class SkMatrix;
struct SkIRect;

/** \class SkPDFShader

    In PDF parlance, this is a pattern, used in place of a color when the
    pattern color space is selected.
*/

class SkPDFShader {
public:
    /** Get the PDF shader for the passed SkShader. If the SkShader is
     *  invalid in some way, returns nullptr. The reference count of
     *  the object is incremented and it is the caller's responsibility to
     *  unreference it when done.  This is needed to accommodate the weak
     *  reference pattern used when the returned object is new and has no
     *  other references.
     *  @param shader      The SkShader to emulate.
     *  @param matrix      The current transform. (PDF shaders are absolutely
     *                     positioned, relative to where the page is drawn.)
     *  @param surfceBBox  The bounding box of the drawing surface (with matrix
     *                     already applied).
     *  @param rasterScale Additional scale to be applied for early
     *                     rasterization.
     */
    static sk_sp<SkPDFObject> GetPDFShader(SkPDFDocument* doc,
                                           SkScalar dpi,
                                           SkShader* shader,
                                           const SkMatrix& matrix,
                                           const SkIRect& surfaceBBox,
                                           SkScalar rasterScale);

    static sk_sp<SkPDFArray> MakeRangeObject();

    class State {
    public:
        SkShader::GradientType fType;
        SkShader::GradientInfo fInfo;
        std::unique_ptr<SkColor[]> fColors;
        std::unique_ptr<SkScalar[]> fStops;
        SkMatrix fCanvasTransform;
        SkMatrix fShaderTransform;
        SkIRect fBBox;

        SkBitmapKey fBitmapKey;
        SkShader::TileMode fImageTileModes[2];

        State(SkShader* shader, const SkMatrix& canvasTransform,
              const SkIRect& bbox, SkScalar rasterScale,
              SkBitmap* dstImage);

        bool operator==(const State& b) const;

        State MakeAlphaToLuminosityState() const;
        State MakeOpaqueState() const;

        bool GradientHasAlpha() const;

        State(State&&) = default;
        State& operator=(State&&) = default;

    private:
        State(const State& other);
        State& operator=(const State& rhs);
        void allocateGradientInfoStorage();
    };
};

#endif
