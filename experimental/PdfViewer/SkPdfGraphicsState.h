/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfGraphicsState_DEFINED
#define SkPdfGraphicsState_DEFINED

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPdfConfig.h"
#include "SkPdfUtils.h"

class SkPdfFont;
class SkPdfNativeObject;
class SkPdfResourceDictionary;
class SkPdfSoftMaskDictionary;

/** \class SkPdfColorOperator
 *   Operates on stroking or non-stroking properties.
 */
class SkPdfColorOperator {

    /*
    color space   name or array     The current color space in which color values are to be interpreted
                                    (see Section 4.5, “Color Spaces”). There are two separate color space
                                    parameters: one for stroking and one for all other painting opera-
                                    tions. Initial value: DeviceGray.
     */

    // TODO(edisonn): implement the array part too
// TODO(edisonn): remove this public, let fields be private
public:
    NotOwnedString fColorSpace;
    SkPdfNativeObject* fPattern;

    /*
    color         (various)         The current color to be used during painting operations (see Section
                                    4.5, “Color Spaces”). The type and interpretation of this parameter
                                    depend on the current color space; for most color spaces, a color
                                    value consists of one to four numbers. There are two separate color
                                    parameters: one for stroking and one for all other painting opera-
                                    tions. Initial value: black.
     */

    SkColor fColor;
    double fOpacity;  // ca or CA

public:
    void setRGBColor(SkColor color) {
        // TODO(edisonn): ASSERT DeviceRGB is the color space.
        fPattern = NULL;
        fColor = color;
    }

    // TODO(edisonn): implement the default values for all fields.
    SkPdfColorOperator() : fPattern(NULL), fColor(SK_ColorBLACK), fOpacity(1) {
        NotOwnedString::init(&fColorSpace, "DeviceRGB");
    }

    void setColorSpace(NotOwnedString* colorSpace) {
        fColorSpace = *colorSpace;
        fPattern = NULL;
    }

    void setPatternColorSpace(SkPdfNativeObject* pattern) {
        fColorSpace.fBuffer = (const unsigned char*)"Pattern";
        fColorSpace.fBytes = 7;  // strlen("Pattern")
        fPattern = pattern;
    }

    void applyGraphicsState(SkPaint* paint) {
        paint->setColor(SkColorSetA(fColor, (U8CPU)(fOpacity * 255)));
    }
};

/**
 *   Operates on stroking or non-stroking properties.
 */
struct SkPdfGraphicsState {
    // TODO(edisonn): deprecate and remove these!
    double              fCurPosX;
    double              fCurPosY;

    double              fCurFontSize;
    bool                fTextBlock;
    SkPdfFont*          fSkFont;
    SkPath              fPath;
    bool                fPathClosed;

    double              fTextLeading;
    double              fWordSpace;
    double              fCharSpace;

    SkPdfResourceDictionary* fResources;


    // TODO(edisonn): Can we move most of these in canvas/paint?
    // Might need to strore some properties in 2 paints (stroking paint and non stroking paint)

//    TABLE 4.2 Device-independent graphics state parameters
/*
 * CTM           array             The current transformation matrix, which maps positions from user
                                coordinates to device coordinates (see Section 4.2, “Coordinate Sys-
                                tems”). This matrix is modiﬁed by each application of the coordi-
                                nate transformation operator, cm. Initial value: a matrix that
                                transforms default user coordinates to device coordinates.
 */
    SkMatrix fCTM;

    SkMatrix fContentStreamMatrix;

/*
clipping path (internal)        The current clipping path, which deﬁnes the boundary against
                                which all output is to be cropped (see Section 4.4.3, “Clipping Path
                                Operators”). Initial value: the boundary of the entire imageable
                                portion of the output page.
 */
    // Clip that is applied after the drawing is done!!!
    bool                fHasClipPathToApply;
    SkPath              fClipPath;

    SkPdfColorOperator  fStroking;
    SkPdfColorOperator  fNonStroking;

/*
text state    (various)         A set of nine graphics state parameters that pertain only to the
                                painting of text. These include parameters that select the font, scale
                                the glyphs to an appropriate size, and accomplish other effects. The
                                text state parameters are described in Section 5.2, “Text State
                                Parameters and Operators.”
 */

