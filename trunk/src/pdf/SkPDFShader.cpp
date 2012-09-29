
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFShader.h"

#include "SkCanvas.h"
#include "SkData.h"
#include "SkPDFCatalog.h"
#include "SkPDFDevice.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkThread.h"
#include "SkTypes.h"

static bool transformBBox(const SkMatrix& matrix, SkRect* bbox) {
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
    matrix->preTranslate(pts[0].fX, pts[0].fY);
    matrix->preScale(mag, mag);
}

/* Assumes t + startOffset is on the stack and does a linear interpolation on t
   between startOffset and endOffset from prevColor to curColor (for each color
   component), leaving the result in component order on the stack.
   @param range                  endOffset - startOffset
   @param curColor[components]   The current color components.
   @param prevColor[components]  The previous color components.
   @param result                 The result ps function.
 */
static void interpolateColorCode(SkScalar range, SkScalar* curColor,
                                 SkScalar* prevColor, int components,
                                 SkString* result) {
    // Figure out how to scale each color component.
    SkAutoSTMalloc<4, SkScalar> multiplierAlloc(components);
    SkScalar *multiplier = multiplierAlloc.get();
    for (int i = 0; i < components; i++) {
        multiplier[i] = SkScalarDiv(curColor[i] - prevColor[i], range);
    }

    // Calculate when we no longer need to keep a copy of the input parameter t.
    // If the last component to use t is i, then dupInput[0..i - 1] = true
    // and dupInput[i .. components] = false.
    SkAutoSTMalloc<4, bool> dupInputAlloc(components);
    bool *dupInput = dupInputAlloc.get();
    dupInput[components - 1] = false;
    for (int i = components - 2; i >= 0; i--) {
        dupInput[i] = dupInput[i + 1] || multiplier[i + 1] != 0;
    }

    if (!dupInput[0] && multiplier[0] == 0) {
        result->append("pop ");
    }

    for (int i = 0; i < components; i++) {
        // If the next components needs t, make a copy.
        if (dupInput[i]) {
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
    for (int i = 1 ; i < info.fColorCount; i++) {
        result->append("{dup ");
        result->appendScalar(info.fColorOffsets[i]);
        result->append(" le {");
        if (info.fColorOffsets[i - 1] != 0) {
            result->appendScalar(info.fColorOffsets[i - 1]);
            result->append(" sub\n");
        }

        interpolateColorCode(info.fColorOffsets[i] - info.fColorOffsets[i - 1],
                             colorData[i], colorData[i - 1], kColorComponents,
                             result);
        result->append("}\n");
    }

    // Clamp the final color.
    result->append("{pop ");
    result->appendScalar(colorData[info.fColorCount - 1][0]);
    result->append(" ");
    result->appendScalar(colorData[info.fColorCount - 1][1]);
    result->append(" ");
    result->appendScalar(colorData[info.fColorCount - 1][2]);

    for (int i = 0 ; i < info.fColorCount; i++) {
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

static SkString linearCode(const SkShader::GradientInfo& info) {
    SkString function("{pop\n");  // Just ditch the y value.
    tileModeCode(info.fTileMode, &function);
    gradientFunctionCode(info, &function);
    function.append("}");
    return function;
}

static SkString radialCode(const SkShader::GradientInfo& info) {
    SkString function("{");
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

/* The math here is all based on the description in Two_Point_Radial_Gradient,
   with one simplification, the coordinate space has been scaled so that
   Dr = 1.  This means we don't need to scale the entire equation by 1/Dr^2.
 */
static SkString twoPointRadialCode(const SkShader::GradientInfo& info) {
    SkScalar dx = info.fPoint[0].fX - info.fPoint[1].fX;
    SkScalar dy = info.fPoint[0].fY - info.fPoint[1].fY;
    SkScalar sr = info.fRadius[0];
    SkScalar a = SkScalarMul(dx, dx) + SkScalarMul(dy, dy) - SK_Scalar1;
    bool posRoot = info.fRadius[1] > info.fRadius[0];

    // We start with a stack of (x y), copy it and then consume one copy in
    // order to calculate b and the other to calculate c.
    SkString function("{");
    function.append("2 copy ");

    // Calculate -b and b^2.
    function.appendScalar(dy);
    function.append(" mul exch ");
    function.appendScalar(dx);
    function.append(" mul add ");
    function.appendScalar(sr);
    function.append(" sub 2 mul neg dup dup mul\n");

    // Calculate c
    function.append("4 2 roll dup mul exch dup mul add ");
    function.appendScalar(SkScalarMul(sr, sr));
    function.append(" sub\n");

    // Calculate the determinate
    function.appendScalar(SkScalarMul(SkIntToScalar(4), a));
    function.append(" mul sub abs sqrt\n");

    // And then the final value of t.
    if (posRoot) {
        function.append("sub ");
    } else {
        function.append("add ");
    }
    function.appendScalar(SkScalarMul(SkIntToScalar(2), a));
    function.append(" div\n");

    tileModeCode(info.fTileMode, &function);
    gradientFunctionCode(info, &function);
    function.append("}");
    return function;
}

/* Conical gradient shader, based on the Canvas spec for radial gradients
   See: http://www.w3.org/TR/2dcontext/#dom-context-2d-createradialgradient
 */
static SkString twoPointConicalCode(const SkShader::GradientInfo& info) {
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

static SkString sweepCode(const SkShader::GradientInfo& info) {
    SkString function("{exch atan 360 div\n");
    tileModeCode(info.fTileMode, &function);
    gradientFunctionCode(info, &function);
    function.append("}");
    return function;
}

class SkPDFShader::State {
public:
    SkShader::GradientType fType;
    SkShader::GradientInfo fInfo;
    SkAutoFree fColorData;
    SkMatrix fCanvasTransform;
    SkMatrix fShaderTransform;
    SkIRect fBBox;

    SkBitmap fImage;
    uint32_t fPixelGeneration;
    SkShader::TileMode fImageTileModes[2];

    explicit State(const SkShader& shader, const SkMatrix& canvasTransform,
                   const SkIRect& bbox);
    bool operator==(const State& b) const;
};

class SkPDFFunctionShader : public SkPDFDict, public SkPDFShader {
public:
    explicit SkPDFFunctionShader(SkPDFShader::State* state);
    virtual ~SkPDFFunctionShader() {
        if (isValid()) {
            RemoveShader(this);
        }
        fResources.unrefAll();
    }

    virtual bool isValid() { return fResources.count() > 0; }

    void getResources(SkTDArray<SkPDFObject*>* resourceList) {
        GetResourcesHelper(&fResources, resourceList);
    }

private:
    static SkPDFObject* RangeObject();

    SkTDArray<SkPDFObject*> fResources;
    SkAutoTDelete<const SkPDFShader::State> fState;

    SkPDFStream* makePSFunction(const SkString& psCode, SkPDFArray* domain);
};

class SkPDFImageShader : public SkPDFStream, public SkPDFShader {
public:
    explicit SkPDFImageShader(SkPDFShader::State* state);
    virtual ~SkPDFImageShader() {
        RemoveShader(this);
        fResources.unrefAll();
    }

    virtual bool isValid() { return size() > 0; }

    void getResources(SkTDArray<SkPDFObject*>* resourceList) {
        GetResourcesHelper(&fResources, resourceList);
    }

private:
    SkTDArray<SkPDFObject*> fResources;
    SkAutoTDelete<const SkPDFShader::State> fState;
};

SkPDFShader::SkPDFShader() {}

// static
void SkPDFShader::RemoveShader(SkPDFObject* shader) {
    SkAutoMutexAcquire lock(CanonicalShadersMutex());
    ShaderCanonicalEntry entry(shader, NULL);
    int index = CanonicalShaders().find(entry);
    SkASSERT(index >= 0);
    CanonicalShaders().removeShuffle(index);
}

// static
SkPDFObject* SkPDFShader::GetPDFShader(const SkShader& shader,
                                       const SkMatrix& matrix,
                                       const SkIRect& surfaceBBox) {
    SkPDFObject* result;
    SkAutoMutexAcquire lock(CanonicalShadersMutex());
    SkAutoTDelete<State> shaderState(new State(shader, matrix, surfaceBBox));
    if (shaderState.get()->fType == SkShader::kNone_GradientType &&
            shaderState.get()->fImage.isNull()) {
        // TODO(vandebo) This drops SKComposeShader on the floor.  We could
        // handle compose shader by pulling things up to a layer, drawing with
        // the first shader, applying the xfer mode and drawing again with the
        // second shader, then applying the layer to the original drawing.
        return NULL;
    }

    ShaderCanonicalEntry entry(NULL, shaderState.get());
    int index = CanonicalShaders().find(entry);
    if (index >= 0) {
        result = CanonicalShaders()[index].fPDFShader;
        result->ref();
        return result;
    }

    bool valid = false;
    // The PDFShader takes ownership of the shaderSate.
    if (shaderState.get()->fType == SkShader::kNone_GradientType) {
        SkPDFImageShader* imageShader =
            new SkPDFImageShader(shaderState.detach());
        valid = imageShader->isValid();
        result = imageShader;
    } else {
        SkPDFFunctionShader* functionShader =
            new SkPDFFunctionShader(shaderState.detach());
        valid = functionShader->isValid();
        result = functionShader;
    }
    if (!valid) {
        delete result;
        return NULL;
    }
    entry.fPDFShader = result;
    CanonicalShaders().push(entry);
    return result;  // return the reference that came from new.
}

// static
SkTDArray<SkPDFShader::ShaderCanonicalEntry>& SkPDFShader::CanonicalShaders() {
    // This initialization is only thread safe with gcc.
    static SkTDArray<ShaderCanonicalEntry> gCanonicalShaders;
    return gCanonicalShaders;
}

// static
SkBaseMutex& SkPDFShader::CanonicalShadersMutex() {
    // This initialization is only thread safe with gcc or when
    // POD-style mutex initialization is used.
    SK_DECLARE_STATIC_MUTEX(gCanonicalShadersMutex);
    return gCanonicalShadersMutex;
}

// static
SkPDFObject* SkPDFFunctionShader::RangeObject() {
    // This initialization is only thread safe with gcc.
    static SkPDFArray* range = NULL;
    // This method is only used with CanonicalShadersMutex, so it's safe to
    // populate domain.
    if (range == NULL) {
        range = new SkPDFArray;
        range->reserve(6);
        range->appendInt(0);
        range->appendInt(1);
        range->appendInt(0);
        range->appendInt(1);
        range->appendInt(0);
        range->appendInt(1);
    }
    return range;
}

SkPDFFunctionShader::SkPDFFunctionShader(SkPDFShader::State* state)
        : SkPDFDict("Pattern"),
          fState(state) {
    SkString (*codeFunction)(const SkShader::GradientInfo& info) = NULL;
    SkPoint transformPoints[2];

    // Depending on the type of the gradient, we want to transform the
    // coordinate space in different ways.
    const SkShader::GradientInfo* info = &fState.get()->fInfo;
    transformPoints[0] = info->fPoint[0];
    transformPoints[1] = info->fPoint[1];
    switch (fState.get()->fType) {
        case SkShader::kLinear_GradientType:
            codeFunction = &linearCode;
            break;
        case SkShader::kRadial_GradientType:
            transformPoints[1] = transformPoints[0];
            transformPoints[1].fX += info->fRadius[0];
            codeFunction = &radialCode;
            break;
        case SkShader::kRadial2_GradientType: {
            // Bail out if the radii are the same.  Empty fResources signals
            // an error and isValid will return false.
            if (info->fRadius[0] == info->fRadius[1]) {
                return;
            }
            transformPoints[1] = transformPoints[0];
            SkScalar dr = info->fRadius[1] - info->fRadius[0];
            transformPoints[1].fX += dr;
            codeFunction = &twoPointRadialCode;
            break;
        }
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
            return;
    }

    // Move any scaling (assuming a unit gradient) or translation
    // (and rotation for linear gradient), of the final gradient from
    // info->fPoints to the matrix (updating bbox appropriately).  Now
    // the gradient can be drawn on on the unit segment.
    SkMatrix mapperMatrix;
    unitToPointsMatrix(transformPoints, &mapperMatrix);
    SkMatrix finalMatrix = fState.get()->fCanvasTransform;
    finalMatrix.preConcat(mapperMatrix);
    finalMatrix.preConcat(fState.get()->fShaderTransform);
    SkRect bbox;
    bbox.set(fState.get()->fBBox);
    if (!transformBBox(finalMatrix, &bbox)) {
        return;
    }

    SkRefPtr<SkPDFArray> domain = new SkPDFArray;
    domain->unref();  // SkRefPtr and new both took a reference.
    domain->reserve(4);
    domain->appendScalar(bbox.fLeft);
    domain->appendScalar(bbox.fRight);
    domain->appendScalar(bbox.fTop);
    domain->appendScalar(bbox.fBottom);

    SkString functionCode;
    // The two point radial gradient further references fState.get()->fInfo
    // in translating from x, y coordinates to the t parameter. So, we have
    // to transform the points and radii according to the calculated matrix.
    if (fState.get()->fType == SkShader::kRadial2_GradientType) {
        SkShader::GradientInfo twoPointRadialInfo = *info;
        SkMatrix inverseMapperMatrix;
        if (!mapperMatrix.invert(&inverseMapperMatrix)) {
            return;
        }
        inverseMapperMatrix.mapPoints(twoPointRadialInfo.fPoint, 2);
        twoPointRadialInfo.fRadius[0] =
            inverseMapperMatrix.mapRadius(info->fRadius[0]);
        twoPointRadialInfo.fRadius[1] =
            inverseMapperMatrix.mapRadius(info->fRadius[1]);
        functionCode = codeFunction(twoPointRadialInfo);
    } else {
        functionCode = codeFunction(*info);
    }

    SkRefPtr<SkPDFStream> function = makePSFunction(functionCode, domain.get());
    // Pass one reference to fResources, SkRefPtr and new both took a reference.
    fResources.push(function.get());

    SkRefPtr<SkPDFDict> pdfShader = new SkPDFDict;
    pdfShader->unref();  // SkRefPtr and new both took a reference.
    pdfShader->insertInt("ShadingType", 1);
    pdfShader->insertName("ColorSpace", "DeviceRGB");
    pdfShader->insert("Domain", domain.get());
    pdfShader->insert("Function", new SkPDFObjRef(function.get()))->unref();

    insertInt("PatternType", 2);
    insert("Matrix", SkPDFUtils::MatrixToArray(finalMatrix))->unref();
    insert("Shading", pdfShader.get());
}

SkPDFImageShader::SkPDFImageShader(SkPDFShader::State* state) : fState(state) {
    fState.get()->fImage.lockPixels();

    SkMatrix finalMatrix = fState.get()->fCanvasTransform;
    finalMatrix.preConcat(fState.get()->fShaderTransform);
    SkRect surfaceBBox;
    surfaceBBox.set(fState.get()->fBBox);
    if (!transformBBox(finalMatrix, &surfaceBBox)) {
        return;
    }

    SkMatrix unflip;
    unflip.setTranslate(0, SkScalarRoundToScalar(surfaceBBox.height()));
    unflip.preScale(SK_Scalar1, -SK_Scalar1);
    SkISize size = SkISize::Make(SkScalarRound(surfaceBBox.width()),
                                 SkScalarRound(surfaceBBox.height()));
    SkPDFDevice pattern(size, size, unflip);
    SkCanvas canvas(&pattern);
    canvas.translate(-surfaceBBox.fLeft, -surfaceBBox.fTop);
    finalMatrix.preTranslate(surfaceBBox.fLeft, surfaceBBox.fTop);

    const SkBitmap* image = &fState.get()->fImage;
    SkScalar width = SkIntToScalar(image->width());
    SkScalar height = SkIntToScalar(image->height());
    SkShader::TileMode tileModes[2];
    tileModes[0] = fState.get()->fImageTileModes[0];
    tileModes[1] = fState.get()->fImageTileModes[1];

    canvas.drawBitmap(*image, 0, 0);
    SkRect patternBBox = SkRect::MakeXYWH(-surfaceBBox.fLeft, -surfaceBBox.fTop,
                                          width, height);

    // Tiling is implied.  First we handle mirroring.
    if (tileModes[0] == SkShader::kMirror_TileMode) {
        SkMatrix xMirror;
        xMirror.setScale(-1, 1);
        xMirror.postTranslate(2 * width, 0);
        canvas.drawBitmapMatrix(*image, xMirror);
        patternBBox.fRight += width;
    }
    if (tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix yMirror;
        yMirror.setScale(SK_Scalar1, -SK_Scalar1);
        yMirror.postTranslate(0, 2 * height);
        canvas.drawBitmapMatrix(*image, yMirror);
        patternBBox.fBottom += height;
    }
    if (tileModes[0] == SkShader::kMirror_TileMode &&
            tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix mirror;
        mirror.setScale(-1, -1);
        mirror.postTranslate(2 * width, 2 * height);
        canvas.drawBitmapMatrix(*image, mirror);
    }

    // Then handle Clamping, which requires expanding the pattern canvas to
    // cover the entire surfaceBBox.

    // If both x and y are in clamp mode, we start by filling in the corners.
    // (Which are just a rectangles of the corner colors.)
    if (tileModes[0] == SkShader::kClamp_TileMode &&
            tileModes[1] == SkShader::kClamp_TileMode) {
        SkPaint paint;
        SkRect rect;
        rect = SkRect::MakeLTRB(surfaceBBox.fLeft, surfaceBBox.fTop, 0, 0);
        if (!rect.isEmpty()) {
            paint.setColor(image->getColor(0, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, surfaceBBox.fTop, surfaceBBox.fRight, 0);
        if (!rect.isEmpty()) {
            paint.setColor(image->getColor(image->width() - 1, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, height, surfaceBBox.fRight,
                                surfaceBBox.fBottom);
        if (!rect.isEmpty()) {
            paint.setColor(image->getColor(image->width() - 1,
                                           image->height() - 1));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(surfaceBBox.fLeft, height, 0,
                                surfaceBBox.fBottom);
        if (!rect.isEmpty()) {
            paint.setColor(image->getColor(0, image->height() - 1));
            canvas.drawRect(rect, paint);
        }
    }

    // Then expand the left, right, top, then bottom.
    if (tileModes[0] == SkShader::kClamp_TileMode) {
        SkIRect subset = SkIRect::MakeXYWH(0, 0, 1, image->height());
        if (surfaceBBox.fLeft < 0) {
            SkBitmap left;
            SkAssertResult(image->extractSubset(&left, subset));

            SkMatrix leftMatrix;
            leftMatrix.setScale(-surfaceBBox.fLeft, 1);
            leftMatrix.postTranslate(surfaceBBox.fLeft, 0);
            canvas.drawBitmapMatrix(left, leftMatrix);

            if (tileModes[1] == SkShader::kMirror_TileMode) {
                leftMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                leftMatrix.postTranslate(0, 2 * height);
                canvas.drawBitmapMatrix(left, leftMatrix);
            }
            patternBBox.fLeft = 0;
        }

        if (surfaceBBox.fRight > width) {
            SkBitmap right;
            subset.offset(image->width() - 1, 0);
            SkAssertResult(image->extractSubset(&right, subset));

            SkMatrix rightMatrix;
            rightMatrix.setScale(surfaceBBox.fRight - width, 1);
            rightMatrix.postTranslate(width, 0);
            canvas.drawBitmapMatrix(right, rightMatrix);

            if (tileModes[1] == SkShader::kMirror_TileMode) {
                rightMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                rightMatrix.postTranslate(0, 2 * height);
                canvas.drawBitmapMatrix(right, rightMatrix);
            }
            patternBBox.fRight = surfaceBBox.width();
        }
    }

    if (tileModes[1] == SkShader::kClamp_TileMode) {
        SkIRect subset = SkIRect::MakeXYWH(0, 0, image->width(), 1);
        if (surfaceBBox.fTop < 0) {
            SkBitmap top;
            SkAssertResult(image->extractSubset(&top, subset));

            SkMatrix topMatrix;
            topMatrix.setScale(SK_Scalar1, -surfaceBBox.fTop);
            topMatrix.postTranslate(0, surfaceBBox.fTop);
            canvas.drawBitmapMatrix(top, topMatrix);

            if (tileModes[0] == SkShader::kMirror_TileMode) {
                topMatrix.postScale(-1, 1);
                topMatrix.postTranslate(2 * width, 0);
                canvas.drawBitmapMatrix(top, topMatrix);
            }
            patternBBox.fTop = 0;
        }

        if (surfaceBBox.fBottom > height) {
            SkBitmap bottom;
            subset.offset(0, image->height() - 1);
            SkAssertResult(image->extractSubset(&bottom, subset));

            SkMatrix bottomMatrix;
            bottomMatrix.setScale(SK_Scalar1, surfaceBBox.fBottom - height);
            bottomMatrix.postTranslate(0, height);
            canvas.drawBitmapMatrix(bottom, bottomMatrix);

            if (tileModes[0] == SkShader::kMirror_TileMode) {
                bottomMatrix.postScale(-1, 1);
                bottomMatrix.postTranslate(2 * width, 0);
                canvas.drawBitmapMatrix(bottom, bottomMatrix);
            }
            patternBBox.fBottom = surfaceBBox.height();
        }
    }

    SkRefPtr<SkPDFArray> patternBBoxArray = new SkPDFArray;
    patternBBoxArray->unref();  // SkRefPtr and new both took a reference.
    patternBBoxArray->reserve(4);
    patternBBoxArray->appendScalar(patternBBox.fLeft);
    patternBBoxArray->appendScalar(patternBBox.fTop);
    patternBBoxArray->appendScalar(patternBBox.fRight);
    patternBBoxArray->appendScalar(patternBBox.fBottom);

    // Put the canvas into the pattern stream (fContent).
    SkRefPtr<SkStream> content = pattern.content();
    content->unref();  // SkRefPtr and content() both took a reference.
    pattern.getResources(&fResources, false);

    setData(content.get());
    insertName("Type", "Pattern");
    insertInt("PatternType", 1);
    insertInt("PaintType", 1);
    insertInt("TilingType", 1);
    insert("BBox", patternBBoxArray.get());
    insertScalar("XStep", patternBBox.width());
    insertScalar("YStep", patternBBox.height());
    insert("Resources", pattern.getResourceDict());
    insert("Matrix", SkPDFUtils::MatrixToArray(finalMatrix))->unref();

    fState.get()->fImage.unlockPixels();
}

SkPDFStream* SkPDFFunctionShader::makePSFunction(const SkString& psCode,
                                                 SkPDFArray* domain) {
    SkAutoDataUnref funcData(SkData::NewWithCopy(psCode.c_str(),
                                                 psCode.size()));
    SkPDFStream* result = new SkPDFStream(funcData.get());
    result->insertInt("FunctionType", 4);
    result->insert("Domain", domain);
    result->insert("Range", RangeObject());
    return result;
}

SkPDFShader::ShaderCanonicalEntry::ShaderCanonicalEntry(SkPDFObject* pdfShader,
                                                        const State* state)
    : fPDFShader(pdfShader),
      fState(state) {
}

bool SkPDFShader::ShaderCanonicalEntry::operator==(
        const ShaderCanonicalEntry& b) const {
    return fPDFShader == b.fPDFShader ||
           (fState != NULL && b.fState != NULL && *fState == *b.fState);
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
            case SkShader::kRadial2_GradientType:
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

SkPDFShader::State::State(const SkShader& shader,
                          const SkMatrix& canvasTransform, const SkIRect& bbox)
        : fCanvasTransform(canvasTransform),
          fBBox(bbox),
          fPixelGeneration(0) {
    fInfo.fColorCount = 0;
    fInfo.fColors = NULL;
    fInfo.fColorOffsets = NULL;
    shader.getLocalMatrix(&fShaderTransform);
    fImageTileModes[0] = fImageTileModes[1] = SkShader::kClamp_TileMode;

    fType = shader.asAGradient(&fInfo);

    if (fType == SkShader::kNone_GradientType) {
        SkShader::BitmapType bitmapType;
        SkMatrix matrix;
        bitmapType = shader.asABitmap(&fImage, &matrix, fImageTileModes);
        if (bitmapType != SkShader::kDefault_BitmapType) {
            fImage.reset();
            return;
        }
        SkASSERT(matrix.isIdentity());
        fPixelGeneration = fImage.getGenerationID();
    } else {
        fColorData.set(sk_malloc_throw(
                    fInfo.fColorCount * (sizeof(SkColor) + sizeof(SkScalar))));
        fInfo.fColors = reinterpret_cast<SkColor*>(fColorData.get());
        fInfo.fColorOffsets =
            reinterpret_cast<SkScalar*>(fInfo.fColors + fInfo.fColorCount);
        shader.asAGradient(&fInfo);
    }
}
