/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFGradientShader.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFFormXObject.h"
#include "src/pdf/SkPDFGraphicState.h"
#include "src/pdf/SkPDFResourceDict.h"
#include "src/pdf/SkPDFTypes.h"
#include "src/pdf/SkPDFUtils.h"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

using namespace skia_private;

static uint32_t hash(const SkShaderBase::GradientInfo& v) {
    uint32_t buffer[] = {
        (uint32_t)v.fColorCount,
        SkChecksum::Hash32(v.fColors, v.fColorCount * sizeof(SkColor)),
        SkChecksum::Hash32(v.fColorOffsets, v.fColorCount * sizeof(SkScalar)),
        SkChecksum::Hash32(v.fPoint, 2 * sizeof(SkPoint)),
        SkChecksum::Hash32(v.fRadius, 2 * sizeof(SkScalar)),
        (uint32_t)v.fTileMode,
        v.fGradientFlags,
    };
    return SkChecksum::Hash32(buffer, sizeof(buffer));
}

static uint32_t hash(const SkPDFGradientShader::Key& k) {
    uint32_t buffer[] = {
        (uint32_t)k.fType,
        hash(k.fInfo),
        SkChecksum::Hash32(&k.fCanvasTransform, sizeof(SkMatrix)),
        SkChecksum::Hash32(&k.fShaderTransform, sizeof(SkMatrix)),
        SkChecksum::Hash32(&k.fBBox, sizeof(SkIRect))
    };
    return SkChecksum::Hash32(buffer, sizeof(buffer));
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

static bool is_premul(const SkShaderBase::GradientInfo& info) {
    return SkToBool(info.fGradientFlags & SkGradientShader::kInterpolateColorsInPremul_Flag);
}

enum NumComponents {
    Three = 3,
    Four = 4,
    Max = Four,
};

/* Assumes t - startOffset is on the stack and does a linear interpolation on t
   between startOffset and endOffset from prevColor to curColor (for each color
   component), leaving the result in component order on the stack.
   @param range         endOffset - startOffset
   @param beginColor    The previous color.
   @param endColor      The current color.
   @param numComponents The number of components (3 or 4 if alpha is needed).
   @param result        The result ps function.
 */
static void interpolate_color_code(SkScalar range, NumComponents numComponents,
                                   SkColor4f prevColor, SkColor4f curColor,
                                   SkDynamicMemoryWStream* result) {
    SkASSERT(range != SkIntToScalar(0));

    /* Linearly interpolate from the previous color to the current.
       Take the components 0..1 and determine the multipliers for interpolation.
       C{r,g,b}(t, section) = t - offset_(section-1) + t * Multiplier{r,g,b}.
     */

    // Figure out how to scale each color component.
    SkScalar multiplier[NumComponents::Max];
    for (int i = 0; i < numComponents; i++) {
        multiplier[i] = (curColor[i] - prevColor[i]) / range;
    }

    // Calculate when we no longer need to keep a copy of the input parameter t.
    // If the last component to use t is i, then dupInput[0..i - 1] = true
    // and dupInput[i .. components] = false.
    bool dupInput[NumComponents::Max];
    dupInput[numComponents - 1] = false;
    for (int i = numComponents - 2; i >= 0; i--) {
        dupInput[i] = dupInput[i + 1] || multiplier[i + 1] != 0;
    }

    if (!dupInput[0] && multiplier[0] == 0) {
        result->writeText("pop ");
    }

    for (int i = 0; i < numComponents; i++) {
        // If the next components needs t and this component will consume a
        // copy, make another copy.
        if (dupInput[i] && multiplier[i] != 0) {
            result->writeText("dup ");
        }

        if (multiplier[i] == 0) {
            SkPDFUtils::AppendColorComponentF(prevColor[i], result);
            result->writeText(" ");
        } else {
            if (multiplier[i] != 1) {
                SkPDFUtils::AppendScalar(multiplier[i], result);
                result->writeText(" mul ");
            }
            if (prevColor[i] != 0) {
                SkPDFUtils::AppendColorComponentF(prevColor[i], result);
                result->writeText(" add ");
            }
        }

        if (dupInput[i]) {
            result->writeText("exch ");
        }
    }
}

// Convert { r, g, b, a } to a == 0 ? {0 0 0} : { r/a, g/a, b/a }
static void unpremul(const SkShaderBase::GradientInfo& info, SkDynamicMemoryWStream* function) {
    // Preview Version 11.0 (1069.7.1) aborts the function if the predicate is like
    // "dup 0 eq" or any other use of "eq" with "a".
    function->writeText("dup abs 0.00001 lt"
                        "{ pop pop pop pop 0 0 0 }"
                        "{"
                        " dup"         // r g b a a
                        " 3 1 roll"    // r g a b a
                        " div"         // r g a b/a
                        " 4 1 roll"    // b/a r g a
                        " dup"         // b/a r g a a
                        " 3 1 roll"    // b/a r a g a
                        " div"         // b/a r a g/a
                        " 4 1 roll"    // g/a b/a r a
                        " div"         // g/a b/a r/a
                        " 3 1 roll"    // r/a g/a b/a
                        "} ifelse\n");
}

static void write_gradient_ranges(const SkShaderBase::GradientInfo& info, SkSpan<size_t> rangeEnds,
                                  NumComponents numComponents,
                                  bool top, bool first, SkDynamicMemoryWStream* result) {
    SkASSERT(!rangeEnds.empty());

    size_t rangeEndIndex = rangeEnds[rangeEnds.size() - 1];
    SkScalar rangeEnd = info.fColorOffsets[rangeEndIndex];

    // Each range check tests 0 < t <= end.
    if (top) {
        SkASSERT(first);
        // t may have been set to 0 to signal that the answer has already been found.
        result->writeText("dup dup 0 gt exch ");  // In Preview 11.0 (1033.3) `0. 0 ne` is true.
        SkPDFUtils::AppendScalar(rangeEnd, result);
        result->writeText(" le and {\n");
    } else if (first) {
        // After the top level check, only t <= end needs to be tested on if (lo) side.
        result->writeText("dup ");
        SkPDFUtils::AppendScalar(rangeEnd, result);
        result->writeText(" le {\n");
    } else {
        // The else (hi) side.
        result->writeText("{\n");
    }

    if (rangeEnds.size() == 1) {
        // Set the stack to [r g b].
        size_t rangeBeginIndex = rangeEndIndex - 1;
        SkScalar rangeBegin = info.fColorOffsets[rangeBeginIndex];
        SkPDFUtils::AppendScalar(rangeBegin, result);
        result->writeText(" sub ");  // consume t, put t - startOffset on the stack.
        interpolate_color_code(rangeEnd - rangeBegin, numComponents,
                               info.fColors[rangeBeginIndex], info.fColors[rangeEndIndex], result);
        result->writeText("\n");
    } else {
        size_t loCount = rangeEnds.size() / 2;
        SkSpan<size_t> loSpan = rangeEnds.subspan(0, loCount);
        write_gradient_ranges(info, loSpan, numComponents, false, true, result);

        SkSpan<size_t> hiSpan = rangeEnds.subspan(loCount, rangeEnds.size() - loCount);
        write_gradient_ranges(info, hiSpan, numComponents, false, false, result);
    }

    if (top) {
        // Put 0 on the stack for t once here instead of after every call to interpolate_color_code.
        result->writeText("0} if\n");
    } else if (first) {
        result->writeText("}");  // The else (hi) side will come next.
    } else {
        result->writeText("} ifelse\n");
    }
}

/* Generate Type 4 function code to map t to the passed gradient, clamping at the ends.
   The types integer, real, and boolean are available.
   There are no string, array, procedure, variable, or name types available.

   The generated code will be of the following form with all values hard coded.

  if (t <= 0) {
    ret = color[0];
    t = 0;
  }
  if (t > 0 && t <= stop[4]) {
    if (t <= stop[2]) {
      if (t <= stop[1]) {
        ret = interp(t - stop[0], stop[1] - stop[0], color[0], color[1]);
      } else {
        ret = interp(t - stop[1], stop[2] - stop[1], color[1], color[2]);
      }
    } else {
      if (t <= stop[3] {
        ret = interp(t - stop[2], stop[3] - stop[2], color[2], color[3]);
      } else {
        ret = interp(t - stop[3], stop[4] - stop[3], color[3], color[4]);
      }
    }
    t = 0;
  }
  if (t > 0) {
    ret = color[4];
  }

   which in PDF will be represented like

  dup 0 le {pop 0 0 0 0} if
  dup dup 0 gt exch 1 le and {
    dup .5 le {
      dup .25 le {
        0 sub 2 mul 0 0
      }{
        .25 sub .5 exch 2 mul 0
      } ifelse
    }{
      dup .75 le {
        .5 sub .5 exch .5 exch 2 mul
      }{
        .75 sub dup 2 mul .5 add exch dup 2 mul .5 add exch 2 mul .5 add
      } ifelse
    } ifelse
  0} if
  0 gt {1 1 1} if
 */
static void gradient_function_code(const SkShaderBase::GradientInfo& info,
                                   SkDynamicMemoryWStream* result) {
    // While looking for a hit the stack is [t].
    // After finding a hit the stack is [r g b 0].
    // The 0 is consumed just before returning.

    const bool premul = is_premul(info);
    NumComponents numComponents = premul ? NumComponents::Four : NumComponents::Three;

    // The initial range has no previous and contains a solid color.
    // Any t <= 0 will be handled by this initial range, so later t == 0 indicates a hit was found.
    result->writeText("dup 0 le {pop ");
    SkPDFUtils::AppendColorComponentF(info.fColors[0].fR, result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponentF(info.fColors[0].fG, result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponentF(info.fColors[0].fB, result);
    if (numComponents == NumComponents::Four) {
        result->writeText(" ");
        SkPDFUtils::AppendColorComponentF(info.fColors[0].fA, result);
    }
    result->writeText(" 0} if\n");

    // Optimize out ranges which don't make any visual difference.
    AutoSTMalloc<4, size_t> rangeEnds(info.fColorCount);
    size_t rangeEndsCount = 0;
    for (int i = 1; i < info.fColorCount; ++i) {
        // Ignoring the alpha, is this range the same solid color as the next range?
        // This optimizes gradients where sometimes only the color or only the alpha is changing.
        auto eqIgnoringAlpha = [&](SkColor4f a, SkColor4f b) {
            if (premul) {
                return a == b;
            } else {
                return a.makeOpaque() == b.makeOpaque();
            }
        };
        bool constantColorBothSides =
            eqIgnoringAlpha(info.fColors[i-1], info.fColors[i]) &&// This range is a solid color.
            i != info.fColorCount-1 &&                            // This is not the last range.
            eqIgnoringAlpha(info.fColors[i], info.fColors[i+1]);  // Next range is same solid color.

        // Does this range have zero size?
        bool degenerateRange = info.fColorOffsets[i-1] == info.fColorOffsets[i];

        if (!degenerateRange && !constantColorBothSides) {
            rangeEnds[rangeEndsCount] = i;
            ++rangeEndsCount;
        }
    }

    // If a cap on depth is needed, loop here.
    write_gradient_ranges(info, SkSpan(rangeEnds.get(), rangeEndsCount),
                          numComponents, true, true, result);

    // Clamp the final color.
    result->writeText("0 gt {");
    SkPDFUtils::AppendColorComponentF(info.fColors[info.fColorCount - 1].fR, result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponentF(info.fColors[info.fColorCount - 1].fG, result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponentF(info.fColors[info.fColorCount - 1].fB, result);
    if (numComponents == NumComponents::Four) {
        result->writeText(" ");
        SkPDFUtils::AppendColorComponentF(info.fColors[info.fColorCount - 1].fA, result);
    }
    result->writeText("} if\n");

    if (premul) {
        unpremul(info, result);
    }
}

static std::unique_ptr<SkPDFDict> createInterpolationFunction(const SkColor4f& color1,
                                                              const SkColor4f& color2) {
    auto retval = SkPDFMakeDict();

    auto c0 = SkPDFMakeArray();
    c0->appendColorComponentF(color1.fR);
    c0->appendColorComponentF(color1.fG);
    c0->appendColorComponentF(color1.fB);
    retval->insertObject("C0", std::move(c0));

    auto c1 = SkPDFMakeArray();
    c1->appendColorComponentF(color2.fR);
    c1->appendColorComponentF(color2.fG);
    c1->appendColorComponentF(color2.fB);
    retval->insertObject("C1", std::move(c1));

    retval->insertObject("Domain", SkPDFMakeArray(0, 1));

    retval->insertInt("FunctionType", 2);
    retval->insertScalar("N", 1.0f);

    return retval;
}

static std::unique_ptr<SkPDFDict> gradientStitchCode(const SkShaderBase::GradientInfo& info) {
    auto retval = SkPDFMakeDict();

    // normalize color stops
    int colorCount = info.fColorCount;
    std::vector<SkColor4f> colors(info.fColors, info.fColors + colorCount);
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

    // no need for a stitch function if there are only 2 stops.
    if (colorCount == 2) {
        return createInterpolationFunction(colors[0], colors[1]);
    }

    auto encode = SkPDFMakeArray();
    auto bounds = SkPDFMakeArray();
    auto functions = SkPDFMakeArray();

    retval->insertObject("Domain", SkPDFMakeArray(0, 1));
    retval->insertInt("FunctionType", 3);

    for (int idx = 1; idx < colorCount; idx++) {
        if (idx > 1) {
            bounds->appendScalar(colorOffsets[idx-1]);
        }

        encode->appendScalar(0);
        encode->appendScalar(1.0f);

        functions->appendObject(createInterpolationFunction(colors[idx-1], colors[idx]));
    }

    retval->insertObject("Encode", std::move(encode));
    retval->insertObject("Bounds", std::move(bounds));
    retval->insertObject("Functions", std::move(functions));

    return retval;
}

/* Map a value of t on the stack into [0, 1) for Repeat or Mirror tile mode. */
static void tileModeCode(SkTileMode mode, SkDynamicMemoryWStream* result) {
    if (mode == SkTileMode::kRepeat) {
        result->writeText("dup truncate sub\n");  // Get the fractional part.
        result->writeText("dup 0 le {1 add} if\n");  // Map (-1,0) => (0,1)
        return;
    }

    if (mode == SkTileMode::kMirror) {
        // In Preview 11.0 (1033.3) `a n mod r eq` (with a and n both integers, r integer or real)
        // early aborts the function when false would be put on the stack.
        // Work around this by re-writing `t 2 mod 1 eq` as `t 2 mod 0 gt`.

        // Map t mod 2 into [0, 1, 1, 0].
        //                Code                 Stack t
        result->writeText("abs "                 // +t
                          "dup "                 // +t.s +t.s
                          "truncate "            // +t.s +t
                          "dup "                 // +t.s +t +t
                          "cvi "                 // +t.s +t +T
                          "2 mod "               // +t.s +t (+T mod 2)
              /*"1 eq "*/ "0 gt "                // +t.s +t true|false
                          "3 1 roll "            // true|false +t.s +t
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

static void linearCode(const SkShaderBase::GradientInfo& info,
                       const SkMatrix& perspectiveRemover,
                       SkDynamicMemoryWStream* function) {
    function->writeText("{");

    apply_perspective_to_coordinates(perspectiveRemover, function);

    function->writeText("pop\n");  // Just ditch the y value.
    tileModeCode((SkTileMode)info.fTileMode, function);
    gradient_function_code(info, function);
    function->writeText("}");
}

static void radialCode(const SkShaderBase::GradientInfo& info,
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

    tileModeCode((SkTileMode)info.fTileMode, function);
    gradient_function_code(info, function);
    function->writeText("}");
}

/* Conical gradient shader, based on the Canvas spec for radial gradients
   See: http://www.w3.org/TR/2dcontext/#dom-context-2d-createradialgradient
 */
static void twoPointConicalCode(const SkShaderBase::GradientInfo& info,
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
    tileModeCode((SkTileMode)info.fTileMode, function);
    gradient_function_code(info, function);

    // otherwise, just write black
    // TODO: Correctly draw gradients_local_persepective, need to mask out this black
    // The "gradients" gm works as falls into the 8.7.4.5.4 "Type 3 (Radial) Shadings" case.
    function->writeText("} {0 0 0} ifelse }");
}

static void sweepCode(const SkShaderBase::GradientInfo& info,
                      const SkMatrix& perspectiveRemover,
                      SkDynamicMemoryWStream* function) {
    function->writeText("{exch atan 360 div\n");
    const SkScalar bias = info.fPoint[1].y();
    if (bias != 0.0f) {
        SkPDFUtils::AppendScalar(bias, function);
        function->writeText(" add\n");
    }
    const SkScalar scale = info.fPoint[1].x();
    if (scale != 1.0f) {
        SkPDFUtils::AppendScalar(scale, function);
        function->writeText(" mul\n");
    }
    tileModeCode((SkTileMode)info.fTileMode, function);
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
    const SkShaderBase::GradientInfo& info = state.fInfo;
    SkMatrix finalMatrix = state.fCanvasTransform;
    finalMatrix.preConcat(state.fShaderTransform);

    bool doStitchFunctions = (state.fType == SkShaderBase::GradientType::kLinear ||
                              state.fType == SkShaderBase::GradientType::kRadial ||
                              state.fType == SkShaderBase::GradientType::kConical) &&
                             (info.fTileMode == SkTileMode::kClamp ||
                              info.fTileMode == SkTileMode::kDecal) &&
                             !finalMatrix.hasPerspective() &&
                             !is_premul(info);

    enum class ShadingType : int32_t {
        Function = 1,
        Axial = 2,
        Radial = 3,
        FreeFormGouraudTriangleMesh = 4,
        LatticeFormGouraudTriangleMesh = 5,
        CoonsPatchMesh = 6,
        TensorProductPatchMesh = 7,
    } shadingType;

    auto pdfShader = SkPDFMakeDict();
    if (doStitchFunctions) {
        pdfShader->insertObject("Function", gradientStitchCode(info));

        if (info.fTileMode == SkTileMode::kClamp) {
            auto extend = SkPDFMakeArray();
            extend->reserve(2);
            extend->appendBool(true);
            extend->appendBool(true);
            pdfShader->insertObject("Extend", std::move(extend));
        }

        std::unique_ptr<SkPDFArray> coords;
        switch (state.fType) {
            case SkShaderBase::GradientType::kLinear: {
                shadingType = ShadingType::Axial;
                const SkPoint& pt1 = info.fPoint[0];
                const SkPoint& pt2 = info.fPoint[1];
                coords = SkPDFMakeArray(pt1.x(), pt1.y(),
                                        pt2.x(), pt2.y());
            } break;
            case SkShaderBase::GradientType::kRadial: {
                shadingType = ShadingType::Radial;
                const SkPoint& pt1 = info.fPoint[0];
                coords = SkPDFMakeArray(pt1.x(), pt1.y(), 0,
                                        pt1.x(), pt1.y(), info.fRadius[0]);
            } break;
            case SkShaderBase::GradientType::kConical: {
                shadingType = ShadingType::Radial;
                SkScalar r1 = info.fRadius[0];
                SkScalar r2 = info.fRadius[1];
                SkPoint pt1 = info.fPoint[0];
                SkPoint pt2 = info.fPoint[1];
                FixUpRadius(pt1, r1, pt2, r2);

                coords = SkPDFMakeArray(pt1.x(), pt1.y(), r1,
                                        pt2.x(), pt2.y(), r2);
                break;
            }
            case SkShaderBase::GradientType::kSweep:
            case SkShaderBase::GradientType::kNone:
            default:
                SkASSERT(false);
                return SkPDFIndirectReference();
        }
        pdfShader->insertObject("Coords", std::move(coords));
    } else {
        shadingType = ShadingType::Function;

        // Transform the coordinate space for the type of gradient.
        transformPoints[0] = info.fPoint[0];
        transformPoints[1] = info.fPoint[1];
        switch (state.fType) {
            case SkShaderBase::GradientType::kLinear:
                break;
            case SkShaderBase::GradientType::kRadial:
                transformPoints[1] = transformPoints[0];
                transformPoints[1].fX += info.fRadius[0];
                break;
            case SkShaderBase::GradientType::kConical: {
                transformPoints[1] = transformPoints[0];
                transformPoints[1].fX += SK_Scalar1;
                break;
            }
            case SkShaderBase::GradientType::kSweep:
                transformPoints[1] = transformPoints[0];
                transformPoints[1].fX += SK_Scalar1;
                break;
            case SkShaderBase::GradientType::kNone:
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
        switch (state.fType) {
            case SkShaderBase::GradientType::kLinear:
                linearCode(info, perspectiveInverseOnly, &functionCode);
                break;
            case SkShaderBase::GradientType::kRadial:
                radialCode(info, perspectiveInverseOnly, &functionCode);
                break;
            case SkShaderBase::GradientType::kConical: {
                // The two point radial gradient further references state.fInfo
                // in translating from x, y coordinates to the t parameter. So, we have
                // to transform the points and radii according to the calculated matrix.
                auto inverseMapperMatrix = mapperMatrix.invert();
                if (!inverseMapperMatrix) {
                    return SkPDFIndirectReference();
                }
                SkShaderBase::GradientInfo infoCopy = info;
                inverseMapperMatrix->mapPoints(infoCopy.fPoint);
                infoCopy.fRadius[0] = inverseMapperMatrix->mapRadius(info.fRadius[0]);
                infoCopy.fRadius[1] = inverseMapperMatrix->mapRadius(info.fRadius[1]);
                twoPointConicalCode(infoCopy, perspectiveInverseOnly, &functionCode);
            } break;
            case SkShaderBase::GradientType::kSweep:
                sweepCode(info, perspectiveInverseOnly, &functionCode);
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

    pdfShader->insertInt("ShadingType", SkToS32(shadingType));
    pdfShader->insertName("ColorSpace", "DeviceRGB");

    SkPDFDict pdfFunctionShader("Pattern");
    pdfFunctionShader.insertInt("PatternType", 2);
    pdfFunctionShader.insertObject("Matrix", SkPDFUtils::MatrixToArray(finalMatrix));
    pdfFunctionShader.insertObject("Shading", std::move(pdfShader));
    return doc->emit(pdfFunctionShader);
}

static SkPDFIndirectReference find_pdf_shader(SkPDFDocument* doc,
                                              SkPDFGradientShader::Key key,
                                              bool makeAlphaShader);

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
    SkPDFUtils::PaintPath(SkPaint::kFill_Style, SkPathFillType::kEvenOdd, &content);
    return content.detachAsStream();
}

static bool gradient_has_alpha(const SkPDFGradientShader::Key& key) {
    SkASSERT(key.fType != SkShaderBase::GradientType::kNone);
    for (int i = 0; i < key.fInfo.fColorCount; i++) {
        if (!key.fInfo.fColors[i].isOpaque()) {
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
        std::unique_ptr<SkColor4f[]>(new SkColor4f[k.fInfo.fColorCount]),
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
    SkASSERT(state.fType != SkShaderBase::GradientType::kNone);
    SkPDFGradientShader::Key luminosityState = clone_key(state);
    for (int i = 0; i < luminosityState.fInfo.fColorCount; i++) {
        float alpha = luminosityState.fInfo.fColors[i].fA;
        luminosityState.fInfo.fColors[i] = SkColor4f{alpha, alpha, alpha, 1.0f};
    }
    luminosityState.fInfo.fGradientFlags &= ~SkGradientShader::kInterpolateColorsInPremul_Flag;
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
    SkASSERT(state.fType != SkShaderBase::GradientType::kNone);
    SkPDFGradientShader::Key opaqueState = clone_key(state);
    const bool keepAlpha = is_premul(opaqueState.fInfo);
    if (!keepAlpha) {
        for (int i = 0; i < opaqueState.fInfo.fColorCount; i++) {
            opaqueState.fInfo.fColors[i].fA = 1.0f;
        }
        opaqueState.fHash = hash(opaqueState);

        SkASSERT(!gradient_has_alpha(opaqueState));
    }
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
         SkShaderBase::GradientType::kNone,
         {0, nullptr, nullptr, {{0, 0}, {0, 0}}, {0, 0}, SkTileMode::kClamp, 0},
         nullptr,
         nullptr,
         canvasTransform,
         SkPDFUtils::GetShaderLocalMatrix(shader),
         bbox, 0};
    key.fType = as_SB(shader)->asGradient(&key.fInfo);
    SkASSERT(SkShaderBase::GradientType::kNone != key.fType);
    SkASSERT(key.fInfo.fColorCount > 0);
    key.fColors = std::make_unique<SkColor4f[]>(key.fInfo.fColorCount);
    key.fStops = std::make_unique<SkScalar[]>(key.fInfo.fColorCount);
    key.fInfo.fColors = key.fColors.get();
    key.fInfo.fColorOffsets = key.fStops.get();
    as_SB(shader)->asGradient(&key.fInfo);
    if (is_premul(key.fInfo)) {
        bool changedByPremul = false;
        for (auto&& c : SkSpan(key.fInfo.fColors, key.fInfo.fColorCount)) {
            if (c.fA != 1.0) {
                changedByPremul = true;
            }
            SkRGBA4f<kPremul_SkAlphaType> pm = c.premul();
            c = SkColor4f{pm.fR, pm.fG, pm.fB, pm.fA};
        }
        if (!changedByPremul) {
            key.fInfo.fGradientFlags &= ~SkGradientShader::kInterpolateColorsInPremul_Flag;
        }
    }
    key.fHash = hash(key);
    return key;
}

static SkPDFIndirectReference find_pdf_shader(SkPDFDocument* doc,
                                              SkPDFGradientShader::Key key,
                                              bool makeAlphaShader) {
    auto& gradientPatternMap = doc->fGradientPatternMap;
    if (SkPDFIndirectReference* ptr = gradientPatternMap.find(key)) {
        return *ptr;
    }
    SkPDFIndirectReference pdfShader;
    if (makeAlphaShader) {
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
    SkASSERT(as_SB(shader)->asGradient() != SkShaderBase::GradientType::kNone);
    SkPDFGradientShader::Key key = make_key(shader, canvasTransform, bbox);
    const bool makeAlphaShader = gradient_has_alpha(key);
    return find_pdf_shader(doc, std::move(key), makeAlphaShader);
}
