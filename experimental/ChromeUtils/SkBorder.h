/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBorder_DEFINED
#define SkBorder_DEFINED

#include "SkColor.h"
#include "SkPaint.h"
#include "SkScalar.h"
#include "SkTArray.h"

// This class provides a concise means of specifying all the geometry/shading
// associated with a CSS-style box/round-rect.
class SkBorder {
public:
    enum BorderStyle {
        /**
        */
        kNone_BorderStyle,
        /**
        */
        kHidden_BorderStyle,
        /**
        */
        kDotted_BorderStyle,
        /**
        */
        kDashed_BorderStyle,
        /**
        */
        kSolid_BorderStyle,
        /**
        */
        kDouble_BorderStyle,
        /**
        */
        kGroove_BorderStyle,
        /**
        */
        kRidge_BorderStyle,
        /**
        */
        kInset_BorderStyle,
        /**
        */
        kOutset_BorderStyle,
    };

    enum BlurStyle {
        kNormal_BlurStyle,  //!< fuzzy inside and outside
        kInner_BlurStyle,   //!< fuzzy inside, nothing outside
    };

    struct ShadowInfo {
        SkScalar  fXOffset;
        SkScalar  fYOffset;
        SkScalar  fBlurSigma;
        SkColor   fColor;
        BlurStyle fStyle;
    };

    SkBorder(SkPaint& p, SkScalar width, BorderStyle style);

    SkBorder(const SkPaint paints[4], const SkScalar widths[4], const BorderStyle styles[4]);

    void setBackground(SkPaint* p) {
        if (NULL == p) {
            fBackground.reset();
            fFlags &= ~kDrawBackground_Flag;
        } else {
            fBackground = *p;
            fFlags |= kDrawBackground_Flag;
        }
    }

    void addShadow(ShadowInfo& info) {
        fShadows.push_back(info);
    }

private:
    enum Flags {
        // One paint "fPaints[0]" is applied to all the borders
        kOnePaint_Flag = 0x01,
        // Use 'fBackground' to draw the background
        kDrawBackground_Flag = 0x02,
    };

    // If kOnePaint_Flag is specified then fBorder[0] is applied to all sides.
    // Otherwise the order is: left, top, right, bottom
    SkPaint              fPaints[4];
    // Only valid if kDrawBackground_Flag is set.
    SkPaint              fBackground;
    SkScalar             fWidths[4];
    BorderStyle          fStyles[4];
    SkTArray<ShadowInfo> fShadows;
    uint32_t             fFlags;
};

#endif
