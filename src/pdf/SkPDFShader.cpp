
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFShader.h"

#include "SkData.h"
#include "SkPDFCanon.h"
#include "SkPDFDevice.h"
#include "SkPDFFormXObject.h"
#include "SkPDFGraphicState.h"
#include "SkPDFResourceDict.h"
#include "SkPDFUtils.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTypes.h"

static bool inverse_transform_bbox(const SkMatrix& matrix, SkRect* bbox) {
    SkMatrix inverse;
    if (!matrix.invert(&inverse)) {
        return false;
    }
    inverse.mapRect(bbox);
    return true;
}

static void unitToPointsMatrix(const SkPoint pts[2], SkMatrix* matrix) {
    SkVector    vec = pts[1] - pts[0];
    SkScalar    mag = vec.length();
    SkScalar    inv = mag ? SkScalarInvert(mag) : 0;

    vec.scale(inv);
    matrix->setSinCos(vec.fY, vec.fX);
    matrix->preScale(mag, mag);
    matrix->postTranslate(pts[0].fX, pts[0].fY);
}

/* Assumes t + startOffset is on the stack and does a linear interpolation on t
   between startOffset and endOffset from prevColor to curColor (for each color
   component), leaving the result in component order on the stack. It assumes
   there are always 3 components per color.
   @param range                  endOffset - startOffset
   @param curColor[components]   The current color components.
   @param prevColor[components]  The previous color components.
   @param result                 The result ps function.
 */
static void interpolateColorCode(SkScalar range, SkScalar* curColor,
                                 SkScalar* prevColor, SkString* result) {
    SkASSERT(range != SkIntToScalar(0));
    static const int kColorComponents = 3;

    // Figure out how to scale each color component.
    SkScalar multiplier[kColorComponents];
    for (int i = 0; i < kColorComponents; i++) {
        multiplier[i] = (curColor[i] - prevColor[i]) / range;
    }

    // Calculate when we no longer need to keep a copy of the input parameter t.
    // If the last component to use t is i, then dupInput[0..i - 1] = true
    // and dupInput[i .. components] = false.
    bool dupInput[kColorComponents];
    dupInput[kColorComponents - 1] = false;
    for (int i = kColorComponents - 2; i >= 0; i--) {
        dupInput[i] = dupInput[i + 1] || multiplier[i + 1] != 0;
    }

    if (!dupInput[0] && multiplier[0] == 0) {
        result->append("pop ");
    }

    for (int i = 0; i < kColorComponents; i++) {
        // If the next components needs t and this component will consume a
        // copy, make another copy.
        if (dupInput[i] && multiplier[i] != 0) {
            result->append("dup ");
        }

        if (multiplier[i] == 0) {
            result->appendScalar(prevColor[i]);
            result->append(" ");
        } else {
            if (multiplier[i] != 1) {
                result->appendScalar(multiplier[i]);
                result->append(" mul ");
            }
            if (prevColor[i] != 0) {
                result->appendScalar(prevColor[i]);
                result->append(" add ");
            }
        }

        if (dupInput[i]) {
            result->append("exch\n");
        }
    }
}

/* Generate Type 4 function code to map t=[0,1) to the passed gradient,
   clamping at the edges of the range.  The generated code will be of the form:
       if (t < 0) {
           return colorData[0][r,g,b];
       } else {
           if (t < info.fColorOffsets[1]) {
               return linearinterpolation(colorData[0][r,g,b],
                                          colorData[1][r,g,b]);
           } else {
               if (t < info.fColorOffsets[2]) {
                   return linearinterpolation(colorData[1][r,g,b],
                                              colorData[2][r,g,b]);
               } else {

                ...    } else {
                           return colorData[info.fColorCount - 1][r,g,b];
                       }
                ...
           }
       }
 */
static void gradientFunctionCode(const SkShader::GradientInfo& info,
                                 SkString* result) {
    /* We want to linearly interpolate from the previous color to the next.
       Scale the colors from 0..255 to 0..1 and determine the multipliers
       for interpolation.
       C{r,g,b}(t, section) = t - offset_(section-1) + t * Multiplier{r,g,b}.
     */
    static const int kColorComponents = 3;
    typedef SkScalar ColorTuple[kColorComponents];
    SkAutoSTMalloc<4, ColorTuple> colorDataAlloc(info.fColorCount);
    ColorTuple *colorData = colorDataAlloc.get();
    const SkScalar scale = SkScalarInvert(SkIntToScalar(255));
    for (int i = 0; i < info.fColorCount; i++) {
        colorData[i][0] = SkScalarMul(SkColorGetR(info.fColors[i]), scale);
        colorData[i][1] = SkScalarMul(SkColorGetG(info.fColors[i]), scale);
        colorData[i][2] = SkScalarMul(SkColorGetB(info.fColors[i]), scale);
    }

    // Clamp the initial color.
    result->append("dup 0 le {pop ");
    result->appendScalar(colorData[0][0]);
    result->append(" ");
    result->appendScalar(colorData[0][1]);
    result->append(" ");
    result->appendScalar(colorData[0][2]);
    result->append(" }\n");

    // The gradient colors.
    int gradients = 0;
    for (int i = 1 ; i < info.fColorCount; i++) {
        if (info.fColorOffsets[i] == info.fColorOffsets[i - 1]) {
            continue;
        }
        gradients++;

        result->append("{dup ");
        result->appendScalar(info.fColorOffsets[i]);
        result->append(" le {");
        if (info.fColorOffsets[i - 1] != 0) {
            result->appendScalar(info.fColorOffsets[i - 1]);
            result->append(" sub\n");
        }

        interpolateColorCode(info.fColorOffsets[i] - info.fColorOffsets[i - 1],
                             colorData[i], colorData[i - 1], result);
        result->append("}\n");
    }

    // Clamp the final color.
    result->append("{pop ");
    result->appendScalar(colorData[info.fColorCount - 1][0]);
    result->append(" ");
    result->appendScalar(colorData[info.fColorCount - 1][1]);
    result->append(" ");
    result->appendScalar(colorData[info.fColorCount - 1][2]);

    for (int i = 0 ; i < gradients + 1; i++) {
        result->append("} ifelse\n");
    }
}

