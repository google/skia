/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFGradientShader.h"

#include "SkOpts.h"
#include "SkPDFDocument.h"
#include "SkPDFDocumentPriv.h"
#include "SkPDFFormXObject.h"
#include "SkPDFGraphicState.h"
#include "SkPDFResourceDict.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"

static uint32_t hash(const SkShader::GradientInfo& v) {
    uint32_t buffer[] = {
        (uint32_t)v.fColorCount,
        SkOpts::hash(v.fColors, v.fColorCount * sizeof(SkColor)),
        SkOpts::hash(v.fColorOffsets, v.fColorCount * sizeof(SkScalar)),
        SkOpts::hash(v.fPoint, 2 * sizeof(SkPoint)),
        SkOpts::hash(v.fRadius, 2 * sizeof(SkScalar)),
        (uint32_t)v.fTileMode,
        v.fGradientFlags,
    };
    return SkOpts::hash(buffer, sizeof(buffer));
}

static uint32_t hash(const SkPDFGradientShader::Key& k) {
    uint32_t buffer[] = {
        (uint32_t)k.fType,
        hash(k.fInfo),
        SkOpts::hash(&k.fCanvasTransform, sizeof(SkMatrix)),
        SkOpts::hash(&k.fShaderTransform, sizeof(SkMatrix)),
        SkOpts::hash(&k.fBBox, sizeof(SkIRect))
    };
    return SkOpts::hash(buffer, sizeof(buffer));
}

