/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cmath>
#include "gm.h"
#include "Resources.h"
#include "SkCodec.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpace_A2B.h"
#include "SkColorSpacePriv.h"
#include "SkData.h"
#include "SkFloatingPoint.h"
#include "SkImageInfo.h"
#include "SkScalar.h"
#include "SkSRGB.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTypes.h"

static inline void interp_3d_clut(float dst[3], float src[3], const SkColorLookUpTable* colorLUT) {
    // Call the src components x, y, and z.
    uint8_t maxX = colorLUT->fGridPoints[0] - 1;
    uint8_t maxY = colorLUT->fGridPoints[1] - 1;
    uint8_t maxZ = colorLUT->fGridPoints[2] - 1;

    // An approximate index into each of the three dimensions of the table.
    float x = src[0] * maxX;
    float y = src[1] * maxY;
    float z = src[2] * maxZ;

    // This gives us the low index for our interpolation.
    int ix = sk_float_floor2int(x);
    int iy = sk_float_floor2int(y);
    int iz = sk_float_floor2int(z);

    // Make sure the low index is not also the max index.
    ix = (maxX == ix) ? ix - 1 : ix;
    iy = (maxY == iy) ? iy - 1 : iy;
    iz = (maxZ == iz) ? iz - 1 : iz;

    // Weighting factors for the interpolation.
    float diffX = x - ix;
    float diffY = y - iy;
    float diffZ = z - iz;

    // Constants to help us navigate the 3D table.
    // Ex: Assume x = a, y = b, z = c.
    //     table[a * n001 + b * n010 + c * n100] logically equals table[a][b][c].
    const int n000 = 0;
    const int n001 = 3 * colorLUT->fGridPoints[1] * colorLUT->fGridPoints[2];
    const int n010 = 3 * colorLUT->fGridPoints[2];
    const int n011 = n001 + n010;
    const int n100 = 3;
    const int n101 = n100 + n001;
    const int n110 = n100 + n010;
    const int n111 = n110 + n001;

    // Base ptr into the table.
    const float* ptr = &(colorLUT->table()[ix*n001 + iy*n010 + iz*n100]);

    // The code below performs a tetrahedral interpolation for each of the three
    // dst components.  Once the tetrahedron containing the interpolation point is
    // identified, the interpolation is a weighted sum of grid values at the
    // vertices of the tetrahedron.  The claim is that tetrahedral interpolation
    // provides a more accurate color conversion.
    // blogs.mathworks.com/steve/2006/11/24/tetrahedral-interpolation-for-colorspace-conversion/
    //
    // I have one test image, and visually I can't tell the difference between
    // tetrahedral and trilinear interpolation.  In terms of computation, the
    // tetrahedral code requires more branches but less computation.  The
    // SampleICC library provides an option for the client to choose either
    // tetrahedral or trilinear.
    for (int i = 0; i < 3; i++) {
        if (diffZ < diffY) {
            if (diffZ < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n110] - ptr[n010]) +
                                      diffY * (ptr[n010] - ptr[n000]) +
                                      diffX * (ptr[n111] - ptr[n110]));
            } else if (diffY < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n111] - ptr[n011]) +
                                      diffY * (ptr[n011] - ptr[n001]) +
                                      diffX * (ptr[n001] - ptr[n000]));
            } else {
                dst[i] = (ptr[n000] + diffZ * (ptr[n111] - ptr[n011]) +
                                      diffY * (ptr[n010] - ptr[n000]) +
                                      diffX * (ptr[n011] - ptr[n010]));
            }
        } else {
            if (diffZ < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n101] - ptr[n001]) +
                                      diffY * (ptr[n111] - ptr[n101]) +
                                      diffX * (ptr[n001] - ptr[n000]));
            } else if (diffY < diffX) {
                dst[i] = (ptr[n000] + diffZ * (ptr[n100] - ptr[n000]) +
                                      diffY * (ptr[n111] - ptr[n101]) +
                                      diffX * (ptr[n101] - ptr[n100]));
            } else {
                dst[i] = (ptr[n000] + diffZ * (ptr[n100] - ptr[n000]) +
                                      diffY * (ptr[n110] - ptr[n100]) +
                                      diffX * (ptr[n111] - ptr[n110]));
            }
        }

        // Increment the table ptr in order to handle the next component.
        // Note that this is the how table is designed: all of nXXX
        // variables are multiples of 3 because there are 3 output
        // components.
        ptr++;
    }
}


/**
 *  This tests decoding from a Lab source image and displays on the left
 *  the image as raw RGB values, and on the right a Lab PCS.
 *  It currently does NOT apply a/b/m-curves, as in the .icc profile
 *  We are testing it on these are all identity transforms.
 */
class LabPCSDemoGM : public skiagm::GM {
public:
    LabPCSDemoGM()
        : fWidth(1080)
        , fHeight(480)
        {}

protected:


    SkString onShortName() override {
        return SkString("labpcsdemo");
    }

    SkISize onISize() override {
        return SkISize::Make(fWidth, fHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorGREEN);
        const char* filename = "brickwork-texture.jpg";
        renderImage(canvas, filename, 0, false);
        renderImage(canvas, filename, 1, true);
    }