/* Map a value of t on the stack into [0, 1) for Repeat or Mirror tile mode. */
static void tileModeCode(SkShader::TileMode mode, SkString* result) {
    if (mode == SkShader::kRepeat_TileMode) {
        result->append("dup truncate sub\n");  // Get the fractional part.
        result->append("dup 0 le {1 add} if\n");  // Map (-1,0) => (0,1)
        return;
    }

    if (mode == SkShader::kMirror_TileMode) {
        // Map t mod 2 into [0, 1, 1, 0].
        //               Code                     Stack
        result->append("abs "                 // Map negative to positive.
                       "dup "                 // t.s t.s
                       "truncate "            // t.s t
                       "dup "                 // t.s t t
                       "cvi "                 // t.s t T
                       "2 mod "               // t.s t (i mod 2)
                       "1 eq "                // t.s t true|false
                       "3 1 roll "            // true|false t.s t
                       "sub "                 // true|false 0.s
                       "exch "                // 0.s true|false
                       "{1 exch sub} if\n");  // 1 - 0.s|0.s
    }
}

/**
 *  Returns PS function code that applies inverse perspective
 *  to a x, y point.
 *  The function assumes that the stack has at least two elements,
 *  and that the top 2 elements are numeric values.
 *  After executing this code on a PS stack, the last 2 elements are updated
 *  while the rest of the stack is preserved intact.
 *  inversePerspectiveMatrix is the inverse perspective matrix.
 */
static SkString apply_perspective_to_coordinates(
        const SkMatrix& inversePerspectiveMatrix) {
    SkString code;
    if (!inversePerspectiveMatrix.hasPerspective()) {
        return code;
    }

    // Perspective matrix should be:
    // 1   0  0
    // 0   1  0
    // p0 p1 p2

    const SkScalar p0 = inversePerspectiveMatrix[SkMatrix::kMPersp0];
    const SkScalar p1 = inversePerspectiveMatrix[SkMatrix::kMPersp1];
    const SkScalar p2 = inversePerspectiveMatrix[SkMatrix::kMPersp2];

    // y = y / (p2 + p0 x + p1 y)
    // x = x / (p2 + p0 x + p1 y)

    // Input on stack: x y
    code.append(" dup ");               // x y y
    code.appendScalar(p1);              // x y y p1
    code.append(" mul "                 // x y y*p1
                " 2 index ");           // x y y*p1 x
    code.appendScalar(p0);              // x y y p1 x p0
    code.append(" mul ");               // x y y*p1 x*p0
    code.appendScalar(p2);              // x y y p1 x*p0 p2
    code.append(" add "                 // x y y*p1 x*p0+p2
                "add "                  // x y y*p1+x*p0+p2
                "3 1 roll "             // y*p1+x*p0+p2 x y
                "2 index "              // z x y y*p1+x*p0+p2
                "div "                  // y*p1+x*p0+p2 x y/(y*p1+x*p0+p2)
                "3 1 roll "             // y/(y*p1+x*p0+p2) y*p1+x*p0+p2 x
                "exch "                 // y/(y*p1+x*p0+p2) x y*p1+x*p0+p2
                "div "                  // y/(y*p1+x*p0+p2) x/(y*p1+x*p0+p2)
                "exch\n");              // x/(y*p1+x*p0+p2) y/(y*p1+x*p0+p2)
    return code;
}

static SkString linearCode(const SkShader::GradientInfo& info,
                           const SkMatrix& perspectiveRemover) {
    SkString function("{");

    function.append(apply_perspective_to_coordinates(perspectiveRemover));

    function.append("pop\n");  // Just ditch the y value.
    tileModeCode(info.fTileMode, &function);
    gradientFunctionCode(info, &function);
    function.append("}");
    return function;
}

static SkString radialCode(const SkShader::GradientInfo& info,
                           const SkMatrix& perspectiveRemover) {
    SkString function("{");

    function.append(apply_perspective_to_coordinates(perspectiveRemover));

    // Find the distance from the origin.
    function.append("dup "      // x y y
                    "mul "      // x y^2
                    "exch "     // y^2 x
                    "dup "      // y^2 x x
                    "mul "      // y^2 x^2
                    "add "      // y^2+x^2
                    "sqrt\n");  // sqrt(y^2+x^2)

    tileModeCode(info.fTileMode, &function);
    gradientFunctionCode(info, &function);
    function.append("}");
    return function;
}

/* Conical gradient shader, based on the Canvas spec for radial gradients
   See: http://www.w3.org/TR/2dcontext/#dom-context-2d-createradialgradient
 */
