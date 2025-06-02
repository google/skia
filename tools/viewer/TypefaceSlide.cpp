/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkContourMeasure.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/base/SkRandom.h"
#include "tools/SkMetaData.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

namespace {

class TypefaceSlide : public Slide {
public:
    TypefaceSlide() { fName = "Typeface Viewer"; }

    void load(SkScalar w, SkScalar h) override {
        fWindowSize = {w, h};

        fBaseTypeface = ToolUtils::CreateTypefaceFromResource("fonts/Variable.ttf");
        fCurrentTypeface = fBaseTypeface;
        fVariationSliders = ToolUtils::VariationSliders(fCurrentTypeface.get(), fVariationPosition);

        fPathDirectionIndicator.reset();
        fPathDirectionIndicator.moveTo(0, -3);
        fPathDirectionIndicator.lineTo(3, 0);
        fPathDirectionIndicator.lineTo(0, 3);
        fPathDirectionIndicator.close();

        fPathDirectionIndicatorPaint.setColor(SK_ColorRED);
        fPathDirectionIndicatorPaint.setStroke(true);
        fPathDirectionIndicatorPaint.setStrokeWidth(0);
    }

    void resize(SkScalar w, SkScalar h) override { fWindowSize = {w, h}; }

    void unload() override {
        fBaseTypeface = nullptr;
        fCurrentTypeface = nullptr;
    }

    SkISize getDimensions() const override {
        if (fDrawArea.isEmpty()) {
            TypefaceSlide& self = *const_cast<TypefaceSlide*>(this);
            SkNoDrawCanvas noDraw(fWindowSize.width(), fWindowSize.height());
            self.draw(&noDraw);
        }
        return fDrawArea;
    }

    void updateCurrentTypeface() {
        if (!fBaseTypeface) {
            return;
        }
        SkSpan<const SkFontArguments::VariationPosition::Coordinate> coords =
                fVariationSliders.getCoordinates();
        SkFontArguments::VariationPosition varPos = {coords.data(),
                                                     static_cast<int>(coords.size())};
        SkFontArguments args;
        args.setVariationDesignPosition(varPos);
        fCurrentTypeface = fBaseTypeface->makeClone(args);
        fCurrentTypefaceDirty = false;
    }

    void draw(SkCanvas* canvas) override {
        SkPaint notationPaint;
        notationPaint.setColor(SK_ColorRED);
        SkFont notationFont(ToolUtils::DefaultTypeface(), 8.0);

        SkPaint paint;
        if (fOutline) {
            paint.setStroke(true);
            paint.setStrokeWidth(0);
        }
        if (fCurrentTypefaceDirty) {
            this->updateCurrentTypeface();
        }
        SkFont font(fCurrentTypeface, fFontSize);
        SkFontMetrics metrics;
        font.getMetrics(&metrics);

        SkPoint origin{0, 0};
        SkPoint position{0, 0};
        SkRect drawBounds = SkRect::MakeXYWH(0, 0, 1, 1);
        struct Line {
            SkRect bounds;
            SkGlyphID firstGlyph; // inclusive
            SkGlyphID lastGlyph; // inclusive
            int number;
        } line{{}, fCurrentGlyphID, fCurrentGlyphID, 0};

        int numGlyphs = fCurrentTypeface->countGlyphs();
        if (numGlyphs == 0) {
            fDrawArea.setEmpty();
            return;
        }
        SkGlyphID lastGlyph = numGlyphs - 1;
        for (SkGlyphID glyph = line.firstGlyph; line.number < 2; ++glyph) {
            // measure a line
            SkRect beginLineGlyphBounds;
            {
                SkRect newLineBounds = line.bounds;
                if (glyph != line.firstGlyph) {
                    newLineBounds.fRight += 10;
                }
                SkScalar advance;
                SkRect glyphBounds;
                font.getWidthsBounds({&glyph, 1}, {&advance, 1}, {&glyphBounds, 1}, nullptr);
                SkRect advanceBounds = SkRect::MakeWH(advance, 1);
                SkRect glyphAndAdvanceBounds = glyphBounds;
                glyphAndAdvanceBounds.join(advanceBounds);
                beginLineGlyphBounds = glyphAndAdvanceBounds;
                beginLineGlyphBounds.offset(-beginLineGlyphBounds.fLeft, 0);

                SkRect glyphDrawBounds = beginLineGlyphBounds.makeOffset(newLineBounds.right(), 0);
                if (line.number == -1) {
                    SkPaint p;
                    p.setColor(SK_ColorBLUE);
                    canvas->drawRect(glyphDrawBounds, p);
                }
                newLineBounds.join(glyphDrawBounds);
                if (newLineBounds.width() < fWindowSize.width()) {
                    line.lastGlyph = glyph;
                    line.bounds = newLineBounds;
                    if (glyph != lastGlyph) {
                        continue;
                    }
                }
            }

            // draw the line
            position.fY -= line.bounds.top();
            for (SkGlyphID gid = line.firstGlyph; gid <= line.lastGlyph; ++gid) {
                if (gid != line.firstGlyph) {
                    position.fX += 10;
                }
                SkScalar advance;
                SkRect glyphBounds;
                font.getWidthsBounds({&gid, 1}, {&advance, 1}, {&glyphBounds, 1}, nullptr);
                SkRect advanceBounds = SkRect::MakeWH(advance, 1);
                SkRect glyphAndAdvanceBounds = glyphBounds;
                glyphAndAdvanceBounds.join(advanceBounds);

                position.fX -= glyphAndAdvanceBounds.left();

                if (fDrawGlyphMetrics) {
                    SkPaint metricPaint;
                    metricPaint.setColor(SK_ColorRED);
                    canvas->drawRect(glyphBounds.makeOffset(position), metricPaint);
                    metricPaint.setColor(SK_ColorGREEN);
                    canvas->drawRect(advanceBounds.makeOffset(position), metricPaint);
                }

                canvas->drawGlyphs({&gid, 1}, {&position, 1}, origin, font, paint);

                if (fGlyphNumbers) {
                    SkString gidStr;
                    gidStr.appendS32(gid);
                    canvas->drawSimpleText(gidStr.c_str(), gidStr.size(), SkTextEncoding::kUTF8,
                                           position.x(), position.y(), notationFont, notationPaint);
                }
                // TODO: also handle drawable by using a paint override canvas?
                SkPath glyphPath;
                if (fOutline && font.getPath(gid, &glyphPath)) {
                    SkContourMeasureIter iter(glyphPath, false);
                    sk_sp<SkContourMeasure> contour;
                    int contourIndex = 0;
                    while ((contour = iter.next())) {
                        SkPoint contourStart;
                        SkVector tangent;
                        if (contour->getPosTan(0, &contourStart, &tangent)) {
                            contourStart += position;
                            SkAutoCanvasRestore acr(canvas, true);
                            SkMatrix matrix;
                            matrix.setSinCos(tangent.y(), tangent.x(), 0, 0);
                            matrix.postTranslate(contourStart.x(), contourStart.y());
                            canvas->concat(matrix);

                            if (fOutlineContourNumbers) {
                                SkString contourStr;
                                contourStr.appendS32(contourIndex);
                                canvas->drawSimpleText(contourStr.c_str(), contourStr.size(),
                                                       SkTextEncoding::kUTF8, 0, 0,
                                                       notationFont, notationPaint);
                            } else {
                                canvas->drawPath(fPathDirectionIndicator, fPathDirectionIndicatorPaint);
                            }

                            ++contourIndex;
                        }
                    }
                }

                position.fX += glyphAndAdvanceBounds.right();
            }
            if (line.lastGlyph == lastGlyph) {
                break;
            }
            drawBounds.join(line.bounds.makeOffset(-line.bounds.fLeft, position.fY));
            position.fX = 0;
            position.fY += line.bounds.bottom() + 10;
            line.bounds = beginLineGlyphBounds;
            line.firstGlyph = glyph;
            line.lastGlyph = glyph;
            ++line.number;
        }

        fDrawArea = drawBounds.roundOut().size();
    }