    // TODO(edisonn): add SkPdfTextState class. remove these two existing fields
    SkMatrix            fMatrixTm;
    SkMatrix            fMatrixTlm;


/*
line width    number            The thickness, in user space units, of paths to be stroked (see “Line
                                Width” on page 152). Initial value: 1.0.
 */
    double              fLineWidth;


/*
line cap      integer           A code specifying the shape of the endpoints for any open path that
                                is stroked (see “Line Cap Style” on page 153). Initial value: 0, for
                                square butt caps.
 */
    // TODO (edisonn): implement defaults - page 153
    int fLineCap;

/*
line join     integer           A code specifying the shape of joints between connected segments
                                of a stroked path (see “Line Join Style” on page 153). Initial value: 0,
                                for mitered joins.
 */
    // TODO (edisonn): implement defaults - page 153
    int fLineJoin;

/*
miter limit   number            The maximum length of mitered line joins for stroked paths (see
                                “Miter Limit” on page 153). This parameter limits the length of
                                “spikes” produced when line segments join at sharp angles. Initial
                                value: 10.0, for a miter cutoff below approximately 11.5 degrees.
 */
    // TODO (edisonn): implement defaults - page 153
    double fMiterLimit;

/*
dash pattern      array and     A description of the dash pattern to be used when paths are
                  number        stroked (see “Line Dash Pattern” on page 155). Initial value: a solid
                                line.
 */
    SkScalar fDashArray[256]; // TODO(edisonn): allocate array?
    int fDashArrayLength;
    SkScalar fDashPhase;


/*
rendering intent  name          The rendering intent to be used when converting CIE-based colors
                                to device colors (see “Rendering Intents” on page 197). Default
                                value: RelativeColorimetric.
 */
    // TODO(edisonn): seems paper only. Verify.

/*
stroke adjustment boolean       (PDF 1.2) A ﬂag specifying whether to compensate for possible ras-
                                terization effects when stroking a path with a line width that is
                                small relative to the pixel resolution of the output device (see Sec-
                                tion 6.5.4, “Automatic Stroke Adjustment”). Note that this is con-
                                sidered a device-independent parameter, even though the details of
                                its effects are device-dependent. Initial value: false.
 */
    // TODO(edisonn): stroke adjustment low priority.


/*
blend mode        name or array (PDF 1.4) The current blend mode to be used in the transparent
                                imaging model (see Sections 7.2.4, “Blend Mode,” and 7.5.2, “Spec-
                                ifying Blending Color Space and Blend Mode”). This parameter is
                                implicitly reset to its initial value at the beginning of execution of a
                                transparency group XObject (see Section 7.5.5, “Transparency
                                Group XObjects”). Initial value: Normal.
 */
    SkXfermode::Mode fBlendModes[256];
    int fBlendModesLength;

/*
soft mask         dictionary    (PDF 1.4) A soft-mask dictionary (see “Soft-Mask Dictionaries” on
                  or name       page 445) specifying the mask shape or mask opacity values to be
                                used in the transparent imaging model (see “Source Shape and
                                Opacity” on page 421 and “Mask Shape and Opacity” on page 443),
                                or the name None if no such mask is speciﬁed. This parameter is
                                implicitly reset to its initial value at the beginning of execution of a
                                transparency group XObject (see Section 7.5.5, “Transparency
                                Group XObjects”). Initial value: None.
 */
    SkPdfSoftMaskDictionary* fSoftMaskDictionary;
    // TODO(edisonn): make sMask private, add setter and getter, ref/unref/..., at the moment we most likely leask
    SkBitmap*                fSMask;


/*
alpha constant    number        (PDF 1.4) The constant shape or constant opacity value to be used
                                in the transparent imaging model (see “Source Shape and Opacity”
                                on page 421 and “Constant Shape and Opacity” on page 444).
                                There are two separate alpha constant parameters: one for stroking
                                and one for all other painting operations. This parameter is implic-
                                itly reset to its initial value at the beginning of execution of a trans-
                                parency group XObject (see Section 7.5.5, “Transparency Group
                                XObjects”). Initial value: 1.0.
 */
    double fAphaConstant;

/*
alpha source      boolean       (PDF 1.4) A ﬂag specifying whether the current soft mask and alpha
                                constant parameters are to be interpreted as shape values (true) or
                                opacity values (false). This ﬂag also governs the interpretation of
                                the SMask entry, if any, in an image dictionary (see Section 4.8.4,
                                “Image Dictionaries”). Initial value: false.
 */
    bool fAlphaSource;


// TODO(edisonn): Device-dependent seem to be required only on the actual physical printer?
//                       TABLE 4.3 Device-dependent graphics state parameters
/*
overprint          boolean            (PDF 1.2) A ﬂag specifying (on output devices that support the
                                      overprint control feature) whether painting in one set of colorants
                                      should cause the corresponding areas of other colorants to be
                                      erased (false) or left unchanged (true); see Section 4.5.6, “Over-
                                      print Control.” In PDF 1.3, there are two separate overprint param-
                                      eters: one for stroking and one for all other painting operations.
                                      Initial value: false.
 */


/*
overprint mode     number             (PDF 1.3) A code specifying whether a color component value of 0
                                      in a DeviceCMYK color space should erase that component (0) or
                                      leave it unchanged (1) when overprinting (see Section 4.5.6, “Over-
                                      print Control”). Initial value: 0.
 */


/*
black generation   function           (PDF 1.2) A function that calculates the level of the black color
                   or name            component to use when converting RGB colors to CMYK (see Sec-
                                      tion 6.2.3, “Conversion from DeviceRGB to DeviceCMYK”). Initial
                                      value: installation-dependent.
 */


/*
undercolor removal function           (PDF 1.2) A function that calculates the reduction in the levels of
                   or name            the cyan, magenta, and yellow color components to compensate for
                                      the amount of black added by black generation (see Section 6.2.3,
                                      “Conversion from DeviceRGB to DeviceCMYK”). Initial value: in-
                                      stallation-dependent.
 */


/*
transfer           function,          (PDF 1.2) A function that adjusts device gray or color component
                   array, or name     levels to compensate for nonlinear response in a particular out-
                                      put device (see Section 6.3, “Transfer Functions”). Initial value:
                                      installation-dependent.
 */


/*
halftone           dictionary,        (PDF 1.2) A halftone screen for gray and color rendering, speciﬁed
                   stream, or name    as a halftone dictionary or stream (see Section 6.4, “Halftones”).
                                      Initial value: installation-dependent.
 */


/*
ﬂatness            number             The precision with which curves are to be rendered on the output
                                      device (see Section 6.5.1, “Flatness Tolerance”). The value of this
                                      parameter gives the maximum error tolerance, measured in output
                                      device pixels; smaller numbers give smoother curves at the expense
                                      of more computation and memory use. Initial value: 1.0.
 */


/*
smoothness             number             (PDF 1.3) The precision with which color gradients are to be ren-
                                          dered on the output device (see Section 6.5.2, “Smoothness Toler-
                                          ance”). The value of this parameter gives the maximum error
                                          tolerance, expressed as a fraction of the range of each color compo-
                                          nent; smaller numbers give smoother color transitions at the
                                          expense of more computation and memory use. Initial value:
                                          installation-dependent.
 */