static SkString twoPointConicalCode(const SkShader::GradientInfo& info,
                                    const SkMatrix& perspectiveRemover) {
    SkScalar dx = info.fPoint[1].fX - info.fPoint[0].fX;
    SkScalar dy = info.fPoint[1].fY - info.fPoint[0].fY;
    SkScalar r0 = info.fRadius[0];
    SkScalar dr = info.fRadius[1] - info.fRadius[0];
    SkScalar a = SkScalarMul(dx, dx) + SkScalarMul(dy, dy) -
                 SkScalarMul(dr, dr);

    // First compute t, if the pixel falls outside the cone, then we'll end
    // with 'false' on the stack, otherwise we'll push 'true' with t below it

    // We start with a stack of (x y), copy it and then consume one copy in
    // order to calculate b and the other to calculate c.
    SkString function("{");

    function.append(apply_perspective_to_coordinates(perspectiveRemover));

    function.append("2 copy ");

    // Calculate b and b^2; b = -2 * (y * dy + x * dx + r0 * dr).
    function.appendScalar(dy);
    function.append(" mul exch ");
    function.appendScalar(dx);
    function.append(" mul add ");
    function.appendScalar(SkScalarMul(r0, dr));
    function.append(" add -2 mul dup dup mul\n");

    // c = x^2 + y^2 + radius0^2
    function.append("4 2 roll dup mul exch dup mul add ");
    function.appendScalar(SkScalarMul(r0, r0));
    function.append(" sub dup 4 1 roll\n");

    // Contents of the stack at this point: c, b, b^2, c

    // if a = 0, then we collapse to a simpler linear case
    if (a == 0) {

        // t = -c/b
        function.append("pop pop div neg dup ");

        // compute radius(t)
        function.appendScalar(dr);
        function.append(" mul ");
        function.appendScalar(r0);
        function.append(" add\n");

        // if r(t) < 0, then it's outside the cone
        function.append("0 lt {pop false} {true} ifelse\n");

    } else {

        // quadratic case: the Canvas spec wants the largest
        // root t for which radius(t) > 0

        // compute the discriminant (b^2 - 4ac)
        function.appendScalar(SkScalarMul(SkIntToScalar(4), a));
        function.append(" mul sub dup\n");

        // if d >= 0, proceed
        function.append("0 ge {\n");

        // an intermediate value we'll use to compute the roots:
        // q = -0.5 * (b +/- sqrt(d))
        function.append("sqrt exch dup 0 lt {exch -1 mul} if");
        function.append(" add -0.5 mul dup\n");

        // first root = q / a
        function.appendScalar(a);
        function.append(" div\n");

        // second root = c / q
        function.append("3 1 roll div\n");

        // put the larger root on top of the stack
        function.append("2 copy gt {exch} if\n");

        // compute radius(t) for larger root
        function.append("dup ");
        function.appendScalar(dr);
        function.append(" mul ");
        function.appendScalar(r0);
        function.append(" add\n");

        // if r(t) > 0, we have our t, pop off the smaller root and we're done
        function.append(" 0 gt {exch pop true}\n");

        // otherwise, throw out the larger one and try the smaller root
        function.append("{pop dup\n");
        function.appendScalar(dr);
        function.append(" mul ");
        function.appendScalar(r0);
        function.append(" add\n");

        // if r(t) < 0, push false, otherwise the smaller root is our t
        function.append("0 le {pop false} {true} ifelse\n");
        function.append("} ifelse\n");

        // d < 0, clear the stack and push false
        function.append("} {pop pop pop false} ifelse\n");
    }

    // if the pixel is in the cone, proceed to compute a color
    function.append("{");
    tileModeCode(info.fTileMode, &function);
    gradientFunctionCode(info, &function);

    // otherwise, just write black
    function.append("} {0 0 0} ifelse }");

    return function;
}

static SkString sweepCode(const SkShader::GradientInfo& info,
                          const SkMatrix& perspectiveRemover) {
    SkString function("{exch atan 360 div\n");
    tileModeCode(info.fTileMode, &function);
    gradientFunctionCode(info, &function);
    function.append("}");
    return function;
}

static void drawBitmapMatrix(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& matrix) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(matrix);
    canvas->drawBitmap(bm, 0, 0);
}

class SkPDFShader::State {
public:
    SkShader::GradientType fType;
    SkShader::GradientInfo fInfo;
    SkAutoFree fColorData;    // This provides storage for arrays in fInfo.
    SkMatrix fCanvasTransform;
    SkMatrix fShaderTransform;
    SkIRect fBBox;

    SkBitmap fImage;
    uint32_t fPixelGeneration;
    SkShader::TileMode fImageTileModes[2];

    State(const SkShader& shader, const SkMatrix& canvasTransform,
          const SkIRect& bbox, SkScalar rasterScale);

    bool operator==(const State& b) const;

    SkPDFShader::State* CreateAlphaToLuminosityState() const;
    SkPDFShader::State* CreateOpaqueState() const;

    bool GradientHasAlpha() const;

private:
    State(const State& other);
    State operator=(const State& rhs);
    void AllocateGradientInfoStorage();
};

////////////////////////////////////////////////////////////////////////////////

SkPDFFunctionShader::SkPDFFunctionShader(SkPDFShader::State* state)
    : SkPDFDict("Pattern"), fShaderState(state) {}

SkPDFFunctionShader::~SkPDFFunctionShader() {}

bool SkPDFFunctionShader::equals(const SkPDFShader::State& state) const {
    return state == *fShaderState;
}

////////////////////////////////////////////////////////////////////////////////

SkPDFAlphaFunctionShader::SkPDFAlphaFunctionShader(SkPDFShader::State* state)
    : fShaderState(state) {}

bool SkPDFAlphaFunctionShader::equals(const SkPDFShader::State& state) const {
    return state == *fShaderState;
}

SkPDFAlphaFunctionShader::~SkPDFAlphaFunctionShader() {}

////////////////////////////////////////////////////////////////////////////////

SkPDFImageShader::SkPDFImageShader(SkPDFShader::State* state)
    : fShaderState(state) {}

bool SkPDFImageShader::equals(const SkPDFShader::State& state) const {
    return state == *fShaderState;
}

SkPDFImageShader::~SkPDFImageShader() {}

////////////////////////////////////////////////////////////////////////////////

static SkPDFObject* get_pdf_shader_by_state(
        SkPDFCanon* canon,
        SkScalar dpi,
        SkAutoTDelete<SkPDFShader::State>* autoState) {
    const SkPDFShader::State& state = **autoState;
    if (state.fType == SkShader::kNone_GradientType && state.fImage.isNull()) {
        // TODO(vandebo) This drops SKComposeShader on the floor.  We could
        // handle compose shader by pulling things up to a layer, drawing with
        // the first shader, applying the xfer mode and drawing again with the
        // second shader, then applying the layer to the original drawing.
        return NULL;
    } else if (state.fType == SkShader::kNone_GradientType) {
        SkPDFObject* shader = canon->findImageShader(state);
        return shader ? SkRef(shader)
                      : SkPDFImageShader::Create(canon, dpi, autoState);
    } else if (state.GradientHasAlpha()) {
        SkPDFObject* shader = canon->findAlphaShader(state);
        return shader ? SkRef(shader)
                      : SkPDFAlphaFunctionShader::Create(canon, dpi, autoState);
    } else {
        SkPDFObject* shader = canon->findFunctionShader(state);
        return shader ? SkRef(shader)
                      : SkPDFFunctionShader::Create(canon, autoState);
    }
}

