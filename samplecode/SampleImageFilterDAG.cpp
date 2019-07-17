/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"

#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkColorFilterImageFilter.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDropShadowImageFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkMergeImageFilter.h"
#include "include/effects/SkOffsetImageFilter.h"

#include "src/core/SkImageFilterPriv.h"
#include "src/core/SkSpecialImage.h"

#include "tools/ToolUtils.h"

namespace {

struct FilterNode {
    // Pointer to the actual filter in the DAG, so it still contains its input filters and
    // may be used as an input in an earlier node. Null when this represents the "source" input
    sk_sp<SkImageFilter> fFilter;

    // FilterNodes wrapping each of fFilter's inputs. Leaf node when fInputNodes is empty.
    SkTArray<FilterNode> fInputNodes;

    // Distance from root filter
    int fDepth;

    // The source content rect (this is the same for all nodes, but is stored here for convenience)
    SkRect fContent;
    // The portion of the original CTM that is kept as the local matrix/ctm when filtering
    SkMatrix fLocalCTM;
    // The portion of the original CTM that the results should be drawn with (or given current
    // canvas impl., the portion of the CTM that is baked into a new DAG)
    SkMatrix fRemainingCTM;

    // Cached reverse bounds using device-space clip bounds (e.g. SkCanvas::clipRectBounds with
    // null first argument). This represents the layer calculated in SkCanvas for the filtering.
    // FIXME: SkCanvas (and this sample), currently do not transform the device clip bounds into the
    // filter local space, which is where it ought to be defined.
    SkIRect fLayerBounds;

    // Cached reverse bounds using the local draw bounds (e.g. SkCanvas::clipRectBounds with the
    // draw bounds provided as first argument). For intermediate nodes in a DAG, this is calculated
    // to match what the filter would compute when being evaluated as part of the original DAG.
    // fReverseLocalIsolatedBounds is the same, except it represents what would be calculated if
    // only this node were being applied as the image filter.
    SkIRect fReverseLocalBounds;
    SkIRect fReverseLocalIsolatedBounds;

    // Cached forward bounds based on local draw bounds. For intermediate nodes in a DAG, this is
    // calculated to match what the filter computes as part of the whole DAG. fForwardIsolatedBounds
    // is the same but represents what would be calculated if only this node were applied.
    SkIRect fForwardBounds;
    SkIRect fForwardIsolatedBounds;

    // Should be called after the input nodes have been created since this will complete the
    // entire tree.
    void computeBounds() {
        // In normal usage, forward bounds are filter-space bounds of the geometry content, so
        // fContent mapped by the local matrix, since we assume the layer content is made by
        // concat(localCTM) -> clipRect(content) -> drawRect(content).
        // Similarly, in normal usage, reverse bounds are the filter-space bounds of the space to
        // be filled by image filter results. Since the clip rect is set to the same as the content,
        // it's the same bounds forward or reverse in this contrived case.
        SkIRect inputRect;
        fLocalCTM.mapRect(fContent).roundOut(&inputRect);

        this->computeForwardBounds(inputRect);
        this->computeReverseLocalIsolatedBounds(inputRect);
        this->computeReverseBounds(inputRect, false);

        // The layer bounds (matching what SkCanvas computes), use the content rect mapped by the
        // entire CTM as its input rect.
        // FIXME (michaelludwig) - Once SkCanvas' filter handling is fixed, this should be
        // (remainder * local)
        SkIRect incorrectCanvasInputRect;
        SkMatrix ctm = SkMatrix::Concat(fLocalCTM, fRemainingCTM);
        ctm.mapRect(fContent).roundOut(&incorrectCanvasInputRect);
        this->computeReverseBounds(incorrectCanvasInputRect, true);
    }

private:
    void computeForwardBounds(const SkIRect srcRect) {
        if (fFilter) {
            // For forward filtering, the leaves of the DAG are evaluated first and are then
            // propagated to the root. This means that every filter's filterBounds() function sees
            // the original src rect. It is never dependent on the parent node (unlike reverse
            // filtering), so calling filterBounds() on an intermediate node gives us the correct
            // intermediate values.
            fForwardBounds = fFilter->filterBounds(
                    srcRect, fLocalCTM, SkImageFilter::kForward_MapDirection, nullptr);

            // For isolated forward filtering, it uses the same input but should not be propagated
            // to the inputs, so get the filter node bounds directly.
            fForwardIsolatedBounds = SkFilterNodeBounds(
                    fFilter.get(), srcRect, fLocalCTM,
                    SkImageFilter::kForward_MapDirection, nullptr);
        } else {
            fForwardBounds = srcRect;
            fForwardIsolatedBounds = srcRect;
        }

        // Fill in children
        for (int i = 0; i < fInputNodes.count(); ++i) {
            fInputNodes[i].computeForwardBounds(srcRect);
        }
    }