    // TODO(edisonn): some defaults are contextual, they could on colorspace, pdf version, ...
    SkPdfGraphicsState() {
        fCurPosX      = 0.0;
        fCurPosY      = 0.0;
        fCurFontSize  = 0.0;
        fTextBlock    = false;
        fCTM          = SkMatrix::I();
        fMatrixTm     = SkMatrix::I();
        fMatrixTlm    = SkMatrix::I();
        fPathClosed   = true;
        fLineWidth    = 0;
        fTextLeading  = 0;
        fWordSpace    = 0;
        fCharSpace    = 0;
        fHasClipPathToApply = false;
        fResources    = NULL;
        fSkFont       = NULL;
        fLineCap      = 0;
        fLineJoin     = 0;
        fMiterLimit   = 10.0;
        fAphaConstant = 1.0;
        fAlphaSource  = false;
        fDashArrayLength = 0;
        fDashPhase    = 0;
        fBlendModesLength = 1;
        fBlendModes[0] = SkXfermode::kSrc_Mode;  // PDF: Normal Blend mode
        fSMask        = NULL;
    }

    // TODO(edisonn): make two functions instead, stroking and non stoking, avoid branching
    void applyGraphicsState(SkPaint* paint, bool stroking);
};

#endif  // SkPdfGraphicsState_DEFINED
