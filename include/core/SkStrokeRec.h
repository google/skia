/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrokeRec_DEFINED
#define SkStrokeRec_DEFINED

#include "SkPaint.h"

class SkPath;

class SkStrokeRec {
public:
    enum InitStyle {
        kHairline_InitStyle,
        kFill_InitStyle
    };
    SkStrokeRec(InitStyle style);

    SkStrokeRec(const SkStrokeRec&);
    explicit SkStrokeRec(const SkPaint&);

    enum Style {
        kHairline_Style,
        kFill_Style,
        kStroke_Style,
        kStrokeAndFill_Style
    };

    Style getStyle() const;
    SkScalar getWidth() const { return fWidth; }
    SkScalar getMiter() const { return fMiterLimit; }
    SkPaint::Cap getCap() const { return fCap; }
    SkPaint::Join getJoin() const { return fJoin; }

    bool isHairlineStyle() const {
        return kHairline_Style == this->getStyle();
    }

    bool isFillStyle() const {
        return kFill_Style == this->getStyle();
    }

    void setFillStyle();
    void setHairlineStyle();
    /**
     *  Specify the strokewidth, and optionally if you want stroke + fill.
     *  Note, if width==0, then this request is taken to mean:
     *      strokeAndFill==true -> new style will be Fill
     *      strokeAndFill==false -> new style will be Hairline
     */
    void setStrokeStyle(SkScalar width, bool strokeAndFill = false);

    void setStrokeParams(SkPaint::Cap cap, SkPaint::Join join, SkScalar miterLimit) {
        fCap = cap;
        fJoin = join;
        fMiterLimit = miterLimit;
    }

    /**
     *  Returns true if this specifes any thick stroking, i.e. applyToPath()
     *  will return true.
     */
    bool needToApply() const {
        Style style = this->getStyle();
        return (kStroke_Style == style) || (kStrokeAndFill_Style == style);
    }

    /**
     *  Apply these stroke parameters to the src path, returning the result
     *  in dst.
     *
     *  If there was no change (i.e. style == hairline or fill) this returns
     *  false and dst is unchanged. Otherwise returns true and the result is
     *  stored in dst.
     *
     *  src and dst may be the same path.
     */
    bool applyToPath(SkPath* dst, const SkPath& src) const;

    bool operator==(const SkStrokeRec& other) const {
            return fWidth == other.fWidth &&
                   fMiterLimit == other.fMiterLimit &&
                   fCap == other.fCap &&
                   fJoin == other.fJoin &&
                   fStrokeAndFill == other.fStrokeAndFill;
    }

private:
    SkScalar        fWidth;
    SkScalar        fMiterLimit;
    SkPaint::Cap    fCap;
    SkPaint::Join   fJoin;
    bool            fStrokeAndFill;
};

#endif
