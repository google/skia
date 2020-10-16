/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"

#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

#include "src/core/SkImageFilter_Base.h"
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
    skif::ParameterSpace<SkRect> fContent;
    // The mapping for the filter dag (same for all nodes, but stored here for convenience)
    skif::Mapping fMapping;

    // Cached reverse bounds using device-space clip bounds (e.g. no local bounds hint passed to
    // saveLayer). This represents the layer calculated in SkCanvas for the filtering.
    skif::LayerSpace<SkIRect> fUnhintedLayerBounds;

    // Cached input bounds using the local draw bounds (e.g. saveLayer with a bounds rect, or
    // an auto-layer for a draw with image filter). This represents the layer bounds up to this
    // point of the DAG.
    skif::LayerSpace<SkIRect> fHintedLayerBounds;

    // Cached output bounds based on local draw bounds. This represents the output up to this
    // point of the DAG.
    skif::LayerSpace<SkIRect> fOutputBounds;

    FilterNode(const SkImageFilter* filter,
               const skif::Mapping& mapping,
               const skif::ParameterSpace<SkRect>& content,
               int depth)
            : fFilter(sk_ref_sp(filter))
            , fDepth(depth)
            , fContent(content)
            , fMapping(mapping) {
        this->computeInputBounds();
        this->computeOutputBounds();
        if (fFilter) {
            fInputNodes.reserve_back(fFilter->countInputs());
            for (int i = 0; i < fFilter->countInputs(); ++i) {
                fInputNodes.emplace_back(fFilter->getInput(i), mapping, content, depth + 1);
            }
        }
    }

private:
    void computeOutputBounds() {
        if (fFilter) {
            // For visualization purposes, we want the output bounds in layer space, before it's
            // been transformed to device space. To achieve that, we mock a new mapping with the
            // identity matrix transform.
            skif::Mapping layerOnly = skif::Mapping(SkMatrix::I(), fMapping.layerMatrix());
            skif::DeviceSpace<SkIRect> pseudoDeviceBounds =
                    as_IFB(fFilter)->getOutputBounds(layerOnly, fContent);
            // Since layerOnly's device matrix is I, this is effectively a cast to layer space
            fOutputBounds = layerOnly.deviceToLayer(pseudoDeviceBounds);
        } else {
            fOutputBounds = fMapping.paramToLayer(fContent).roundOut();
        }

        // Fill in children
        for (int i = 0; i < fInputNodes.count(); ++i) {
            fInputNodes[i].computeOutputBounds();
        }
    }

    void computeInputBounds() {
        // As a proxy for what the base device had, use the content rect mapped to device space
        // (e.g. clipRect() was called with the same coords prior to the draw).
        skif::DeviceSpace<SkIRect> targetOutput(fMapping.totalMatrix()
                                                        .mapRect(SkRect(fContent))
                                                        .roundOut());

        if (fFilter) {
            fHintedLayerBounds = as_IFB(fFilter)->getInputBounds(fMapping, targetOutput, &fContent);
            fUnhintedLayerBounds = as_IFB(fFilter)->getInputBounds(fMapping, targetOutput, nullptr);
        } else {
            fHintedLayerBounds = fMapping.paramToLayer(fContent).roundOut();
            fUnhintedLayerBounds = fMapping.deviceToLayer(targetOutput);
        }
    }
};

} // anonymous namespace

static FilterNode build_dag(const SkMatrix& ctm, const SkRect& rect,
                            const SkImageFilter* rootFilter) {
    // Emulate SkCanvas::internalSaveLayer's decomposition of the CTM.
    skif::ParameterSpace<SkRect> content(rect);
    skif::ParameterSpace<SkPoint> center({rect.centerX(), rect.centerY()});
    skif::Mapping mapping = skif::Mapping::DecomposeCTM(ctm, rootFilter, center);
    return FilterNode(rootFilter, mapping, content, 0);
}