    void computeReverseLocalIsolatedBounds(const SkIRect& srcRect) {
        if (fFilter) {
            fReverseLocalIsolatedBounds = SkFilterNodeBounds(
                    fFilter.get(), srcRect, fLocalCTM,
                    SkImageFilter::kReverse_MapDirection, &srcRect);
        } else {
            fReverseLocalIsolatedBounds = srcRect;
        }

        // Fill in children. Unlike regular reverse bounds mapping, the input nodes see the original
        // bounds. Normally, the bounds that the child nodes see have already been mapped processed
        // by this node.
        for (int i = 0; i < fInputNodes.count(); ++i) {
            fInputNodes[i].computeReverseLocalIsolatedBounds(srcRect);
        }
    }

    // fReverseLocalBounds and fLayerBounds are computed the same, except they differ in what the
    // initial bounding rectangle was. It is assumed that the 'srcRect' has already been processed
    // by the parent node's onFilterNodeBounds() function, as in SkImageFilter::filterBounds().
    void computeReverseBounds(const SkIRect& srcRect, bool writeToLayerBounds) {
        SkIRect reverseBounds = srcRect;

        if (fFilter) {
            // Since srcRect has been through parent's onFilterNodeBounds(), calling filterBounds()
            // directly on this node will calculate the same rectangle that this filter would report
            // during the parent node's onFilterBounds() recursion.

            reverseBounds = fFilter->filterBounds(
                    srcRect, fLocalCTM, SkImageFilter::kReverse_MapDirection, &srcRect);

            // To calculate the appropriate intermediate reverse bounds for the children, we need
            // this node's onFilterNodeBounds() results based on its parents' bounds (the current
            // 'srcRect')
            SkIRect nextSrcRect = SkFilterNodeBounds(
                    fFilter.get(), srcRect, fLocalCTM,
                    SkImageFilter::kReverse_MapDirection, &srcRect);

            // Fill in the children. The union of these bounds should equal the value calculated
            // for reverseBounds already.
            SkDEBUGCODE(SkIRect netReverseBounds = SkIRect::MakeEmpty();)
            for (int i = 0; i < fInputNodes.count(); ++i) {
                fInputNodes[i].computeReverseBounds(nextSrcRect, writeToLayerBounds);
                SkDEBUGCODE(netReverseBounds.join(
                        writeToLayerBounds ? fInputNodes[i].fLayerBounds
                                           : fInputNodes[i].fReverseLocalBounds);)
            }
            SkASSERT(netReverseBounds == reverseBounds);
        }

        if (writeToLayerBounds) {
            fLayerBounds = reverseBounds;
        } else {
            fReverseLocalBounds = reverseBounds;
        }
    }
};

} // anonymous namespace

static FilterNode build_dag(const SkMatrix& local, const SkMatrix& remainder, const SkRect& rect,
                            const SkImageFilter* filter, int depth) {
    FilterNode node;
    node.fFilter = sk_ref_sp(filter);
    node.fDepth = depth;
    node.fContent = rect;

    node.fLocalCTM = local;
    node.fRemainingCTM = remainder;

    if (node.fFilter) {
        if (depth > 0) {
            // We don't visit children when at the root because the real child filters are replaced
            // with the internalSaveLayer decomposition emulation, which then cycles back to the
            // original filter but with an updated matrix (and then we process the children).
            node.fInputNodes.reserve(node.fFilter->countInputs());
            for (int i = 0; i < node.fFilter->countInputs(); ++i) {
                node.fInputNodes.push_back() =
                        build_dag(local, remainder, rect, node.fFilter->getInput(i), depth + 1);
            }
        }
    }

    return node;
}