    bool onGetControls(SkMetaData* controls) override {
        // requested font size
        SkScalar size[3] = {fFontSize, 0, 256};
        controls->setScalars("Size", 3, size);

        // the first glyph on the first line
        SkScalar glyph[3] = {SkScalar(fCurrentGlyphID), 0, SkScalar(this->fCurrentTypeface->countGlyphs())};
        controls->setScalars("Glyph", 3, glyph);

        // TODO: limit number of glyphs
        // TODO: choose typeface factory
        // TODO: choose between typefaces
        // TODO: font metrics like underline, strikeout, x-height, cap-height, etc.

        // show glyph metrics like advances and bounds
        controls->setBool("Glyph Metrics", fDrawGlyphMetrics);

        // Show glyph numbers at origin
        controls->setBool("Glyph numbers", fGlyphNumbers);

        // hairline contours with initial direction mark
        controls->setBool("Outline", fOutline);

        // draw contour numbers instead of initial contour direction mark
        controls->setBool("Outline contour numbers", fOutlineContourNumbers);

        return fVariationSliders.writeControls(controls);
    }

    void onSetControls(const SkMetaData& controls) override {
        SkScalar size[3] = {0};
        int numReturnedScalars = 0;
        SkASSERT_RELEASE(controls.findScalars("Size", &numReturnedScalars, size));
        SkASSERT_RELEASE(numReturnedScalars == 3);
        if (fFontSize != size[0]) {
            fFontSize = size[0];
            fDrawArea.setEmpty();
        }

        SkScalar glyph[3] = {0};
        numReturnedScalars = 0;
        SkASSERT_RELEASE(controls.findScalars("Glyph", &numReturnedScalars, glyph));
        SkASSERT_RELEASE(numReturnedScalars == 3);
        if (fCurrentGlyphID != SkScalarRoundToInt(glyph[0])) {
            fCurrentGlyphID = SkScalarRoundToInt(glyph[0]);
            fDrawArea.setEmpty();
        }

        controls.findBool("Glyph Metrics", &fDrawGlyphMetrics);
        controls.findBool("Glyph numbers", &fGlyphNumbers);
        controls.findBool("Outline", &fOutline);
        controls.findBool("Outline contour numbers", &fOutlineContourNumbers);

        fVariationSliders.readControls(controls, &fCurrentTypefaceDirty);
    }

private:
    sk_sp<SkTypeface> fBaseTypeface;
    sk_sp<SkTypeface> fCurrentTypeface;

    std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> fCoordinates;
    SkFontArguments::VariationPosition fVariationPosition;
    ToolUtils::VariationSliders fVariationSliders;
    bool fCurrentTypefaceDirty = true;
    SkScalar fFontSize = 80;
    bool fOutline = false;
    bool fOutlineContourNumbers = false;
    bool fGlyphNumbers = false;
    bool fDrawGlyphMetrics = false;
    SkGlyphID fCurrentGlyphID = 0;

    SkSize fWindowSize;
    SkISize fDrawArea;

    SkPath fPathDirectionIndicator;
    SkPaint fPathDirectionIndicatorPaint;
};

}  //namespace

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new TypefaceSlide(); )