static void draw_node(SkCanvas* canvas, const FilterNode& node) {
    canvas->clear(SK_ColorTRANSPARENT);

    SkPaint filterPaint;
    filterPaint.setImageFilter(node.fFilter);

    SkRect content = SkRect(node.fContent);
    SkPaint paint;
    static const SkColor kColors[2] = {SK_ColorGREEN, SK_ColorWHITE};
    SkPoint points[2] = { {content.fLeft + 15.f, content.fTop + 15.f},
                          {content.fRight - 15.f, content.fBottom - 15.f} };
    paint.setShader(SkGradientShader::MakeLinear(points, kColors, nullptr, SK_ARRAY_COUNT(kColors),
                                                 SkTileMode::kRepeat));

    SkPaint line;
    line.setStrokeWidth(0.f);
    line.setStyle(SkPaint::kStroke_Style);

    canvas->save();
    canvas->concat(node.fMapping.deviceMatrix());
    canvas->save();
    canvas->concat(node.fMapping.layerMatrix());

    canvas->saveLayer(&content, &filterPaint);
    canvas->drawRect(content, paint);
    canvas->restore(); // Completes the image filter

    // Draw content-rect bounds
    line.setColor(SK_ColorBLACK);
    canvas->drawRect(content, line);

    // Bounding boxes have all been mapped by the layer matrix from local to layer space, so undo
    // the layer matrix, leaving just the device matrix.
    canvas->restore();

    // The hinted bounds of the layer saved for the filtering
    line.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::Make(SkIRect(node.fHintedLayerBounds)).makeOutset(3.f, 3.f), line);
    // The bounds of the layer if there was no local content hint
    line.setColor(SK_ColorGREEN);
    canvas->drawRect(SkRect::Make(SkIRect(node.fUnhintedLayerBounds)).makeOutset(2.f, 2.f), line);

    // The output bounds in layer space
    line.setColor(SK_ColorBLUE);
    canvas->drawRect(SkRect::Make(SkIRect(node.fOutputBounds)).makeOutset(1.f, 1.f), line);
    // Device-space bounding box of the output bounds (e.g. what legacy DAG manipulation via
    // MatrixTransform would produce).
    static const SkScalar kDashParams[] = {6.f, 12.f};
    line.setPathEffect(SkDashPathEffect::Make(kDashParams, 2, 0.f));
    SkRect devOutputBounds = SkRect::Make(SkIRect(node.fMapping.layerToDevice(node.fOutputBounds)));
    canvas->restore(); // undoes device matrix
    canvas->drawRect(devOutputBounds, line);
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
    if (node.fFilter) {
        canvas->drawString(node.fFilter->getTypeName(), kLineInset, y, font, text);
        y += kLineHeight;
        if (node.fDepth == 0) {
            // The mapping is the same for all nodes, so only print at the root
            y = print_matrix(canvas, "Param->Layer", node.fMapping.layerMatrix(),
                        kLineInset, y, font, text);
            y = print_matrix(canvas, "Layer->Device", node.fMapping.deviceMatrix(),
                        kLineInset, y, font, text);
        }

        y = print_size(canvas, "Layer Size", SkIRect(node.fUnhintedLayerBounds),
                       kLineInset, y, font, text);
        y = print_size(canvas, "Layer Size (hinted)", SkIRect(node.fHintedLayerBounds),
                       kLineInset, y, font, text);
    } else {
        canvas->drawString("Source Input", kLineInset, y, font, text);
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
    return std::max(y, nodeResults->height() + textHeight + kPad);
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

    sk_sp<SkSurface> nodeSurface =
            canvas->makeSurface(canvas->imageInfo().makeDimensions(surfaceSize));
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
        sk_sp<SkImageFilter> drop2 = SkImageFilters::DropShadow(
                10.f, 5.f, 3.f, 3.f, SK_ColorBLACK, nullptr);
        sk_sp<SkImageFilter> blur1 = SkImageFilters::Blur(2.f, 2.f, std::move(drop2));

        sk_sp<SkImageFilter> offset3 = SkImageFilters::Offset(-5.f, -5.f, nullptr);
        sk_sp<SkImageFilter> blur2 = SkImageFilters::Blur(4.f, 4.f, std::move(offset3));
        sk_sp<SkImageFilter> cf1 = SkImageFilters::ColorFilter(
                SkColorFilters::Blend(SK_ColorGRAY, SkBlendMode::kModulate), std::move(blur2));

        sk_sp<SkImageFilter> merge0 = SkImageFilters::Merge(std::move(blur1), std::move(cf1));

        draw_dag(canvas, std::move(merge0), kFilterRect, kFilterSurfaceSize);
    }

    SkString name() override { return SkString("ImageFilterDAG"); }

private:

    using INHERITED = Sample;
};

DEF_SAMPLE(return new ImageFilterDAGSample();)