// static
SkPDFObject* SkPDFShader::GetPDFShader(SkPDFCanon* canon,
                                       SkScalar dpi,
                                       const SkShader& shader,
                                       const SkMatrix& matrix,
                                       const SkIRect& surfaceBBox,
                                       SkScalar rasterScale) {
    SkAutoTDelete<SkPDFShader::State> state(
            SkNEW_ARGS(State, (shader, matrix, surfaceBBox, rasterScale)));
    return get_pdf_shader_by_state(canon, dpi, &state);
}

static SkPDFDict* get_gradient_resource_dict(
        SkPDFObject* functionShader,
        SkPDFObject* gState) {
    SkTDArray<SkPDFObject*> patterns;
    if (functionShader) {
        patterns.push(functionShader);
    }
    SkTDArray<SkPDFObject*> graphicStates;
    if (gState) {
        graphicStates.push(gState);
    }
    return SkPDFResourceDict::Create(&graphicStates, &patterns, NULL, NULL);
}

static void populate_tiling_pattern_dict(SkPDFDict* pattern,
                                         SkRect& bbox,
                                         SkPDFDict* resources,
                                         const SkMatrix& matrix) {
    const int kTiling_PatternType = 1;
    const int kColoredTilingPattern_PaintType = 1;
    const int kConstantSpacing_TilingType = 1;

    pattern->insertName("Type", "Pattern");
    pattern->insertInt("PatternType", kTiling_PatternType);
    pattern->insertInt("PaintType", kColoredTilingPattern_PaintType);
    pattern->insertInt("TilingType", kConstantSpacing_TilingType);
    pattern->insertObject("BBox", SkPDFUtils::RectToArray(bbox));
    pattern->insertScalar("XStep", bbox.width());
    pattern->insertScalar("YStep", bbox.height());
    pattern->insertObject("Resources", SkRef(resources));
    if (!matrix.isIdentity()) {
        pattern->insertObject("Matrix", SkPDFUtils::MatrixToArray(matrix));
    }
}

/**
 * Creates a content stream which fills the pattern P0 across bounds.
 * @param gsIndex A graphics state resource index to apply, or <0 if no
 * graphics state to apply.
 */
static SkStream* create_pattern_fill_content(int gsIndex, SkRect& bounds) {
    SkDynamicMemoryWStream content;
    if (gsIndex >= 0) {
        SkPDFUtils::ApplyGraphicState(gsIndex, &content);
    }
    SkPDFUtils::ApplyPattern(0, &content);
    SkPDFUtils::AppendRectangle(bounds, &content);
    SkPDFUtils::PaintPath(SkPaint::kFill_Style, SkPath::kEvenOdd_FillType,
                          &content);

    return content.detachAsStream();
}

/**
 * Creates a ExtGState with the SMask set to the luminosityShader in
 * luminosity mode. The shader pattern extends to the bbox.
 */
static SkPDFObject* create_smask_graphic_state(
        SkPDFCanon* canon, SkScalar dpi, const SkPDFShader::State& state) {
    SkRect bbox;
    bbox.set(state.fBBox);

    SkAutoTDelete<SkPDFShader::State> alphaToLuminosityState(
            state.CreateAlphaToLuminosityState());
    SkAutoTUnref<SkPDFObject> luminosityShader(
            get_pdf_shader_by_state(canon, dpi, &alphaToLuminosityState));

    SkAutoTDelete<SkStream> alphaStream(create_pattern_fill_content(-1, bbox));

    SkAutoTUnref<SkPDFDict>
        resources(get_gradient_resource_dict(luminosityShader, NULL));

    SkAutoTUnref<SkPDFFormXObject> alphaMask(
            new SkPDFFormXObject(alphaStream.get(), bbox, resources.get()));

    return SkPDFGraphicState::GetSMaskGraphicState(
            alphaMask.get(), false,
            SkPDFGraphicState::kLuminosity_SMaskMode);
}

SkPDFAlphaFunctionShader* SkPDFAlphaFunctionShader::Create(
        SkPDFCanon* canon,
        SkScalar dpi,
        SkAutoTDelete<SkPDFShader::State>* autoState) {
    const SkPDFShader::State& state = **autoState;
    SkRect bbox;
    bbox.set(state.fBBox);

    SkAutoTDelete<SkPDFShader::State> opaqueState(state.CreateOpaqueState());

    SkAutoTUnref<SkPDFObject> colorShader(
            get_pdf_shader_by_state(canon, dpi, &opaqueState));
    if (!colorShader) {
        return NULL;
    }

    // Create resource dict with alpha graphics state as G0 and
    // pattern shader as P0, then write content stream.
    SkAutoTUnref<SkPDFObject> alphaGs(
            create_smask_graphic_state(canon, dpi, state));

    SkPDFAlphaFunctionShader* alphaFunctionShader =
            SkNEW_ARGS(SkPDFAlphaFunctionShader, (autoState->detach()));

    SkAutoTUnref<SkPDFDict> resourceDict(
            get_gradient_resource_dict(colorShader.get(), alphaGs.get()));

    SkAutoTDelete<SkStream> colorStream(
            create_pattern_fill_content(0, bbox));
    alphaFunctionShader->setData(colorStream.get());

    populate_tiling_pattern_dict(alphaFunctionShader, bbox, resourceDict.get(),
                                 SkMatrix::I());
    canon->addAlphaShader(alphaFunctionShader);
    return alphaFunctionShader;
}