static void unit_to_points_matrix(const SkPoint pts[2], SkMatrix* matrix) {
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
static void interpolate_color_code(SkScalar range, const ColorTuple& curColor,
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
static void gradient_function_code(const SkShader::GradientInfo& info,
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

        interpolate_color_code(info.fColorOffsets[i] - info.fColorOffsets[i - 1],
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

static std::unique_ptr<SkPDFDict> createInterpolationFunction(const ColorTuple& color1,
                                                    const ColorTuple& color2) {
    auto retval = SkPDFMakeDict();

    auto c0 = SkPDFMakeArray();
    c0->appendColorComponent(color1[0]);
    c0->appendColorComponent(color1[1]);
    c0->appendColorComponent(color1[2]);
    retval->insertObject("C0", std::move(c0));

    auto c1 = SkPDFMakeArray();
    c1->appendColorComponent(color2[0]);
    c1->appendColorComponent(color2[1]);
    c1->appendColorComponent(color2[2]);
    retval->insertObject("C1", std::move(c1));

    retval->insertObject("Domain", SkPDFMakeArray(0, 1));

    retval->insertInt("FunctionType", 2);
    retval->insertScalar("N", 1.0f);

    return retval;
}

static std::unique_ptr<SkPDFDict> gradientStitchCode(const SkShader::GradientInfo& info) {
    auto retval = SkPDFMakeDict();

    // normalize color stops
    int colorCount = info.fColorCount;
    std::vector<SkColor>  colors(info.fColors, info.fColors + colorCount);
    std::vector<SkScalar> colorOffsets(info.fColorOffsets, info.fColorOffsets + colorCount);

    int i = 1;
    while (i < colorCount - 1) {
        // ensure stops are in order
        if (colorOffsets[i - 1] > colorOffsets[i]) {
            colorOffsets[i] = colorOffsets[i - 1];
        }

        // remove points that are between 2 coincident points
        if ((colorOffsets[i - 1] == colorOffsets[i]) && (colorOffsets[i] == colorOffsets[i + 1])) {
            colorCount -= 1;
            colors.erase(colors.begin() + i);
            colorOffsets.erase(colorOffsets.begin() + i);
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

    auto encode = SkPDFMakeArray();
    auto bounds = SkPDFMakeArray();
    auto functions = SkPDFMakeArray();

    retval->insertObject("Domain", SkPDFMakeArray(0, 1));
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
static void apply_perspective_to_coordinates(const SkMatrix& inversePerspectiveMatrix,
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
    gradient_function_code(info, function);
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
    gradient_function_code(info, function);
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
    gradient_function_code(info, function);

    // otherwise, just write black
    function->writeText("} {0 0 0} ifelse }");
}

static void sweepCode(const SkShader::GradientInfo& info,
                          const SkMatrix& perspectiveRemover,
                          SkDynamicMemoryWStream* function) {
    function->writeText("{exch atan 360 div\n");
    tileModeCode(info.fTileMode, function);
    gradient_function_code(info, function);
    function->writeText("}");
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

static SkPDFIndirectReference make_ps_function(std::unique_ptr<SkStreamAsset> psCode,
                                               std::unique_ptr<SkPDFArray> domain,
                                               std::unique_ptr<SkPDFObject> range,
                                               SkPDFDocument* doc) {
    std::unique_ptr<SkPDFDict> dict = SkPDFMakeDict();
    dict->insertInt("FunctionType", 4);
    dict->insertObject("Domain", std::move(domain));
    dict->insertObject("Range", std::move(range));
    return SkPDFStreamOut(std::move(dict), std::move(psCode), doc);
}

static SkPDFIndirectReference make_function_shader(SkPDFDocument* doc,
                                                   const SkPDFGradientShader::Key& state) {
    SkPoint transformPoints[2];
    const SkShader::GradientInfo& info = state.fInfo;
    SkMatrix finalMatrix = state.fCanvasTransform;
    finalMatrix.preConcat(state.fShaderTransform);

    bool doStitchFunctions = (state.fType == SkShader::kLinear_GradientType ||
                              state.fType == SkShader::kRadial_GradientType ||
                              state.fType == SkShader::kConical_GradientType) &&
                             info.fTileMode == SkShader::kClamp_TileMode &&
                             !finalMatrix.hasPerspective();

    int32_t shadingType = 1;
    auto pdfShader = SkPDFMakeDict();
    // The two point radial gradient further references
    // state.fInfo
    // in translating from x, y coordinates to the t parameter. So, we have
    // to transform the points and radii according to the calculated matrix.
    if (doStitchFunctions) {
        pdfShader->insertObject("Function", gradientStitchCode(info));
        shadingType = (state.fType == SkShader::kLinear_GradientType) ? 2 : 3;

        auto extend = SkPDFMakeArray();
        extend->reserve(2);
        extend->appendBool(true);
        extend->appendBool(true);
        pdfShader->insertObject("Extend", std::move(extend));

        std::unique_ptr<SkPDFArray> coords;
        if (state.fType == SkShader::kConical_GradientType) {
            SkScalar r1 = info.fRadius[0];
            SkScalar r2 = info.fRadius[1];
            SkPoint pt1 = info.fPoint[0];
            SkPoint pt2 = info.fPoint[1];
            FixUpRadius(pt1, r1, pt2, r2);

            coords = SkPDFMakeArray(pt1.x(),
                                    pt1.y(),
                                    r1,
                                    pt2.x(),
                                    pt2.y(),
                                    r2);
        } else if (state.fType == SkShader::kRadial_GradientType) {
            const SkPoint& pt1 = info.fPoint[0];
            coords = SkPDFMakeArray(pt1.x(),
                                    pt1.y(),
                                    0,
                                    pt1.x(),
                                    pt1.y(),
                                    info.fRadius[0]);
        } else {
            const SkPoint& pt1 = info.fPoint[0];
            const SkPoint& pt2 = info.fPoint[1];
            coords = SkPDFMakeArray(pt1.x(),
                                    pt1.y(),
                                    pt2.x(),
                                    pt2.y());
        }

        pdfShader->insertObject("Coords", std::move(coords));
    } else {
        // Depending on the type of the gradient, we want to transform the
        // coordinate space in different ways.
        transformPoints[0] = info.fPoint[0];
        transformPoints[1] = info.fPoint[1];
        switch (state.fType) {
            case SkShader::kLinear_GradientType:
                break;
            case SkShader::kRadial_GradientType:
                transformPoints[1] = transformPoints[0];
                transformPoints[1].fX += info.fRadius[0];
                break;
            case SkShader::kConical_GradientType: {
                transformPoints[1] = transformPoints[0];
                transformPoints[1].fX += SK_Scalar1;
                break;
            }
            case SkShader::kSweep_GradientType:
                transformPoints[1] = transformPoints[0];
                transformPoints[1].fX += SK_Scalar1;
                break;
            case SkShader::kColor_GradientType:
            case SkShader::kNone_GradientType:
            default:
                return SkPDFIndirectReference();
        }

        // Move any scaling (assuming a unit gradient) or translation
        // (and rotation for linear gradient), of the final gradient from
        // info.fPoints to the matrix (updating bbox appropriately).  Now
        // the gradient can be drawn on on the unit segment.
        SkMatrix mapperMatrix;
        unit_to_points_matrix(transformPoints, &mapperMatrix);

        finalMatrix.preConcat(mapperMatrix);

        // Preserves as much as possible in the final matrix, and only removes
        // the perspective. The inverse of the perspective is stored in
        // perspectiveInverseOnly matrix and has 3 useful numbers
        // (p0, p1, p2), while everything else is either 0 or 1.
        // In this way the shader will handle it eficiently, with minimal code.
        SkMatrix perspectiveInverseOnly = SkMatrix::I();
        if (finalMatrix.hasPerspective()) {
            if (!split_perspective(finalMatrix,
                                   &finalMatrix, &perspectiveInverseOnly)) {
                return SkPDFIndirectReference();
            }
        }

        SkRect bbox;
        bbox.set(state.fBBox);
        if (!SkPDFUtils::InverseTransformBBox(finalMatrix, &bbox)) {
            return SkPDFIndirectReference();
        }
        SkDynamicMemoryWStream functionCode;

        SkShader::GradientInfo infoCopy = info;

        if (state.fType == SkShader::kConical_GradientType) {
            SkMatrix inverseMapperMatrix;
            if (!mapperMatrix.invert(&inverseMapperMatrix)) {
                return SkPDFIndirectReference();
            }
            inverseMapperMatrix.mapPoints(infoCopy.fPoint, 2);
            infoCopy.fRadius[0] = inverseMapperMatrix.mapRadius(info.fRadius[0]);
            infoCopy.fRadius[1] = inverseMapperMatrix.mapRadius(info.fRadius[1]);
        }
        switch (state.fType) {
            case SkShader::kLinear_GradientType:
                linearCode(infoCopy, perspectiveInverseOnly, &functionCode);
                break;
            case SkShader::kRadial_GradientType:
                radialCode(infoCopy, perspectiveInverseOnly, &functionCode);
                break;
            case SkShader::kConical_GradientType:
                twoPointConicalCode(infoCopy, perspectiveInverseOnly, &functionCode);
                break;
            case SkShader::kSweep_GradientType:
                sweepCode(infoCopy, perspectiveInverseOnly, &functionCode);
                break;
            default:
                SkASSERT(false);
        }
        pdfShader->insertObject(
                "Domain", SkPDFMakeArray(bbox.left(), bbox.right(), bbox.top(), bbox.bottom()));

        auto domain = SkPDFMakeArray(bbox.left(), bbox.right(), bbox.top(), bbox.bottom());
        std::unique_ptr<SkPDFArray> rangeObject = SkPDFMakeArray(0, 1, 0, 1, 0, 1);
        pdfShader->insertRef("Function",
                             make_ps_function(functionCode.detachAsStream(), std::move(domain),
                                              std::move(rangeObject), doc));
    }

    pdfShader->insertInt("ShadingType", shadingType);
    pdfShader->insertName("ColorSpace", "DeviceRGB");

    SkPDFDict pdfFunctionShader("Pattern");
    pdfFunctionShader.insertInt("PatternType", 2);
    pdfFunctionShader.insertObject("Matrix", SkPDFUtils::MatrixToArray(finalMatrix));
    pdfFunctionShader.insertObject("Shading", std::move(pdfShader));
    return doc->emit(pdfFunctionShader);
}

static SkPDFIndirectReference find_pdf_shader(SkPDFDocument* doc,
                                              SkPDFGradientShader::Key key,
                                              bool keyHasAlpha);

static std::unique_ptr<SkPDFDict> get_gradient_resource_dict(SkPDFIndirectReference functionShader,
                                                   SkPDFIndirectReference gState) {
    std::vector<SkPDFIndirectReference> patternShaders;
    if (functionShader != SkPDFIndirectReference()) {
        patternShaders.push_back(functionShader);
    }
    std::vector<SkPDFIndirectReference> graphicStates;
    if (gState != SkPDFIndirectReference()) {
        graphicStates.push_back(gState);
    }
    return SkPDFMakeResourceDict(std::move(graphicStates),
                                 std::move(patternShaders),
                                 std::vector<SkPDFIndirectReference>(),
                                 std::vector<SkPDFIndirectReference>());
}

// Creates a content stream which fills the pattern P0 across bounds.
// @param gsIndex A graphics state resource index to apply, or <0 if no
// graphics state to apply.
static std::unique_ptr<SkStreamAsset> create_pattern_fill_content(int gsIndex,
                                                                  int patternIndex,
                                                                  SkRect& bounds) {
    SkDynamicMemoryWStream content;
    if (gsIndex >= 0) {
        SkPDFUtils::ApplyGraphicState(gsIndex, &content);
    }
    SkPDFUtils::ApplyPattern(patternIndex, &content);
    SkPDFUtils::AppendRectangle(bounds, &content);
    SkPDFUtils::PaintPath(SkPaint::kFill_Style, SkPath::kEvenOdd_FillType, &content);
    return content.detachAsStream();
}

static bool gradient_has_alpha(const SkPDFGradientShader::Key& key) {
    SkASSERT(key.fType != SkShader::kNone_GradientType);
    for (int i = 0; i < key.fInfo.fColorCount; i++) {
        if ((SkAlpha)SkColorGetA(key.fInfo.fColors[i]) != SK_AlphaOPAQUE) {
            return true;
        }
    }
    return false;
}

// warning: does not set fHash on new key.  (Both callers need to change fields.)
static SkPDFGradientShader::Key clone_key(const SkPDFGradientShader::Key& k) {
    SkPDFGradientShader::Key clone = {
        k.fType,
        k.fInfo,  // change pointers later.
        std::unique_ptr<SkColor[]>(new SkColor[k.fInfo.fColorCount]),
        std::unique_ptr<SkScalar[]>(new SkScalar[k.fInfo.fColorCount]),
        k.fCanvasTransform,
        k.fShaderTransform,
        k.fBBox, 0};
    clone.fInfo.fColors = clone.fColors.get();
    clone.fInfo.fColorOffsets = clone.fStops.get();
    for (int i = 0; i < clone.fInfo.fColorCount; i++) {
        clone.fInfo.fColorOffsets[i] = k.fInfo.fColorOffsets[i];
        clone.fInfo.fColors[i] = k.fInfo.fColors[i];
    }
    return clone;
}

static SkPDFIndirectReference create_smask_graphic_state(SkPDFDocument* doc,
                                                     const SkPDFGradientShader::Key& state) {
    SkASSERT(state.fType != SkShader::kNone_GradientType);
    SkPDFGradientShader::Key luminosityState = clone_key(state);
    for (int i = 0; i < luminosityState.fInfo.fColorCount; i++) {
        SkAlpha alpha = SkColorGetA(luminosityState.fInfo.fColors[i]);
        luminosityState.fInfo.fColors[i] = SkColorSetARGB(255, alpha, alpha, alpha);
    }
    luminosityState.fHash = hash(luminosityState);

    SkASSERT(!gradient_has_alpha(luminosityState));
    SkPDFIndirectReference luminosityShader = find_pdf_shader(doc, std::move(luminosityState), false);
    std::unique_ptr<SkPDFDict> resources = get_gradient_resource_dict(luminosityShader,
                                                            SkPDFIndirectReference());
    SkRect bbox = SkRect::Make(state.fBBox);
    SkPDFIndirectReference alphaMask =
            SkPDFMakeFormXObject(doc,
                                 create_pattern_fill_content(-1, luminosityShader.fValue, bbox),
                                 SkPDFUtils::RectToArray(bbox),
                                 std::move(resources),
                                 SkMatrix::I(),
                                 "DeviceRGB");
    return SkPDFGraphicState::GetSMaskGraphicState(
            alphaMask, false, SkPDFGraphicState::kLuminosity_SMaskMode, doc);
}

static SkPDFIndirectReference make_alpha_function_shader(SkPDFDocument* doc,
                                                         const SkPDFGradientShader::Key& state) {
    SkASSERT(state.fType != SkShader::kNone_GradientType);
    SkPDFGradientShader::Key opaqueState = clone_key(state);
    for (int i = 0; i < opaqueState.fInfo.fColorCount; i++) {
        opaqueState.fInfo.fColors[i] = SkColorSetA(opaqueState.fInfo.fColors[i], SK_AlphaOPAQUE);
    }
    opaqueState.fHash = hash(opaqueState);

    SkASSERT(!gradient_has_alpha(opaqueState));
    SkRect bbox = SkRect::Make(state.fBBox);
    SkPDFIndirectReference colorShader = find_pdf_shader(doc, std::move(opaqueState), false);
    if (!colorShader) {
        return SkPDFIndirectReference();
    }
    // Create resource dict with alpha graphics state as G0 and
    // pattern shader as P0, then write content stream.
    SkPDFIndirectReference alphaGsRef = create_smask_graphic_state(doc, state);

    std::unique_ptr<SkPDFDict> resourceDict = get_gradient_resource_dict(colorShader, alphaGsRef);

    std::unique_ptr<SkStreamAsset> colorStream =
            create_pattern_fill_content(alphaGsRef.fValue, colorShader.fValue, bbox);
    std::unique_ptr<SkPDFDict> alphaFunctionShader = SkPDFMakeDict();
    SkPDFUtils::PopulateTilingPatternDict(alphaFunctionShader.get(), bbox,
                                 std::move(resourceDict), SkMatrix::I());
    return SkPDFStreamOut(std::move(alphaFunctionShader), std::move(colorStream), doc);
}

static SkPDFGradientShader::Key make_key(const SkShader* shader,
                                         const SkMatrix& canvasTransform,
                                         const SkIRect& bbox) {
    SkPDFGradientShader::Key key = {
         SkShader::kNone_GradientType,
         {0, nullptr, nullptr, {{0, 0}, {0, 0}}, {0, 0}, SkShader::kClamp_TileMode, 0},
         nullptr,
         nullptr,
         canvasTransform,
         SkPDFUtils::GetShaderLocalMatrix(shader),
         bbox, 0};
    key.fType = shader->asAGradient(&key.fInfo);
    SkASSERT(SkShader::kNone_GradientType != key.fType);
    SkASSERT(key.fInfo.fColorCount > 0);
    key.fColors.reset(new SkColor[key.fInfo.fColorCount]);
    key.fStops.reset(new SkScalar[key.fInfo.fColorCount]);
    key.fInfo.fColors = key.fColors.get();
    key.fInfo.fColorOffsets = key.fStops.get();
    (void)shader->asAGradient(&key.fInfo);
    key.fHash = hash(key);
    return key;
}

static SkPDFIndirectReference find_pdf_shader(SkPDFDocument* doc,
                                              SkPDFGradientShader::Key key,
                                              bool keyHasAlpha) {
    SkASSERT(gradient_has_alpha(key) == keyHasAlpha);
    auto& gradientPatternMap = doc->fGradientPatternMap;
    if (SkPDFIndirectReference* ptr = gradientPatternMap.find(key)) {
        return *ptr;
    }
    SkPDFIndirectReference pdfShader;
    if (keyHasAlpha) {
        pdfShader = make_alpha_function_shader(doc, key);
    } else {
        pdfShader = make_function_shader(doc, key);
    }
    gradientPatternMap.set(std::move(key), pdfShader);
    return pdfShader;
}

SkPDFIndirectReference SkPDFGradientShader::Make(SkPDFDocument* doc,
                                             SkShader* shader,
                                             const SkMatrix& canvasTransform,
                                             const SkIRect& bbox) {
    SkASSERT(shader);
    SkASSERT(SkShader::kNone_GradientType != shader->asAGradient(nullptr));
    SkPDFGradientShader::Key key = make_key(shader, canvasTransform, bbox);
    bool alpha = gradient_has_alpha(key);
    return find_pdf_shader(doc, std::move(key), alpha);
}
