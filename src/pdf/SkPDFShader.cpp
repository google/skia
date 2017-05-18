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
#include "SkPDFDocument.h"
#include "SkPDFFormXObject.h"
#include "SkPDFGraphicState.h"
#include "SkPDFResourceDict.h"
#include "SkPDFUtils.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTemplates.h"

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

static const int kColorComponents = 3;
typedef uint8_t ColorTuple[kColorComponents];

/* Assumes t + startOffset is on the stack and does a linear interpolation on t
   between startOffset and endOffset from prevColor to curColor (for each color
   component), leaving the result in component order on the stack. It assumes
   there are always 3 components per color.
   @param range                  endOffset - startOffset
   @param curColor[components]   The current color components.
   @param prevColor[components]  The previous color components.
   @param result                 The result ps function.
 */
static void interpolateColorCode(SkScalar range, const ColorTuple& curColor,
                                 const ColorTuple& prevColor,
                                 SkDynamicMemoryWStream* result) {
    SkASSERT(range != SkIntToScalar(0));

    // Figure out how to scale each color component.
    SkScalar multiplier[kColorComponents];
    for (int i = 0; i < kColorComponents; i++) {
        static const SkScalar kColorScale = SkScalarInvert(255);
        multiplier[i] = kColorScale * (curColor[i] - prevColor[i]) / range;
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
        result->writeText("pop ");
    }

    for (int i = 0; i < kColorComponents; i++) {
        // If the next components needs t and this component will consume a
        // copy, make another copy.
        if (dupInput[i] && multiplier[i] != 0) {
            result->writeText("dup ");
        }

        if (multiplier[i] == 0) {
            SkPDFUtils::AppendColorComponent(prevColor[i], result);
            result->writeText(" ");
        } else {
            if (multiplier[i] != 1) {
                SkPDFUtils::AppendScalar(multiplier[i], result);
                result->writeText(" mul ");
            }
            if (prevColor[i] != 0) {
                SkPDFUtils::AppendColorComponent(prevColor[i], result);
                result->writeText(" add ");
            }
        }

        if (dupInput[i]) {
            result->writeText("exch\n");
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
                                 SkDynamicMemoryWStream* result) {
    /* We want to linearly interpolate from the previous color to the next.
       Scale the colors from 0..255 to 0..1 and determine the multipliers
       for interpolation.
       C{r,g,b}(t, section) = t - offset_(section-1) + t * Multiplier{r,g,b}.
     */

    SkAutoSTMalloc<4, ColorTuple> colorDataAlloc(info.fColorCount);
    ColorTuple *colorData = colorDataAlloc.get();
    for (int i = 0; i < info.fColorCount; i++) {
        colorData[i][0] = SkColorGetR(info.fColors[i]);
        colorData[i][1] = SkColorGetG(info.fColors[i]);
        colorData[i][2] = SkColorGetB(info.fColors[i]);
    }

    // Clamp the initial color.
    result->writeText("dup 0 le {pop ");
    SkPDFUtils::AppendColorComponent(colorData[0][0], result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponent(colorData[0][1], result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponent(colorData[0][2], result);
    result->writeText(" }\n");

    // The gradient colors.
    int gradients = 0;
    for (int i = 1 ; i < info.fColorCount; i++) {
        if (info.fColorOffsets[i] == info.fColorOffsets[i - 1]) {
            continue;
        }
        gradients++;

        result->writeText("{dup ");
        SkPDFUtils::AppendScalar(info.fColorOffsets[i], result);
        result->writeText(" le {");
        if (info.fColorOffsets[i - 1] != 0) {
            SkPDFUtils::AppendScalar(info.fColorOffsets[i - 1], result);
            result->writeText(" sub\n");
        }

        interpolateColorCode(info.fColorOffsets[i] - info.fColorOffsets[i - 1],
                             colorData[i], colorData[i - 1], result);
        result->writeText("}\n");
    }

    // Clamp the final color.
    result->writeText("{pop ");
    SkPDFUtils::AppendColorComponent(colorData[info.fColorCount - 1][0], result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponent(colorData[info.fColorCount - 1][1], result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponent(colorData[info.fColorCount - 1][2], result);

    for (int i = 0 ; i < gradients + 1; i++) {
        result->writeText("} ifelse\n");
    }
}

static sk_sp<SkPDFDict> createInterpolationFunction(const ColorTuple& color1,
                                                    const ColorTuple& color2) {
    auto retval = sk_make_sp<SkPDFDict>();

    auto c0 = sk_make_sp<SkPDFArray>();
    c0->appendColorComponent(color1[0]);
    c0->appendColorComponent(color1[1]);
    c0->appendColorComponent(color1[2]);
    retval->insertObject("C0", std::move(c0));

    auto c1 = sk_make_sp<SkPDFArray>();
    c1->appendColorComponent(color2[0]);
    c1->appendColorComponent(color2[1]);
    c1->appendColorComponent(color2[2]);
    retval->insertObject("C1", std::move(c1));

    auto domain = sk_make_sp<SkPDFArray>();
    domain->appendScalar(0);
    domain->appendScalar(1.0f);
    retval->insertObject("Domain", std::move(domain));

    retval->insertInt("FunctionType", 2);
    retval->insertScalar("N", 1.0f);

    return retval;
}

static sk_sp<SkPDFDict> gradientStitchCode(const SkShader::GradientInfo& info) {
    auto retval = sk_make_sp<SkPDFDict>();

    // normalize color stops
    int colorCount = info.fColorCount;
    SkTDArray<SkColor>    colors(info.fColors, colorCount);
    SkTDArray<SkScalar>   colorOffsets(info.fColorOffsets, colorCount);

    int i = 1;
    while (i < colorCount - 1) {
        // ensure stops are in order
        if (colorOffsets[i - 1] > colorOffsets[i]) {
            colorOffsets[i] = colorOffsets[i - 1];
        }

        // remove points that are between 2 coincident points
        if ((colorOffsets[i - 1] == colorOffsets[i]) && (colorOffsets[i] == colorOffsets[i + 1])) {
            colorCount -= 1;
            colors.remove(i);
            colorOffsets.remove(i);
        } else {
            i++;
        }
    }
    // find coincident points and slightly move them over
    for (i = 1; i < colorCount - 1; i++) {
        if (colorOffsets[i - 1] == colorOffsets[i]) {
            colorOffsets[i] += 0.00001f;
        }
    }
    // check if last 2 stops coincide
    if (colorOffsets[i - 1] == colorOffsets[i]) {
        colorOffsets[i - 1] -= 0.00001f;
    }

    SkAutoSTMalloc<4, ColorTuple> colorDataAlloc(colorCount);
    ColorTuple *colorData = colorDataAlloc.get();
    for (int i = 0; i < colorCount; i++) {
        colorData[i][0] = SkColorGetR(colors[i]);
        colorData[i][1] = SkColorGetG(colors[i]);
        colorData[i][2] = SkColorGetB(colors[i]);
    }

    // no need for a stitch function if there are only 2 stops.
    if (colorCount == 2)
        return createInterpolationFunction(colorData[0], colorData[1]);

    auto encode = sk_make_sp<SkPDFArray>();
    auto bounds = sk_make_sp<SkPDFArray>();
    auto functions = sk_make_sp<SkPDFArray>();

    auto domain = sk_make_sp<SkPDFArray>();
    domain->appendScalar(0);
    domain->appendScalar(1.0f);
    retval->insertObject("Domain", std::move(domain));
    retval->insertInt("FunctionType", 3);

    for (int i = 1; i < colorCount; i++) {
        if (i > 1) {
            bounds->appendScalar(colorOffsets[i-1]);
        }

        encode->appendScalar(0);
        encode->appendScalar(1.0f);
    
        functions->appendObject(createInterpolationFunction(colorData[i-1], colorData[i]));
    }

    retval->insertObject("Encode", std::move(encode));
    retval->insertObject("Bounds", std::move(bounds));
    retval->insertObject("Functions", std::move(functions));

    return retval;
}

/* Map a value of t on the stack into [0, 1) for Repeat or Mirror tile mode. */
static void tileModeCode(SkShader::TileMode mode,
                         SkDynamicMemoryWStream* result) {
    if (mode == SkShader::kRepeat_TileMode) {
        result->writeText("dup truncate sub\n");  // Get the fractional part.
        result->writeText("dup 0 le {1 add} if\n");  // Map (-1,0) => (0,1)
        return;
    }

    if (mode == SkShader::kMirror_TileMode) {
        // Map t mod 2 into [0, 1, 1, 0].
        //               Code                     Stack
        result->writeText("abs "                 // Map negative to positive.
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
static void apply_perspective_to_coordinates(
        const SkMatrix& inversePerspectiveMatrix,
        SkDynamicMemoryWStream* code) {
    if (!inversePerspectiveMatrix.hasPerspective()) {
        return;
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
    code->writeText(" dup ");             // x y y
    SkPDFUtils::AppendScalar(p1, code);   // x y y p1
    code->writeText(" mul "               // x y y*p1
                    " 2 index ");         // x y y*p1 x
    SkPDFUtils::AppendScalar(p0, code);   // x y y p1 x p0
    code->writeText(" mul ");             // x y y*p1 x*p0
    SkPDFUtils::AppendScalar(p2, code);   // x y y p1 x*p0 p2
    code->writeText(" add "               // x y y*p1 x*p0+p2
                    "add "                // x y y*p1+x*p0+p2
                    "3 1 roll "           // y*p1+x*p0+p2 x y
                    "2 index "            // z x y y*p1+x*p0+p2
                    "div "                // y*p1+x*p0+p2 x y/(y*p1+x*p0+p2)
                    "3 1 roll "           // y/(y*p1+x*p0+p2) y*p1+x*p0+p2 x
                    "exch "               // y/(y*p1+x*p0+p2) x y*p1+x*p0+p2
                    "div "                // y/(y*p1+x*p0+p2) x/(y*p1+x*p0+p2)
                    "exch\n");            // x/(y*p1+x*p0+p2) y/(y*p1+x*p0+p2)
}

static void linearCode(const SkShader::GradientInfo& info,
                       const SkMatrix& perspectiveRemover,
                       SkDynamicMemoryWStream* function) {
    function->writeText("{");

    apply_perspective_to_coordinates(perspectiveRemover, function);

    function->writeText("pop\n");  // Just ditch the y value.
    tileModeCode(info.fTileMode, function);
    gradientFunctionCode(info, function);
    function->writeText("}");
}

static void radialCode(const SkShader::GradientInfo& info,
                       const SkMatrix& perspectiveRemover,
                       SkDynamicMemoryWStream* function) {
    function->writeText("{");

    apply_perspective_to_coordinates(perspectiveRemover, function);

    // Find the distance from the origin.
    function->writeText("dup "      // x y y
                    "mul "      // x y^2
                    "exch "     // y^2 x
                    "dup "      // y^2 x x
                    "mul "      // y^2 x^2
                    "add "      // y^2+x^2
                    "sqrt\n");  // sqrt(y^2+x^2)

    tileModeCode(info.fTileMode, function);
    gradientFunctionCode(info, function);
    function->writeText("}");
}

/* Conical gradient shader, based on the Canvas spec for radial gradients
   See: http://www.w3.org/TR/2dcontext/#dom-context-2d-createradialgradient
 */
static void twoPointConicalCode(const SkShader::GradientInfo& info,
                                const SkMatrix& perspectiveRemover,
                                SkDynamicMemoryWStream* function) {
    SkScalar dx = info.fPoint[1].fX - info.fPoint[0].fX;
    SkScalar dy = info.fPoint[1].fY - info.fPoint[0].fY;
    SkScalar r0 = info.fRadius[0];
    SkScalar dr = info.fRadius[1] - info.fRadius[0];
    SkScalar a = dx * dx + dy * dy - dr * dr;

    // First compute t, if the pixel falls outside the cone, then we'll end
    // with 'false' on the stack, otherwise we'll push 'true' with t below it

    // We start with a stack of (x y), copy it and then consume one copy in
    // order to calculate b and the other to calculate c.
    function->writeText("{");

    apply_perspective_to_coordinates(perspectiveRemover, function);

    function->writeText("2 copy ");

    // Calculate b and b^2; b = -2 * (y * dy + x * dx + r0 * dr).
    SkPDFUtils::AppendScalar(dy, function);
    function->writeText(" mul exch ");
    SkPDFUtils::AppendScalar(dx, function);
    function->writeText(" mul add ");
    SkPDFUtils::AppendScalar(r0 * dr, function);
    function->writeText(" add -2 mul dup dup mul\n");

    // c = x^2 + y^2 + radius0^2
    function->writeText("4 2 roll dup mul exch dup mul add ");
    SkPDFUtils::AppendScalar(r0 * r0, function);
    function->writeText(" sub dup 4 1 roll\n");

    // Contents of the stack at this point: c, b, b^2, c

    // if a = 0, then we collapse to a simpler linear case
    if (a == 0) {

        // t = -c/b
        function->writeText("pop pop div neg dup ");

        // compute radius(t)
        SkPDFUtils::AppendScalar(dr, function);
        function->writeText(" mul ");
        SkPDFUtils::AppendScalar(r0, function);
        function->writeText(" add\n");

        // if r(t) < 0, then it's outside the cone
        function->writeText("0 lt {pop false} {true} ifelse\n");

    } else {

        // quadratic case: the Canvas spec wants the largest
        // root t for which radius(t) > 0

        // compute the discriminant (b^2 - 4ac)
        SkPDFUtils::AppendScalar(a * 4, function);
        function->writeText(" mul sub dup\n");

        // if d >= 0, proceed
        function->writeText("0 ge {\n");

        // an intermediate value we'll use to compute the roots:
        // q = -0.5 * (b +/- sqrt(d))
        function->writeText("sqrt exch dup 0 lt {exch -1 mul} if");
        function->writeText(" add -0.5 mul dup\n");

        // first root = q / a
        SkPDFUtils::AppendScalar(a, function);
        function->writeText(" div\n");

        // second root = c / q
        function->writeText("3 1 roll div\n");

        // put the larger root on top of the stack
        function->writeText("2 copy gt {exch} if\n");

        // compute radius(t) for larger root
        function->writeText("dup ");
        SkPDFUtils::AppendScalar(dr, function);
        function->writeText(" mul ");
        SkPDFUtils::AppendScalar(r0, function);
        function->writeText(" add\n");

        // if r(t) > 0, we have our t, pop off the smaller root and we're done
        function->writeText(" 0 gt {exch pop true}\n");

        // otherwise, throw out the larger one and try the smaller root
        function->writeText("{pop dup\n");
        SkPDFUtils::AppendScalar(dr, function);
        function->writeText(" mul ");
        SkPDFUtils::AppendScalar(r0, function);
        function->writeText(" add\n");

        // if r(t) < 0, push false, otherwise the smaller root is our t
        function->writeText("0 le {pop false} {true} ifelse\n");
        function->writeText("} ifelse\n");

        // d < 0, clear the stack and push false
        function->writeText("} {pop pop pop false} ifelse\n");
    }

    // if the pixel is in the cone, proceed to compute a color
    function->writeText("{");
    tileModeCode(info.fTileMode, function);
    gradientFunctionCode(info, function);

    // otherwise, just write black
    function->writeText("} {0 0 0} ifelse }");
}

static void sweepCode(const SkShader::GradientInfo& info,
                          const SkMatrix& perspectiveRemover,
                          SkDynamicMemoryWStream* function) {
    function->writeText("{exch atan 360 div\n");
    tileModeCode(info.fTileMode, function);
    gradientFunctionCode(info, function);
    function->writeText("}");
}

static void drawBitmapMatrix(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& matrix) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(matrix);
    canvas->drawBitmap(bm, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkPDFStream> make_alpha_function_shader(SkPDFDocument* doc,
                                                     SkScalar dpi,
                                                     const SkPDFShader::State& state);
static sk_sp<SkPDFDict> make_function_shader(SkPDFCanon* canon,
                                             const SkPDFShader::State& state);

static sk_sp<SkPDFStream> make_image_shader(SkPDFDocument* doc,
                                            SkScalar dpi,
                                            const SkPDFShader::State& state,
                                            SkBitmap image);

static sk_sp<SkPDFObject> get_pdf_shader_by_state(
        SkPDFDocument* doc,
        SkScalar dpi,
        SkPDFShader::State state,
        SkBitmap image) {
    SkPDFCanon* canon = doc->canon();
    if (state.fType == SkShader::kNone_GradientType && image.isNull()) {
        // TODO(vandebo) This drops SKComposeShader on the floor.  We could
        // handle compose shader by pulling things up to a layer, drawing with
        // the first shader, applying the xfer mode and drawing again with the
        // second shader, then applying the layer to the original drawing.
        return nullptr;
    } else if (state.fType == SkShader::kNone_GradientType) {
        sk_sp<SkPDFObject> shader = canon->findImageShader(state);
        if (!shader) {
            shader = make_image_shader(doc, dpi, state, std::move(image));
            canon->addImageShader(shader, std::move(state));
        }
        return shader;
    } else if (state.GradientHasAlpha()) {
        sk_sp<SkPDFObject> shader = canon->findAlphaShader(state);
        if (!shader) {
            shader = make_alpha_function_shader(doc, dpi, state);
            canon->addAlphaShader(shader, std::move(state));
        }
        return shader;
    } else {
        sk_sp<SkPDFObject> shader = canon->findFunctionShader(state);
        if (!shader) {
            shader = make_function_shader(canon, state);
            canon->addFunctionShader(shader, std::move(state));
        }
        return shader;
    }
}

sk_sp<SkPDFObject> SkPDFShader::GetPDFShader(SkPDFDocument* doc,
                                             SkScalar dpi,
                                             SkShader* shader,
                                             const SkMatrix& matrix,
                                             const SkIRect& surfaceBBox,
                                             SkScalar rasterScale) {
    if (surfaceBBox.isEmpty()) {
        return nullptr;
    }
    SkBitmap image;
    State state(shader, matrix, surfaceBBox, rasterScale, &image);
    return get_pdf_shader_by_state(
            doc, dpi, std::move(state), std::move(image));
}

static sk_sp<SkPDFDict> get_gradient_resource_dict(
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
    return SkPDFResourceDict::Make(&graphicStates, &patterns, nullptr, nullptr);
}

static void populate_tiling_pattern_dict(SkPDFDict* pattern,
                                         SkRect& bbox,
                                         sk_sp<SkPDFDict> resources,
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
    pattern->insertObject("Resources", std::move(resources));
    if (!matrix.isIdentity()) {
        pattern->insertObject("Matrix", SkPDFUtils::MatrixToArray(matrix));
    }
}

/**
 * Creates a content stream which fills the pattern P0 across bounds.
 * @param gsIndex A graphics state resource index to apply, or <0 if no
 * graphics state to apply.
 */
static std::unique_ptr<SkStreamAsset> create_pattern_fill_content(
        int gsIndex, SkRect& bounds) {
    SkDynamicMemoryWStream content;
    if (gsIndex >= 0) {
        SkPDFUtils::ApplyGraphicState(gsIndex, &content);
    }
    SkPDFUtils::ApplyPattern(0, &content);
    SkPDFUtils::AppendRectangle(bounds, &content);
    SkPDFUtils::PaintPath(SkPaint::kFill_Style, SkPath::kEvenOdd_FillType,
                          &content);

    return std::unique_ptr<SkStreamAsset>(content.detachAsStream());
}

/**
 * Creates a ExtGState with the SMask set to the luminosityShader in
 * luminosity mode. The shader pattern extends to the bbox.
 */
static sk_sp<SkPDFObject> create_smask_graphic_state(
        SkPDFDocument* doc, SkScalar dpi, const SkPDFShader::State& state) {
    SkRect bbox;
    bbox.set(state.fBBox);

    sk_sp<SkPDFObject> luminosityShader(
            get_pdf_shader_by_state(doc, dpi, state.MakeAlphaToLuminosityState(),
                                    SkBitmap()));

    std::unique_ptr<SkStreamAsset> alphaStream(create_pattern_fill_content(-1, bbox));

    sk_sp<SkPDFDict> resources =
        get_gradient_resource_dict(luminosityShader.get(), nullptr);

    sk_sp<SkPDFObject> alphaMask =
        SkPDFMakeFormXObject(std::move(alphaStream),
                             SkPDFUtils::RectToArray(bbox),
                             std::move(resources),
                             SkMatrix::I(),
                             "DeviceRGB");
    return SkPDFGraphicState::GetSMaskGraphicState(
            std::move(alphaMask), false,
            SkPDFGraphicState::kLuminosity_SMaskMode, doc->canon());
}

static sk_sp<SkPDFStream> make_alpha_function_shader(SkPDFDocument* doc,
                                                     SkScalar dpi,
                                                     const SkPDFShader::State& state) {
    SkRect bbox;
    bbox.set(state.fBBox);

    SkPDFShader::State opaqueState(state.MakeOpaqueState());

    sk_sp<SkPDFObject> colorShader(
            get_pdf_shader_by_state(doc, dpi, std::move(opaqueState), SkBitmap()));
    if (!colorShader) {
        return nullptr;
    }

    // Create resource dict with alpha graphics state as G0 and
    // pattern shader as P0, then write content stream.
    sk_sp<SkPDFObject> alphaGs = create_smask_graphic_state(doc, dpi, state);

    sk_sp<SkPDFDict> resourceDict =
            get_gradient_resource_dict(colorShader.get(), alphaGs.get());

    std::unique_ptr<SkStreamAsset> colorStream(
            create_pattern_fill_content(0, bbox));
    auto alphaFunctionShader = sk_make_sp<SkPDFStream>(std::move(colorStream));

    populate_tiling_pattern_dict(alphaFunctionShader->dict(), bbox,
                                 std::move(resourceDict), SkMatrix::I()); 
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

static sk_sp<SkPDFArray> make_range_object() {
    auto range = sk_make_sp<SkPDFArray>();
    range->reserve(6);
    range->appendInt(0);
    range->appendInt(1);
    range->appendInt(0);
    range->appendInt(1);
    range->appendInt(0);
    range->appendInt(1);
    return range;
}

static sk_sp<SkPDFStream> make_ps_function(
        std::unique_ptr<SkStreamAsset> psCode,
        sk_sp<SkPDFArray> domain,
        sk_sp<SkPDFObject> range) {
    auto result = sk_make_sp<SkPDFStream>(std::move(psCode));
    result->dict()->insertInt("FunctionType", 4);
    result->dict()->insertObject("Domain", std::move(domain));
    result->dict()->insertObject("Range", std::move(range));
    return result;
}

// catch cases where the inner just touches the outer circle
// and make the inner circle just inside the outer one to match raster
static void FixUpRadius(const SkPoint& p1, SkScalar& r1, const SkPoint& p2, SkScalar& r2) {
    // detect touching circles
    SkScalar distance = SkPoint::Distance(p1, p2);
    SkScalar subtractRadii = fabs(r1 - r2);
    if (fabs(distance - subtractRadii) < 0.002f) {
        if (r1 > r2) {
            r1 += 0.002f;
        } else {
            r2 += 0.002f;
        }
    }
}

static sk_sp<SkPDFDict> make_function_shader(SkPDFCanon* canon,
                                             const SkPDFShader::State& state) {
    void (*codeFunction)(const SkShader::GradientInfo& info,
                         const SkMatrix& perspectiveRemover,
                         SkDynamicMemoryWStream* function) = nullptr;
    SkPoint transformPoints[2];
    const SkShader::GradientInfo* info = &state.fInfo;
    SkMatrix finalMatrix = state.fCanvasTransform;
    finalMatrix.preConcat(state.fShaderTransform);

    bool doStitchFunctions = (state.fType == SkShader::kLinear_GradientType ||
                              state.fType == SkShader::kRadial_GradientType ||
                              state.fType == SkShader::kConical_GradientType) &&
                             info->fTileMode == SkShader::kClamp_TileMode &&
                             !finalMatrix.hasPerspective();

    auto domain = sk_make_sp<SkPDFArray>();

    int32_t shadingType = 1;
    auto pdfShader = sk_make_sp<SkPDFDict>();
    // The two point radial gradient further references
    // state.fInfo
    // in translating from x, y coordinates to the t parameter. So, we have
    // to transform the points and radii according to the calculated matrix.
    if (doStitchFunctions) {
        pdfShader->insertObject("Function", gradientStitchCode(*info));
        shadingType = (state.fType == SkShader::kLinear_GradientType) ? 2 : 3;

        auto extend = sk_make_sp<SkPDFArray>();
        extend->reserve(2);
        extend->appendBool(true);
        extend->appendBool(true);
        pdfShader->insertObject("Extend", std::move(extend));

        auto coords = sk_make_sp<SkPDFArray>();
        if (state.fType == SkShader::kConical_GradientType) {
            coords->reserve(6);
            SkScalar r1 = info->fRadius[0];
            SkScalar r2 = info->fRadius[1];
            SkPoint pt1 = info->fPoint[0];
            SkPoint pt2 = info->fPoint[1];
            FixUpRadius(pt1, r1, pt2, r2);

            coords->appendScalar(pt1.fX);
            coords->appendScalar(pt1.fY);
            coords->appendScalar(r1);

            coords->appendScalar(pt2.fX);
            coords->appendScalar(pt2.fY);
            coords->appendScalar(r2);
        } else if (state.fType == SkShader::kRadial_GradientType) {
            coords->reserve(6);
            const SkPoint& pt1 = info->fPoint[0];

            coords->appendScalar(pt1.fX);
            coords->appendScalar(pt1.fY);
            coords->appendScalar(0);

            coords->appendScalar(pt1.fX);
            coords->appendScalar(pt1.fY);
            coords->appendScalar(info->fRadius[0]);
        } else {
            coords->reserve(4);
            const SkPoint& pt1 = info->fPoint[0];
            const SkPoint& pt2 = info->fPoint[1];

            coords->appendScalar(pt1.fX);
            coords->appendScalar(pt1.fY);

            coords->appendScalar(pt2.fX);
            coords->appendScalar(pt2.fY);
        }

        pdfShader->insertObject("Coords", std::move(coords));
    } else {
        // Depending on the type of the gradient, we want to transform the
        // coordinate space in different ways.
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
                return nullptr;
        }

        // Move any scaling (assuming a unit gradient) or translation
        // (and rotation for linear gradient), of the final gradient from
        // info->fPoints to the matrix (updating bbox appropriately).  Now
        // the gradient can be drawn on on the unit segment.
        SkMatrix mapperMatrix;
        unitToPointsMatrix(transformPoints, &mapperMatrix);

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
                return nullptr;
            }
        }

        SkRect bbox;
        bbox.set(state.fBBox);
        if (!inverse_transform_bbox(finalMatrix, &bbox)) {
            return nullptr;
        }
        domain->reserve(4);
        domain->appendScalar(bbox.fLeft);
        domain->appendScalar(bbox.fRight);
        domain->appendScalar(bbox.fTop);
        domain->appendScalar(bbox.fBottom);
        
        SkDynamicMemoryWStream functionCode;
        
        if (state.fType == SkShader::kConical_GradientType) {
            SkShader::GradientInfo twoPointRadialInfo = *info;
            SkMatrix inverseMapperMatrix;
            if (!mapperMatrix.invert(&inverseMapperMatrix)) {
                return nullptr;
            }
            inverseMapperMatrix.mapPoints(twoPointRadialInfo.fPoint, 2);
            twoPointRadialInfo.fRadius[0] =
                inverseMapperMatrix.mapRadius(info->fRadius[0]);
            twoPointRadialInfo.fRadius[1] =
                inverseMapperMatrix.mapRadius(info->fRadius[1]);
            codeFunction(twoPointRadialInfo, perspectiveInverseOnly, &functionCode);
        } else {
            codeFunction(*info, perspectiveInverseOnly, &functionCode);
        }
        
        pdfShader->insertObject("Domain", domain);

        std::unique_ptr<SkStreamAsset> functionStream(functionCode.detachAsStream());
        sk_sp<SkPDFArray> rangeObject =
                SkPDFUtils::GetCachedT(&canon->fRangeObject, &make_range_object);
        sk_sp<SkPDFStream> function = make_ps_function(std::move(functionStream), std::move(domain),
                                                       std::move(rangeObject));
        pdfShader->insertObjRef("Function", std::move(function));
    }

    pdfShader->insertInt("ShadingType", shadingType);
    pdfShader->insertName("ColorSpace", "DeviceRGB");

    auto pdfFunctionShader = sk_make_sp<SkPDFDict>("Pattern");
    pdfFunctionShader->insertInt("PatternType", 2);
    pdfFunctionShader->insertObject("Matrix",
                                    SkPDFUtils::MatrixToArray(finalMatrix));
    pdfFunctionShader->insertObject("Shading", std::move(pdfShader));

    return pdfFunctionShader;
}

static sk_sp<SkPDFStream> make_image_shader(SkPDFDocument* doc,
                                            SkScalar dpi,
                                            const SkPDFShader::State& state,
                                            SkBitmap image) {
    SkASSERT(state.fBitmapKey ==
             (SkBitmapKey{image.getSubset(), image.getGenerationID()}));

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
        return nullptr;
    }

    SkRect bitmapBounds;
    image.getBounds(&bitmapBounds);

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
    sk_sp<SkPDFDevice> patternDevice(
            SkPDFDevice::CreateUnflipped(size, dpi, doc));
    SkCanvas canvas(patternDevice.get());

    SkRect patternBBox;
    image.getBounds(&patternBBox);

    // Translate the canvas so that the bitmap origin is at (0, 0).
    canvas.translate(-deviceBounds.left(), -deviceBounds.top());
    patternBBox.offset(-deviceBounds.left(), -deviceBounds.top());
    // Undo the translation in the final matrix
    finalMatrix.preTranslate(deviceBounds.left(), deviceBounds.top());

    // If the bitmap is out of bounds (i.e. clamp mode where we only see the
    // stretched sides), canvas will clip this out and the extraneous data
    // won't be saved to the PDF.
    canvas.drawBitmap(image, 0, 0);

    SkScalar width = SkIntToScalar(image.width());
    SkScalar height = SkIntToScalar(image.height());

    // Tiling is implied.  First we handle mirroring.
    if (tileModes[0] == SkShader::kMirror_TileMode) {
        SkMatrix xMirror;
        xMirror.setScale(-1, 1);
        xMirror.postTranslate(2 * width, 0);
        drawBitmapMatrix(&canvas, image, xMirror);
        patternBBox.fRight += width;
    }
    if (tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix yMirror;
        yMirror.setScale(SK_Scalar1, -SK_Scalar1);
        yMirror.postTranslate(0, 2 * height);
        drawBitmapMatrix(&canvas, image, yMirror);
        patternBBox.fBottom += height;
    }
    if (tileModes[0] == SkShader::kMirror_TileMode &&
            tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix mirror;
        mirror.setScale(-1, -1);
        mirror.postTranslate(2 * width, 2 * height);
        drawBitmapMatrix(&canvas, image, mirror);
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
            paint.setColor(image.getColor(0, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, deviceBounds.top(),
                                deviceBounds.right(), 0);
        if (!rect.isEmpty()) {
            paint.setColor(image.getColor(image.width() - 1, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, height,
                                deviceBounds.right(), deviceBounds.bottom());
        if (!rect.isEmpty()) {
            paint.setColor(image.getColor(image.width() - 1,
                                           image.height() - 1));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(deviceBounds.left(), height,
                                0, deviceBounds.bottom());
        if (!rect.isEmpty()) {
            paint.setColor(image.getColor(0, image.height() - 1));
            canvas.drawRect(rect, paint);
        }
    }

    // Then expand the left, right, top, then bottom.
    if (tileModes[0] == SkShader::kClamp_TileMode) {
        SkIRect subset = SkIRect::MakeXYWH(0, 0, 1, image.height());
        if (deviceBounds.left() < 0) {
            SkBitmap left;
            SkAssertResult(image.extractSubset(&left, subset));

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
            subset.offset(image.width() - 1, 0);
            SkAssertResult(image.extractSubset(&right, subset));

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
        SkIRect subset = SkIRect::MakeXYWH(0, 0, image.width(), 1);
        if (deviceBounds.top() < 0) {
            SkBitmap top;
            SkAssertResult(image.extractSubset(&top, subset));

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
            subset.offset(0, image.height() - 1);
            SkAssertResult(image.extractSubset(&bottom, subset));

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

    auto imageShader = sk_make_sp<SkPDFStream>(patternDevice->content());
    populate_tiling_pattern_dict(imageShader->dict(), patternBBox,
                                 patternDevice->makeResourceDict(), finalMatrix);
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
        if (fBitmapKey != b.fBitmapKey ||
                fBitmapKey.fID == 0 ||
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

SkPDFShader::State::State(SkShader* shader, const SkMatrix& canvasTransform,
                          const SkIRect& bbox, SkScalar rasterScale,
                          SkBitmap* imageDst)
        : fType(SkShader::kNone_GradientType)
        , fInfo{0, nullptr, nullptr, {{0.0f, 0.0f}, {0.0f, 0.0f}},
                {0.0f, 0.0f}, SkShader::kClamp_TileMode, 0}
        , fCanvasTransform(canvasTransform)
        , fShaderTransform{SkMatrix::I()}
        , fBBox(bbox)
        , fBitmapKey{{0, 0, 0, 0}, 0}
        , fImageTileModes{SkShader::kClamp_TileMode,
                          SkShader::kClamp_TileMode} {
    SkASSERT(imageDst);
    fInfo.fColorCount = 0;
    fInfo.fColors = nullptr;
    fInfo.fColorOffsets = nullptr;
    fImageTileModes[0] = fImageTileModes[1] = SkShader::kClamp_TileMode;
    fType = shader->asAGradient(&fInfo);

    if (fType != SkShader::kNone_GradientType) {
        fBitmapKey = SkBitmapKey{{0, 0, 0, 0}, 0};
        fShaderTransform = shader->getLocalMatrix();
        this->allocateGradientInfoStorage();
        shader->asAGradient(&fInfo);
        return;
    }
    if (SkImage* skimg = shader->isAImage(&fShaderTransform, fImageTileModes)) {
        // TODO(halcanary): delay converting to bitmap.
        if (skimg->asLegacyBitmap(imageDst, SkImage::kRO_LegacyBitmapMode)) {
            fBitmapKey = SkBitmapKey{imageDst->getSubset(), imageDst->getGenerationID()};
            return;
        }
    }
    fShaderTransform = shader->getLocalMatrix();
    // Generic fallback for unsupported shaders:
    //  * allocate a bbox-sized bitmap
    //  * shade the whole area
    //  * use the result as a bitmap shader

    // bbox is in device space. While that's exactly what we
    // want for sizing our bitmap, we need to map it into
    // shader space for adjustments (to match
    // MakeImageShader's behavior).
    SkRect shaderRect = SkRect::Make(bbox);
    if (!inverse_transform_bbox(canvasTransform, &shaderRect)) {
        imageDst->reset();
        return;
    }

    // Clamp the bitmap size to about 1M pixels
    static const SkScalar kMaxBitmapArea = 1024 * 1024;
    SkScalar bitmapArea = rasterScale * bbox.width() * rasterScale * bbox.height();
    if (bitmapArea > kMaxBitmapArea) {
        rasterScale *= SkScalarSqrt(kMaxBitmapArea / bitmapArea);
    }

    SkISize size = {SkScalarRoundToInt(rasterScale * bbox.width()),
                    SkScalarRoundToInt(rasterScale * bbox.height())};
    SkSize scale = {SkIntToScalar(size.width()) / shaderRect.width(),
                    SkIntToScalar(size.height()) / shaderRect.height()};

    imageDst->allocN32Pixels(size.width(), size.height());
    imageDst->eraseColor(SK_ColorTRANSPARENT);

    SkPaint p;
    p.setShader(sk_ref_sp(shader));

    SkCanvas canvas(*imageDst);
    canvas.scale(scale.width(), scale.height());
    canvas.translate(-shaderRect.x(), -shaderRect.y());
    canvas.drawPaint(p);

    fShaderTransform.setTranslate(shaderRect.x(), shaderRect.y());
    fShaderTransform.preScale(1 / scale.width(), 1 / scale.height());
    fBitmapKey = SkBitmapKey{imageDst->getSubset(), imageDst->getGenerationID()};
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

        this->allocateGradientInfoStorage();
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
SkPDFShader::State SkPDFShader::State::MakeAlphaToLuminosityState() const {
    SkASSERT(fBitmapKey == (SkBitmapKey{{0, 0, 0, 0}, 0}));
    SkASSERT(fType != SkShader::kNone_GradientType);

    SkPDFShader::State newState(*this);

    for (int i = 0; i < fInfo.fColorCount; i++) {
        SkAlpha alpha = SkColorGetA(fInfo.fColors[i]);
        newState.fInfo.fColors[i] = SkColorSetARGB(255, alpha, alpha, alpha);
    }

    return newState;
}

/**
 * Create a copy of this gradient state with alpha set to fully opaque
 * Only valid for gradient states.
 */
SkPDFShader::State SkPDFShader::State::MakeOpaqueState() const {
    SkASSERT(fBitmapKey == (SkBitmapKey{{0, 0, 0, 0}, 0}));
    SkASSERT(fType != SkShader::kNone_GradientType);

    SkPDFShader::State newState(*this);
    for (int i = 0; i < fInfo.fColorCount; i++) {
        newState.fInfo.fColors[i] = SkColorSetA(fInfo.fColors[i],
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

void SkPDFShader::State::allocateGradientInfoStorage() {
    fColors.reset(new SkColor[fInfo.fColorCount]);
    fStops.reset(new SkScalar[fInfo.fColorCount]);
    fInfo.fColors = fColors.get();
    fInfo.fColorOffsets = fStops.get();
}