// Finds affine and persp such that in = affine * persp.
// but it returns the inverse of perspective matrix.
static bool split_perspective(const SkMatrix in, SkMatrix* affine,
                              SkMatrix* perspectiveInverse) {
    const SkScalar p2 = in[SkMatrix::kMPersp2];

    if (SkScalarNearlyZero(p2)) {
        return false;
    }

    const SkScalar zero = SkIntToScalar(0);
    const SkScalar one = SkIntToScalar(1);

    const SkScalar sx = in[SkMatrix::kMScaleX];
    const SkScalar kx = in[SkMatrix::kMSkewX];
    const SkScalar tx = in[SkMatrix::kMTransX];
    const SkScalar ky = in[SkMatrix::kMSkewY];
    const SkScalar sy = in[SkMatrix::kMScaleY];
    const SkScalar ty = in[SkMatrix::kMTransY];
    const SkScalar p0 = in[SkMatrix::kMPersp0];
    const SkScalar p1 = in[SkMatrix::kMPersp1];

    // Perspective matrix would be:
    // 1  0  0
    // 0  1  0
    // p0 p1 p2
    // But we need the inverse of persp.
    perspectiveInverse->setAll(one,          zero,       zero,
                               zero,         one,        zero,
                               -p0/p2,     -p1/p2,     1/p2);

    affine->setAll(sx - p0 * tx / p2,       kx - p1 * tx / p2,      tx / p2,
                   ky - p0 * ty / p2,       sy - p1 * ty / p2,      ty / p2,
                   zero,                    zero,                   one);

    return true;
}

namespace {
SkPDFObject* create_range_object() {
    SkPDFArray* range = SkNEW(SkPDFArray);
    range->reserve(6);
    range->appendInt(0);
    range->appendInt(1);
    range->appendInt(0);
    range->appendInt(1);
    range->appendInt(0);
    range->appendInt(1);
    return range;
}

template <typename T> void unref(T* ptr) { ptr->unref();}
}  // namespace

SK_DECLARE_STATIC_LAZY_PTR(SkPDFObject, rangeObject,
                           create_range_object, unref<SkPDFObject>);

static SkPDFStream* make_ps_function(const SkString& psCode,
                                     SkPDFArray* domain) {
    SkAutoDataUnref funcData(
            SkData::NewWithCopy(psCode.c_str(), psCode.size()));
    SkPDFStream* result = SkNEW_ARGS(SkPDFStream, (funcData.get()));
    result->insertInt("FunctionType", 4);
    result->insertObject("Domain", SkRef(domain));
    result->insertObject("Range", SkRef(rangeObject.get()));
    return result;
}

SkPDFFunctionShader* SkPDFFunctionShader::Create(
        SkPDFCanon* canon, SkAutoTDelete<SkPDFShader::State>* autoState) {
    const SkPDFShader::State& state = **autoState;

    SkString (*codeFunction)(const SkShader::GradientInfo& info,
                             const SkMatrix& perspectiveRemover) = NULL;
    SkPoint transformPoints[2];

    // Depending on the type of the gradient, we want to transform the
    // coordinate space in different ways.
    const SkShader::GradientInfo* info = &state.fInfo;
    transformPoints[0] = info->fPoint[0];
    transformPoints[1] = info->fPoint[1];
    switch (state.fType) {
        case SkShader::kLinear_GradientType:
            codeFunction = &linearCode;
            break;
        case SkShader::kRadial_GradientType:
            transformPoints[1] = transformPoints[0];
            transformPoints[1].fX += info->fRadius[0];
            codeFunction = &radialCode;
            break;
        case SkShader::kConical_GradientType: {
            transformPoints[1] = transformPoints[0];
            transformPoints[1].fX += SK_Scalar1;
            codeFunction = &twoPointConicalCode;
            break;
        }
        case SkShader::kSweep_GradientType:
            transformPoints[1] = transformPoints[0];
            transformPoints[1].fX += SK_Scalar1;
            codeFunction = &sweepCode;
            break;
        case SkShader::kColor_GradientType:
        case SkShader::kNone_GradientType:
        default:
            return NULL;
    }

    // Move any scaling (assuming a unit gradient) or translation
    // (and rotation for linear gradient), of the final gradient from
    // info->fPoints to the matrix (updating bbox appropriately).  Now
    // the gradient can be drawn on on the unit segment.
    SkMatrix mapperMatrix;
    unitToPointsMatrix(transformPoints, &mapperMatrix);

    SkMatrix finalMatrix = state.fCanvasTransform;
    finalMatrix.preConcat(state.fShaderTransform);
    finalMatrix.preConcat(mapperMatrix);

    // Preserves as much as posible in the final matrix, and only removes
    // the perspective. The inverse of the perspective is stored in
    // perspectiveInverseOnly matrix and has 3 useful numbers
    // (p0, p1, p2), while everything else is either 0 or 1.
    // In this way the shader will handle it eficiently, with minimal code.
    SkMatrix perspectiveInverseOnly = SkMatrix::I();
    if (finalMatrix.hasPerspective()) {
        if (!split_perspective(finalMatrix,
                               &finalMatrix, &perspectiveInverseOnly)) {
            return NULL;
        }
    }

    SkRect bbox;
    bbox.set(state.fBBox);
    if (!inverse_transform_bbox(finalMatrix, &bbox)) {
        return NULL;
    }

    SkAutoTUnref<SkPDFArray> domain(new SkPDFArray);
    domain->reserve(4);
    domain->appendScalar(bbox.fLeft);
    domain->appendScalar(bbox.fRight);
    domain->appendScalar(bbox.fTop);
    domain->appendScalar(bbox.fBottom);

    SkString functionCode;
    // The two point radial gradient further references
    // state.fInfo
    // in translating from x, y coordinates to the t parameter. So, we have
    // to transform the points and radii according to the calculated matrix.
    if (state.fType == SkShader::kConical_GradientType) {
        SkShader::GradientInfo twoPointRadialInfo = *info;
        SkMatrix inverseMapperMatrix;
        if (!mapperMatrix.invert(&inverseMapperMatrix)) {
            return NULL;
        }
        inverseMapperMatrix.mapPoints(twoPointRadialInfo.fPoint, 2);
        twoPointRadialInfo.fRadius[0] =
            inverseMapperMatrix.mapRadius(info->fRadius[0]);
        twoPointRadialInfo.fRadius[1] =
            inverseMapperMatrix.mapRadius(info->fRadius[1]);
        functionCode = codeFunction(twoPointRadialInfo, perspectiveInverseOnly);
    } else {
        functionCode = codeFunction(*info, perspectiveInverseOnly);
    }

    SkAutoTUnref<SkPDFDict> pdfShader(new SkPDFDict);
    pdfShader->insertInt("ShadingType", 1);
    pdfShader->insertName("ColorSpace", "DeviceRGB");
    pdfShader->insertObject("Domain", SkRef(domain.get()));

    SkAutoTUnref<SkPDFStream> function(
            make_ps_function(functionCode, domain.get()));
    pdfShader->insertObjRef("Function", function.detach());

    SkPDFFunctionShader* pdfFunctionShader =
            SkNEW_ARGS(SkPDFFunctionShader, (autoState->detach()));

    pdfFunctionShader->insertInt("PatternType", 2);
    pdfFunctionShader->insertObject("Matrix",
                                    SkPDFUtils::MatrixToArray(finalMatrix));
    pdfFunctionShader->insertObject("Shading", pdfShader.detach());

    canon->addFunctionShader(pdfFunctionShader);
    return pdfFunctionShader;
}