    void renderImage(SkCanvas* canvas, const char* filename, int col, bool convertLabToXYZ) {
        SkBitmap bitmap;
        SkStream* stream(GetResourceAsStream(filename));
        if (stream == nullptr) {
            return;
        }
        std::unique_ptr<SkCodec> codec(SkCodec::NewFromStream(stream));
        

        // srgb_lab_pcs.icc is an elaborate way to specify sRGB but uses
        // Lab as the PCS, so we can take any arbitrary image that should
        // be sRGB and this should show a reasonable image
        const SkString iccFilename(GetResourcePath("icc_profiles/srgb_lab_pcs.icc"));
        sk_sp<SkData> iccData = SkData::MakeFromFileName(iccFilename.c_str());
        if (iccData == nullptr) {
            return;
        }
        sk_sp<SkColorSpace> colorSpace = SkColorSpace::NewICC(iccData->bytes(), iccData->size());

        const int imageWidth = codec->getInfo().width();
        const int imageHeight = codec->getInfo().height();
        // Using nullptr as the color space instructs the codec to decode in legacy mode,
        // meaning that we will get the raw encoded bytes without any color correction.
        SkImageInfo imageInfo = SkImageInfo::Make(imageWidth, imageHeight, kN32_SkColorType,  
                                                  kOpaque_SkAlphaType, nullptr);
        bitmap.allocPixels(imageInfo);
        codec->getPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes());
        if (convertLabToXYZ) {
            SkASSERT(SkColorSpace_Base::Type::kA2B == as_CSB(colorSpace)->type());
            SkColorSpace_A2B& cs = *static_cast<SkColorSpace_A2B*>(colorSpace.get());
            bool printConversions = false;
            SkASSERT(cs.colorLUT());
            // We're skipping evaluating the TRCs and the matrix here since they aren't
            // in the ICC profile initially used here.
            SkASSERT(kLinear_SkGammaNamed == cs.aCurveNamed());
            SkASSERT(kLinear_SkGammaNamed == cs.mCurveNamed());
            SkASSERT(kLinear_SkGammaNamed == cs.bCurveNamed());
            SkASSERT(cs.matrix().isIdentity());
            for (int y = 0; y < imageHeight; ++y) {
                for (int x = 0; x < imageWidth; ++x) {
                    uint32_t& p = *bitmap.getAddr32(x, y);
                    const int r = SkColorGetR(p);
                    const int g = SkColorGetG(p);
                    const int b = SkColorGetB(p);
                    if (printConversions) {
                        SkColorSpacePrintf("\nraw = (%d, %d, %d)\t", r, g, b);
                    }

                    float lab[4] = { r * (1.f/255.f), g * (1.f/255.f), b * (1.f/255.f), 1.f };
                    
                    interp_3d_clut(lab, lab, cs.colorLUT());
                    
                    // Lab has ranges [0,100] for L and [-128,127] for a and b
                    // but the ICC profile loader stores as [0,1]. The ICC
                    // specifies an offset of -128 to convert.
                    // note: formula could be adjusted to remove this conversion,
                    //       but for now let's keep it like this for clarity until
                    //       an optimized version is added.
                    lab[0] *= 100.f;
                    lab[1] = 255.f * lab[1] - 128.f;
                    lab[2] = 255.f * lab[2] - 128.f;
                    if (printConversions) {
                        SkColorSpacePrintf("Lab = < %f, %f, %f >\n", lab[0], lab[1], lab[2]);
                    }

                    // convert from Lab to XYZ
                    float Y = (lab[0] + 16.f) * (1.f/116.f);
                    float X = lab[1] * (1.f/500.f) + Y;
                    float Z = Y - (lab[2] * (1.f/200.f));
                    float cubed;
                    cubed = X*X*X;
                    if (cubed > 0.008856f)
                        X = cubed;
                    else
                        X = (X - (16.f/116.f)) * (1.f/7.787f);
                    cubed = Y*Y*Y;
                    if (cubed > 0.008856f)
                        Y = cubed;
                    else
                        Y = (Y - (16.f/116.f)) * (1.f/7.787f);
                    cubed = Z*Z*Z;
                    if (cubed > 0.008856f)
                        Z = cubed;
                    else
                        Z = (Z - (16.f/116.f)) * (1.f/7.787f);

                    // adjust to D50 illuminant
                    X *= 0.96422f;
                    Y *= 1.00000f;
                    Z *= 0.82521f;

                    if (printConversions) {
                        SkColorSpacePrintf("XYZ = (%4f, %4f, %4f)\t", X, Y, Z);
                    }

                    // convert XYZ -> linear sRGB
                    Sk4f lRGB( 3.1338561f*X - 1.6168667f*Y - 0.4906146f*Z,
                              -0.9787684f*X + 1.9161415f*Y + 0.0334540f*Z,
                               0.0719453f*X - 0.2289914f*Y + 1.4052427f*Z,
                              1.f);
                    // and apply sRGB gamma
                    Sk4i sRGB = sk_linear_to_srgb(lRGB);
                    if (printConversions) {
                        SkColorSpacePrintf("sRGB = (%d, %d, %d)\n", sRGB[0], sRGB[1], sRGB[2]);
                    }
                    p = SkColorSetRGB(sRGB[0], sRGB[1], sRGB[2]);
                }
            }
        }
        const int freeWidth = fWidth - 2*imageWidth;
        const int freeHeight = fHeight - imageHeight;
        canvas->drawBitmap(bitmap,
                           static_cast<SkScalar>((col+1) * (freeWidth / 3) + col*imageWidth),
                           static_cast<SkScalar>(freeHeight / 2));
        ++col;
    }

private:
    const int fWidth;
    const int fHeight;

    typedef skiagm::GM INHERITED;
};

DEF_GM( return new LabPCSDemoGM; )