static FilterNode build_dag(const SkMatrix& ctm, const SkRect& rect,
                            const SkImageFilter* rootFilter) {
    // Emulate SkCanvas::internalSaveLayer's decomposition of the CTM.
    SkMatrix local;
    sk_sp<SkImageFilter> finalFilter = SkApplyCTMToFilter(rootFilter, ctm, &local);

    // FIXME (michaelludwig)
    // decomposeScale computes scale * remaining, and SkApplyCTMToFilter puts the remaining
    // matrix into the DAG. This is actually incorrect, since the input content for filtering
    // should be transformed by the local matrix ('scale' in this case) first, and then the matrix
    // filter applies 'remaining'. Once they are applied properly, update this code to reflect
    // the proper matrix multiplication order (currently, this is written to match SkCanvas).
    SkMatrix invLocal, embedded;
    if (local.invert(&invLocal)) {
        // Currently, CTM = local * embedded, so embedded = local^-1 * CTM
        embedded = SkMatrix::Concat(invLocal, ctm);
    } else {
        embedded = SkMatrix::I();
    }

    // Create a root node that represents the full result
    FilterNode rootNode = build_dag(ctm, SkMatrix::I(), rect, rootFilter, 0);
    // Set its only child as the modified DAG that handles the CTM decomposition
    rootNode.fInputNodes.push_back() =
            build_dag(local, embedded, rect, finalFilter.get(), 1);
    // Fill in bounds information that requires the entire node DAG to have been extracted first.
    rootNode.fInputNodes[0].computeBounds();
    return rootNode;
}

static void draw_node(SkCanvas* canvas, const FilterNode& node) {
    canvas->clear(SK_ColorTRANSPARENT);

    SkPaint filterPaint;
    filterPaint.setImageFilter(node.fFilter);

    SkPaint paint;
    static const SkColor kColors[2] = {SK_ColorGREEN, SK_ColorWHITE};
    SkPoint points[2] = { {node.fContent.fLeft + 15.f, node.fContent.fTop + 15.f},
                          {node.fContent.fRight - 15.f, node.fContent.fBottom - 15.f} };
    paint.setShader(SkGradientShader::MakeLinear(points, kColors, nullptr, SK_ARRAY_COUNT(kColors),
                                                 SkTileMode::kRepeat));

    SkPaint line;
    line.setStrokeWidth(1.f);
    line.setStyle(SkPaint::kStroke_Style);

    if (node.fDepth == 0) {
        // The root node, so draw this one the canonical way through SkCanvas to show current
        // net behavior. Will not include bounds visualization.
        canvas->save();
        canvas->concat(node.fLocalCTM);
        SkASSERT(node.fRemainingCTM.isIdentity());

        canvas->clipRect(node.fContent, /* aa */ true);
        canvas->saveLayer(nullptr, &filterPaint);
        canvas->drawRect(node.fContent, paint);
        canvas->restore(); // Completes the image filter
        canvas->restore(); // Undoes matrix and clip
    } else {
        canvas->save();
        canvas->concat(node.fLocalCTM);

        canvas->saveLayer(nullptr, &filterPaint);
        canvas->drawRect(node.fContent, paint);
        canvas->restore(); // Completes the image filter

        // Draw content-rect bounds
        line.setColor(SK_ColorBLACK);
        canvas->drawRect(node.fContent, line);
        canvas->restore(); // Undoes the matrix

        // Bounding boxes have all been mapped by the local matrix already, so drawing them with
        // the identity CTM should align everything
        canvas->save();
        canvas->resetMatrix();

        line.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::Make(node.fLayerBounds).makeOutset(5.f, 5.f), line);

        line.setColor(SK_ColorMAGENTA);
        canvas->drawRect(SkRect::Make(node.fReverseLocalBounds).makeOutset(4.f, 4.f), line);
        line.setColor(SK_ColorBLUE);
        canvas->drawRect(SkRect::Make(node.fForwardBounds).makeOutset(2.f, 2.f), line);

        // Dashed lines for the isolated shapes
        static const SkScalar kDashParams[] = {6.f, 12.f};
        line.setPathEffect(SkDashPathEffect::Make(kDashParams, 2, 0.f));

        line.setColor(SK_ColorMAGENTA);
        canvas->drawRect(SkRect::Make(node.fReverseLocalIsolatedBounds).makeOutset(3.f, 3.f), line);
        line.setColor(SK_ColorBLUE);
        canvas->drawRect(SkRect::Make(node.fForwardIsolatedBounds).makeOutset(1.f, 1.f), line);

        canvas->restore();
    }
}