SkPDFImageShader* SkPDFImageShader::Create(
        SkPDFCanon* canon,
        SkScalar dpi,
        SkAutoTDelete<SkPDFShader::State>* autoState) {
    const SkPDFShader::State& state = **autoState;

    state.fImage.lockPixels();

    // The image shader pattern cell will be drawn into a separate device
    // in pattern cell space (no scaling on the bitmap, though there may be
    // translations so that all content is in the device, coordinates > 0).

    // Map clip bounds to shader space to ensure the device is large enough
    // to handle fake clamping.
    SkMatrix finalMatrix = state.fCanvasTransform;
    finalMatrix.preConcat(state.fShaderTransform);
    SkRect deviceBounds;
    deviceBounds.set(state.fBBox);
    if (!inverse_transform_bbox(finalMatrix, &deviceBounds)) {
        return NULL;
    }

    const SkBitmap* image = &state.fImage;
    SkRect bitmapBounds;
    image->getBounds(&bitmapBounds);

    // For tiling modes, the bounds should be extended to include the bitmap,
    // otherwise the bitmap gets clipped out and the shader is empty and awful.
    // For clamp modes, we're only interested in the clip region, whether
    // or not the main bitmap is in it.
    SkShader::TileMode tileModes[2];
    tileModes[0] = state.fImageTileModes[0];
    tileModes[1] = state.fImageTileModes[1];
    if (tileModes[0] != SkShader::kClamp_TileMode ||
            tileModes[1] != SkShader::kClamp_TileMode) {
        deviceBounds.join(bitmapBounds);
    }

    SkISize size = SkISize::Make(SkScalarRoundToInt(deviceBounds.width()),
                                 SkScalarRoundToInt(deviceBounds.height()));
    SkAutoTUnref<SkPDFDevice> patternDevice(
            SkPDFDevice::CreateUnflipped(size, dpi, canon));
    SkCanvas canvas(patternDevice.get());

    SkRect patternBBox;
    image->getBounds(&patternBBox);

    // Translate the canvas so that the bitmap origin is at (0, 0).
    canvas.translate(-deviceBounds.left(), -deviceBounds.top());
    patternBBox.offset(-deviceBounds.left(), -deviceBounds.top());
    // Undo the translation in the final matrix
    finalMatrix.preTranslate(deviceBounds.left(), deviceBounds.top());

    // If the bitmap is out of bounds (i.e. clamp mode where we only see the
    // stretched sides), canvas will clip this out and the extraneous data
    // won't be saved to the PDF.
    canvas.drawBitmap(*image, 0, 0);

    SkScalar width = SkIntToScalar(image->width());
    SkScalar height = SkIntToScalar(image->height());

    // Tiling is implied.  First we handle mirroring.
    if (tileModes[0] == SkShader::kMirror_TileMode) {
        SkMatrix xMirror;
        xMirror.setScale(-1, 1);
        xMirror.postTranslate(2 * width, 0);
        drawBitmapMatrix(&canvas, *image, xMirror);
        patternBBox.fRight += width;
    }
    if (tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix yMirror;
        yMirror.setScale(SK_Scalar1, -SK_Scalar1);
        yMirror.postTranslate(0, 2 * height);
        drawBitmapMatrix(&canvas, *image, yMirror);
        patternBBox.fBottom += height;
    }
    if (tileModes[0] == SkShader::kMirror_TileMode &&
            tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix mirror;
        mirror.setScale(-1, -1);
        mirror.postTranslate(2 * width, 2 * height);
        drawBitmapMatrix(&canvas, *image, mirror);
    }

    // Then handle Clamping, which requires expanding the pattern canvas to
    // cover the entire surfaceBBox.

    // If both x and y are in clamp mode, we start by filling in the corners.
    // (Which are just a rectangles of the corner colors.)
    if (tileModes[0] == SkShader::kClamp_TileMode &&
            tileModes[1] == SkShader::kClamp_TileMode) {
        SkPaint paint;
        SkRect rect;
        rect = SkRect::MakeLTRB(deviceBounds.left(), deviceBounds.top(), 0, 0);
        if (!rect.isEmpty()) {
            paint.setColor(image->getColor(0, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, deviceBounds.top(),
                                deviceBounds.right(), 0);
        if (!rect.isEmpty()) {
            paint.setColor(image->getColor(image->width() - 1, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, height,
                                deviceBounds.right(), deviceBounds.bottom());
        if (!rect.isEmpty()) {
            paint.setColor(image->getColor(image->width() - 1,
                                           image->height() - 1));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(deviceBounds.left(), height,
                                0, deviceBounds.bottom());
        if (!rect.isEmpty()) {
            paint.setColor(image->getColor(0, image->height() - 1));
            canvas.drawRect(rect, paint);
        }
    }

    // Then expand the left, right, top, then bottom.
    if (tileModes[0] == SkShader::kClamp_TileMode) {
        SkIRect subset = SkIRect::MakeXYWH(0, 0, 1, image->height());
        if (deviceBounds.left() < 0) {
            SkBitmap left;
            SkAssertResult(image->extractSubset(&left, subset));

            SkMatrix leftMatrix;
            leftMatrix.setScale(-deviceBounds.left(), 1);
            leftMatrix.postTranslate(deviceBounds.left(), 0);
            drawBitmapMatrix(&canvas, left, leftMatrix);

            if (tileModes[1] == SkShader::kMirror_TileMode) {
                leftMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                leftMatrix.postTranslate(0, 2 * height);
                drawBitmapMatrix(&canvas, left, leftMatrix);
            }
            patternBBox.fLeft = 0;
        }

        if (deviceBounds.right() > width) {
            SkBitmap right;
            subset.offset(image->width() - 1, 0);
            SkAssertResult(image->extractSubset(&right, subset));

            SkMatrix rightMatrix;
            rightMatrix.setScale(deviceBounds.right() - width, 1);
            rightMatrix.postTranslate(width, 0);
            drawBitmapMatrix(&canvas, right, rightMatrix);

            if (tileModes[1] == SkShader::kMirror_TileMode) {
                rightMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                rightMatrix.postTranslate(0, 2 * height);
                drawBitmapMatrix(&canvas, right, rightMatrix);
            }
            patternBBox.fRight = deviceBounds.width();
        }
    }

    if (tileModes[1] == SkShader::kClamp_TileMode) {
        SkIRect subset = SkIRect::MakeXYWH(0, 0, image->width(), 1);
        if (deviceBounds.top() < 0) {
            SkBitmap top;
            SkAssertResult(image->extractSubset(&top, subset));

            SkMatrix topMatrix;
            topMatrix.setScale(SK_Scalar1, -deviceBounds.top());
            topMatrix.postTranslate(0, deviceBounds.top());
            drawBitmapMatrix(&canvas, top, topMatrix);

            if (tileModes[0] == SkShader::kMirror_TileMode) {
                topMatrix.postScale(-1, 1);
                topMatrix.postTranslate(2 * width, 0);
                drawBitmapMatrix(&canvas, top, topMatrix);
            }
            patternBBox.fTop = 0;
        }

        if (deviceBounds.bottom() > height) {
            SkBitmap bottom;
            subset.offset(0, image->height() - 1);
            SkAssertResult(image->extractSubset(&bottom, subset));

            SkMatrix bottomMatrix;
            bottomMatrix.setScale(SK_Scalar1, deviceBounds.bottom() - height);
            bottomMatrix.postTranslate(0, height);
            drawBitmapMatrix(&canvas, bottom, bottomMatrix);

            if (tileModes[0] == SkShader::kMirror_TileMode) {
                bottomMatrix.postScale(-1, 1);
                bottomMatrix.postTranslate(2 * width, 0);
                drawBitmapMatrix(&canvas, bottom, bottomMatrix);
            }
            patternBBox.fBottom = deviceBounds.height();
        }
    }

    // Put the canvas into the pattern stream (fContent).
    SkAutoTDelete<SkStreamAsset> content(patternDevice->content());

    SkPDFImageShader* imageShader =
            SkNEW_ARGS(SkPDFImageShader, (autoState->detach()));
    imageShader->setData(content.get());

    SkAutoTUnref<SkPDFDict> resourceDict(
            patternDevice->createResourceDict());
    populate_tiling_pattern_dict(imageShader, patternBBox,
                                 resourceDict.get(), finalMatrix);

    imageShader->fShaderState->fImage.unlockPixels();

    canon->addImageShader(imageShader);
    return imageShader;
}

bool SkPDFShader::State::operator==(const SkPDFShader::State& b) const {
    if (fType != b.fType ||
            fCanvasTransform != b.fCanvasTransform ||
            fShaderTransform != b.fShaderTransform ||
            fBBox != b.fBBox) {
        return false;
    }

    if (fType == SkShader::kNone_GradientType) {
        if (fPixelGeneration != b.fPixelGeneration ||
                fPixelGeneration == 0 ||
                fImageTileModes[0] != b.fImageTileModes[0] ||
                fImageTileModes[1] != b.fImageTileModes[1]) {
            return false;
        }
    } else {
        if (fInfo.fColorCount != b.fInfo.fColorCount ||
                memcmp(fInfo.fColors, b.fInfo.fColors,
                       sizeof(SkColor) * fInfo.fColorCount) != 0 ||
                memcmp(fInfo.fColorOffsets, b.fInfo.fColorOffsets,
                       sizeof(SkScalar) * fInfo.fColorCount) != 0 ||
                fInfo.fPoint[0] != b.fInfo.fPoint[0] ||
                fInfo.fTileMode != b.fInfo.fTileMode) {
            return false;
        }

        switch (fType) {
            case SkShader::kLinear_GradientType:
                if (fInfo.fPoint[1] != b.fInfo.fPoint[1]) {
                    return false;
                }
                break;
            case SkShader::kRadial_GradientType:
                if (fInfo.fRadius[0] != b.fInfo.fRadius[0]) {
                    return false;
                }
                break;
            case SkShader::kConical_GradientType:
                if (fInfo.fPoint[1] != b.fInfo.fPoint[1] ||
                        fInfo.fRadius[0] != b.fInfo.fRadius[0] ||
                        fInfo.fRadius[1] != b.fInfo.fRadius[1]) {
                    return false;
                }
                break;
            case SkShader::kSweep_GradientType:
            case SkShader::kNone_GradientType:
            case SkShader::kColor_GradientType:
                break;
        }
    }
    return true;
}

SkPDFShader::State::State(const SkShader& shader, const SkMatrix& canvasTransform,
                          const SkIRect& bbox, SkScalar rasterScale)
        : fCanvasTransform(canvasTransform),
          fBBox(bbox),
          fPixelGeneration(0) {
    fInfo.fColorCount = 0;
    fInfo.fColors = NULL;
    fInfo.fColorOffsets = NULL;
    fShaderTransform = shader.getLocalMatrix();
    fImageTileModes[0] = fImageTileModes[1] = SkShader::kClamp_TileMode;

    fType = shader.asAGradient(&fInfo);

    if (fType == SkShader::kNone_GradientType) {
        SkShader::BitmapType bitmapType;
        SkMatrix matrix;
        bitmapType = shader.asABitmap(&fImage, &matrix, fImageTileModes);
        if (bitmapType != SkShader::kDefault_BitmapType) {
            // Generic fallback for unsupported shaders:
            //  * allocate a bbox-sized bitmap
            //  * shade the whole area
            //  * use the result as a bitmap shader

            // bbox is in device space. While that's exactly what we want for sizing our bitmap,
            // we need to map it into shader space for adjustments (to match
            // SkPDFImageShader::Create's behavior).
            SkRect shaderRect = SkRect::Make(bbox);
            if (!inverse_transform_bbox(canvasTransform, &shaderRect)) {
                fImage.reset();
                return;
            }

            // Clamp the bitmap size to about 1M pixels
            static const SkScalar kMaxBitmapArea = 1024 * 1024;
            SkScalar bitmapArea = rasterScale * bbox.width() * rasterScale * bbox.height();
            if (bitmapArea > kMaxBitmapArea) {
                rasterScale *= SkScalarSqrt(kMaxBitmapArea / bitmapArea);
            }

            SkISize size = SkISize::Make(SkScalarRoundToInt(rasterScale * bbox.width()),
                                         SkScalarRoundToInt(rasterScale * bbox.height()));
            SkSize scale = SkSize::Make(SkIntToScalar(size.width()) / shaderRect.width(),
                                        SkIntToScalar(size.height()) / shaderRect.height());

            fImage.allocN32Pixels(size.width(), size.height());
            fImage.eraseColor(SK_ColorTRANSPARENT);

            SkPaint p;
            p.setShader(const_cast<SkShader*>(&shader));

            SkCanvas canvas(fImage);
            canvas.scale(scale.width(), scale.height());
            canvas.translate(-shaderRect.x(), -shaderRect.y());
            canvas.drawPaint(p);

            fShaderTransform.setTranslate(shaderRect.x(), shaderRect.y());
            fShaderTransform.preScale(1 / scale.width(), 1 / scale.height());
        } else {
            SkASSERT(matrix.isIdentity());
        }
        fPixelGeneration = fImage.getGenerationID();
    } else {
        AllocateGradientInfoStorage();
        shader.asAGradient(&fInfo);
    }
}

SkPDFShader::State::State(const SkPDFShader::State& other)
  : fType(other.fType),
    fCanvasTransform(other.fCanvasTransform),
    fShaderTransform(other.fShaderTransform),
    fBBox(other.fBBox)
{
    // Only gradients supported for now, since that is all that is used.
    // If needed, image state copy constructor can be added here later.
    SkASSERT(fType != SkShader::kNone_GradientType);

    if (fType != SkShader::kNone_GradientType) {
        fInfo = other.fInfo;

        AllocateGradientInfoStorage();
        for (int i = 0; i < fInfo.fColorCount; i++) {
            fInfo.fColors[i] = other.fInfo.fColors[i];
            fInfo.fColorOffsets[i] = other.fInfo.fColorOffsets[i];
        }
    }
}

/**
 * Create a copy of this gradient state with alpha assigned to RGB luminousity.
 * Only valid for gradient states.
 */
SkPDFShader::State* SkPDFShader::State::CreateAlphaToLuminosityState() const {
    SkASSERT(fType != SkShader::kNone_GradientType);

    SkPDFShader::State* newState = new SkPDFShader::State(*this);

    for (int i = 0; i < fInfo.fColorCount; i++) {
        SkAlpha alpha = SkColorGetA(fInfo.fColors[i]);
        newState->fInfo.fColors[i] = SkColorSetARGB(255, alpha, alpha, alpha);
    }

    return newState;
}

/**
 * Create a copy of this gradient state with alpha set to fully opaque
 * Only valid for gradient states.
 */
SkPDFShader::State* SkPDFShader::State::CreateOpaqueState() const {
    SkASSERT(fType != SkShader::kNone_GradientType);

    SkPDFShader::State* newState = new SkPDFShader::State(*this);
    for (int i = 0; i < fInfo.fColorCount; i++) {
        newState->fInfo.fColors[i] = SkColorSetA(fInfo.fColors[i],
                                                 SK_AlphaOPAQUE);
    }

    return newState;
}

/**
 * Returns true if state is a gradient and the gradient has alpha.
 */
bool SkPDFShader::State::GradientHasAlpha() const {
    if (fType == SkShader::kNone_GradientType) {
        return false;
    }

    for (int i = 0; i < fInfo.fColorCount; i++) {
        SkAlpha alpha = SkColorGetA(fInfo.fColors[i]);
        if (alpha != SK_AlphaOPAQUE) {
            return true;
        }
    }
    return false;
}

void SkPDFShader::State::AllocateGradientInfoStorage() {
    fColorData.set(sk_malloc_throw(
               fInfo.fColorCount * (sizeof(SkColor) + sizeof(SkScalar))));
    fInfo.fColors = reinterpret_cast<SkColor*>(fColorData.get());
    fInfo.fColorOffsets =
            reinterpret_cast<SkScalar*>(fInfo.fColors + fInfo.fColorCount);
}