static constexpr float kLineHeight = 16.f;
static constexpr float kLineInset = 8.f;

static float print_matrix(SkCanvas* canvas, const char* prefix, const SkMatrix& matrix,
                         float x, float y, const SkFont& font, const SkPaint& paint) {
    canvas->drawString(prefix, x, y, font, paint);
    y += kLineHeight;
    for (int i = 0; i < 3; ++i) {
        SkString row;
        row.appendf("[%.2f %.2f %.2f]",
                    matrix.get(i * 3), matrix.get(i * 3 + 1), matrix.get(i * 3 + 2));
        canvas->drawString(row, x, y, font, paint);
        y += kLineHeight;
    }
    return y;
}

static float print_size(SkCanvas* canvas, const char* prefix, const SkIRect& rect,
                       float x, float y, const SkFont& font, const SkPaint& paint) {
    canvas->drawString(prefix, x, y, font, paint);
    y += kLineHeight;
    SkString sz;
    sz.appendf("%d x %d", rect.width(), rect.height());
    canvas->drawString(sz, x, y, font, paint);
    return y + kLineHeight;
}

static float print_info(SkCanvas* canvas, const FilterNode& node) {
    SkFont font(nullptr, 12);
    SkPaint text;
    text.setAntiAlias(true);

    float y = kLineHeight;
    if (node.fDepth == 0) {
        canvas->drawString("Final Results", kLineInset, y, font, text);
        // The actual interesting matrices are in the root node's first child
        y = print_matrix(canvas, "Local", node.fInputNodes[0].fLocalCTM,
                     kLineInset, y + kLineHeight, font, text);
        y = print_matrix(canvas, "Embedded", node.fInputNodes[0].fRemainingCTM,
                     kLineInset, y, font, text);
    } else if (node.fFilter) {
        canvas->drawString(node.fFilter->getTypeName(), kLineInset, y, font, text);
        print_size(canvas, "Layer Size", node.fLayerBounds, kLineInset, y + kLineHeight,
                   font, text);
        y = print_size(canvas, "Ideal Size", node.fReverseLocalBounds, 10 * kLineInset,
                       y + kLineHeight, font, text);
    } else {
        canvas->drawString("Source Input", kLineInset, kLineHeight, font, text);
        y += kLineHeight;
    }

    return y;
}

// Returns bottom edge in pixels that the subtree reached in canvas
static float draw_dag(SkCanvas* canvas, SkSurface* nodeSurface, const FilterNode& node) {
    // First capture the results of the node, into nodeSurface
    draw_node(nodeSurface->getCanvas(), node);
    sk_sp<SkImage> nodeResults = nodeSurface->makeImageSnapshot();

    // Fill in background of the filter node with a checkerboard
    canvas->save();
    canvas->clipRect(SkRect::MakeWH(nodeResults->width(), nodeResults->height()));
    ToolUtils::draw_checkerboard(canvas, SK_ColorGRAY, SK_ColorLTGRAY, 10);
    canvas->restore();

    // Display filtered results in current canvas' location (assumed CTM is set for this node)
    canvas->drawImage(nodeResults, 0, 0);

    SkPaint line;
    line.setAntiAlias(true);
    line.setStyle(SkPaint::kStroke_Style);
    line.setStrokeWidth(3.f);

    // Text info
    canvas->save();
    canvas->translate(0, nodeResults->height());
    float textHeight = print_info(canvas, node);
    canvas->restore();

    // Border around filtered results + text info
    canvas->drawRect(SkRect::MakeWH(nodeResults->width(), nodeResults->height() + textHeight),
                     line);

    static const float kPad = 20.f;
    float x = nodeResults->width() + kPad;
    float y = 0;
    for (int i = 0; i < node.fInputNodes.count(); ++i) {
        // Line connecting this node to its child
        canvas->drawLine(nodeResults->width(), 0.5f * nodeResults->height(), // right of node
                         x, y + 0.5f * nodeResults->height(), line);         // left of child
        canvas->save();
        canvas->translate(x, y);
        y = draw_dag(canvas, nodeSurface, node.fInputNodes[i]);
        canvas->restore();
    }
    return SkMaxScalar(y, nodeResults->height() + textHeight + kPad);
}

static void draw_dag(SkCanvas* canvas, sk_sp<SkImageFilter> filter,
                     const SkRect& rect, const SkISize& surfaceSize) {
    // Get the current CTM, which includes all the viewer's UI modifications, which we want to
    // pass into our mock canvases for each DAG node.
    SkMatrix ctm = canvas->getTotalMatrix();

    canvas->save();
    // Reset the matrix so that the DAG layout and instructional text is fixed to the window.
    canvas->resetMatrix();

    // Process the image filter DAG to display intermediate results later on, which will apply the
    // provided CTM during draw_node calls.
    FilterNode dag = build_dag(ctm, rect, filter.get());

    sk_sp<SkSurface> nodeSurface = canvas->makeSurface(
            canvas->imageInfo().makeWH(surfaceSize.width(), surfaceSize.height()));
    draw_dag(canvas, nodeSurface.get(), dag);

    canvas->restore();
}

class ImageFilterDAGSample : public Sample {
public:
    ImageFilterDAGSample() {}

    void onDrawContent(SkCanvas* canvas) override {
        static const SkRect kFilterRect = SkRect::MakeXYWH(20.f, 20.f, 60.f, 60.f);
        static const SkISize kFilterSurfaceSize = SkISize::Make(
                2 * (kFilterRect.fRight + kFilterRect.fLeft),
                2 * (kFilterRect.fBottom + kFilterRect.fTop));

        // Somewhat clunky, but we want to use the viewer calculated CTM in the mini surfaces used
        // per DAG node. The rotation matrix viewer calculates is based on the sample size so trick
        // it into calculating the right matrix for us w/ 1 frame latency.
        this->setSize(kFilterSurfaceSize.width(), kFilterSurfaceSize.height());

        // Make a large DAG
        //        /--- Color Filter <---- Blur <--- Offset
        // Merge <
        //        \--- Blur <--- Drop Shadow
        sk_sp<SkImageFilter> drop2 = SkDropShadowImageFilter::Make(
                10.f, 5.f, 3.f, 3.f, SK_ColorBLACK,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, nullptr);
        sk_sp<SkImageFilter> blur1 = SkBlurImageFilter::Make(2.f, 2.f, std::move(drop2));

        sk_sp<SkImageFilter> offset3 = SkOffsetImageFilter::Make(-5.f, -5.f, nullptr);
        sk_sp<SkImageFilter> blur2 = SkBlurImageFilter::Make(4.f, 4.f, std::move(offset3));
        sk_sp<SkImageFilter> cf1 = SkColorFilterImageFilter::Make(
                SkColorFilters::Blend(SK_ColorGRAY, SkBlendMode::kModulate), std::move(blur2));

        sk_sp<SkImageFilter> merge0 = SkMergeImageFilter::Make(std::move(blur1), std::move(cf1));

        draw_dag(canvas, std::move(merge0), kFilterRect, kFilterSurfaceSize);
    }

    SkString name() override { return SkString("ImageFilterDAG"); }

private:

    typedef Sample INHERITED;
};

DEF_SAMPLE(return new ImageFilterDAGSample();)
