/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cmath>
#include <utility>
#include <queue>
#include <deque>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkCubicMap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkSize.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkAssert.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "tools/viewer/Slide.h"

#include "imgui.h"

/*
* Helper function that gets the all of the t-values that need to be added between
* one t-value on a path to the next, from a sorted queue |tValuesToAdd|. Converts
* the value from its proportion across the whole line to it's proportion relative
* to the current segment.
*/
std::vector<SkScalar> getTValuesForSegment(std::deque<float>* tValuesToAdd, float t, float tNext) {
    std::vector<SkScalar> tVector;
    while (!tValuesToAdd->empty() && tValuesToAdd->front() > t && tValuesToAdd->front() < tNext) {
        SkScalar total_t = tValuesToAdd->front();
        tValuesToAdd->pop_front();
        SkScalar relative_t = (total_t - t) / (tNext - t);
        tVector.push_back(relative_t);
    }
    return tVector;
}

/*
* Helper function that takes a vector of t-values and chops a cubic at those correct
* values, added it to the path |out|.
*/
void addSegmentsFromTValues(const SkPoint cubic_pts[4], std::vector<SkScalar> t_values, SkPath* out) {
    const size_t arr_size = t_values.size();
    const int dst_size = (3*arr_size) + 4;
    std::vector<SkPoint> split_pts(dst_size);
    SkChopCubicAt(cubic_pts, split_pts.data(), t_values.data(), arr_size);

    for (size_t i = 0; i < arr_size + 1; i++) {
      out->cubicTo(split_pts[(i*3)+1], split_pts[(i*3)+2], split_pts[(i*3)+3]);
    }
}

/*
* Helper function that given a path, it's t-values (sorted), and t-values to add
* (sorted), returns a new path that is all cubic beziers, with verbs at each of
* those t-values.
*/
bool createPathFromTValues(const SkPath& in, std::deque<float> tValuesToAdd, std::vector<float> tValues, SkPath* out) {
    SkPath::Iter iter(in, false);
    bool fBreak = false;

    // Only increment if we draw on the path.
    size_t t_value_idx = 0;

    for (;;) {
      if (fBreak) break;
      bool needToSplit = false;
      SkPoint pts[4];
      SkPath::Verb verb = iter.next(pts);

      // The last t-value is always the end of the path (when t=1).
      if (t_value_idx >= tValues.size() - 1) {
        break;
      }
      // t and tNext are the start and end of the current segment.
      float t = tValues[t_value_idx];
      float tNext = tValues[t_value_idx+1];

      // Check if current tValueToAdd is on this current segment.
      if (!tValuesToAdd.empty() && tValuesToAdd.front() > t && tValuesToAdd.front() < tNext) {
        needToSplit = true;
      }

      switch (verb) {
        case SkPath::kMove_Verb:
         // Only supports one contour currently.
          out->moveTo(pts[0]);
          break;
        case SkPath::kLine_Verb: {
            t_value_idx++;
              SkPoint pt1, pt2;
              pt1 = pts[0]*(1.0f / 3.0f) + pts[1]*(2.0f / 3.0f);
              pt2 = pts[0]*(2.0f / 3.0f) + pts[1]*(1.0f / 3.0f);
            if (!needToSplit) {
              out->cubicTo(pt1, pt2, pts[1]);
            } else {
              std::vector<SkScalar> tVector = getTValuesForSegment(&tValuesToAdd, t, tNext);
              const SkPoint cubic_pts[4] = {pts[0], pt1, pt2, pts[1]};
              addSegmentsFromTValues(cubic_pts, tVector, out);
            }
            break;
        }
        case SkPath::kQuad_Verb: {
            t_value_idx++;
            SkPoint pt1, pt2;
            pt1 = pts[0] + (pts[1]-pts[0])*(2.0f / 3.0f);
            pt2 = pts[2] + (pts[1]-pts[2])*(2.0f / 3.0f);
            if (!needToSplit) {
              out->cubicTo(pt1, pt2, pts[2]);
            } else {
              std::vector<SkScalar> tVector = getTValuesForSegment(&tValuesToAdd, t, tNext);
              const SkPoint cubic_pts[4] = {pts[0], pt1, pt2, pts[2]};
              addSegmentsFromTValues(cubic_pts, tVector, out);
            }
            break;
        }
        case SkPath::kCubic_Verb:
          t_value_idx++;
          if (!needToSplit) {
            out->cubicTo(pts[1], pts[2], pts[3]);
          } else {
              std::vector<SkScalar> tVector = getTValuesForSegment(&tValuesToAdd, t, tNext);
              addSegmentsFromTValues(pts, tVector, out);
          }
          break;
        case SkPath::kConic_Verb:
          // Conic not yet supported.
          return false;
        case SkPath::kClose_Verb:
          // Close not yet supported.
          out->close();
          break;
        case SkPath::kDone_Verb:
          fBreak = true;
      }
    }
    return true;
}

/*
* Helper function to get the total lengths the verbs take of a path and put it
* into a vector.
*/
std::vector<SkScalar> getTValues(const SkPath& path) {
  std::vector<SkScalar> tValues;
  SkPathMeasure measure(path, false);
  SkScalar length = measure.getLength();
  if (length <= 0) {
    SkDebugf("Length of path is 0.\n");
    return tValues;
  }
  const SkContourMeasure* cmeasure = measure.currentMeasure();
  tValues.push_back(0.0f);
  for (const auto vmeasure: *cmeasure) {
    tValues.push_back(vmeasure.fDistance / length);
  }

  if (measure.nextContour()) {
    SkDebugf("Path has more than 1 contour.\n");
    return {};
  }
  return tValues;
}

/*
* Helper function that creates a deque from a sorted vector |original| and adds
* values from a sorted vector |additional| in sorted order.
*/
std::deque<float> getTValuesToAdd(std::vector<SkScalar> original, std::vector<SkScalar> additional) {
    std::deque<float> tValuesToAdd;
    size_t i = 0, j = 0;
    while (i < original.size() && j < additional.size()) {
      if (additional[j] < original[i]) {
        tValuesToAdd.push_back(additional[j]);
        j++;
      } else if (additional[j] > original[i]) {
        i++;
      } else { // additional[j] == original[i]
        i++;
        j++;
      }
    }
    while (j < additional.size()) {
      tValuesToAdd.push_back(additional[j]);
      j++;
    }
    return tValuesToAdd;
}

/*
* Extension to SkPath::Interpolate function that takes two arbitrary SkPaths.
*
* The current functionality of SkPath::Interpolate requires that the two paths
* have identical verbs (same number of verbs and verb types). This function does
* preprocessing on the two paths to create two new paths that fit this requirement
* without modifying the original path.
*
* The function uses a list of t-values to determine where to place points along
* the path, and adds more of these points based on the other paths values. Then all
* verbs are converted to cubic functions. When t-values need to be added, the cubic
* is chopped at the correct positions in accordance to the t-values.
*
* TODO: Add support for multiple contours, conic verbs, and close verbs.
* TODO: Fix phrasing as we don't use actual t-values of the curves just proportional
* distances(?)
*/
bool generalInterpolate(const SkPath& beginning, const SkPath& ending, SkScalar weight, SkPath* out) {
  // Use existing path interpolation if possible.
  if (beginning.isInterpolatable(ending)) {
      return beginning.interpolate(ending, weight, out);
  }

  // TODO: Check if isValid()?
  if (beginning.isEmpty() || !beginning.isFinite() || ending.isEmpty() || !ending.isFinite()) {
    return false;
  }

  // New paths to store the transformed paths.
  SkPath beginningCubic;
  SkPath endingCubic;
  beginningCubic.reset();
  endingCubic.reset();

  // Append the total distances up to each verb in the path into a vector.
  std::vector<SkScalar> tValues1 = getTValues(beginning);
  std::vector<SkScalar> tValues2 = getTValues(ending);
  if (tValues1.empty() || tValues2.empty()) {
    return false;
  }

  // The t-values to add for the respective paths from the other.
  std::deque<float> tValuesToAdd1 = getTValuesToAdd(tValues1, tValues2);
  std::deque<float> tValuesToAdd2 = getTValuesToAdd(tValues2, tValues1);

  // Form the cubic versions of each path with the new points along it.
  createPathFromTValues(beginning, tValuesToAdd1, tValues1, &beginningCubic);
  createPathFromTValues(ending, tValuesToAdd2, tValues2, &endingCubic);

  return beginningCubic.interpolate(endingCubic, weight, out);
}


namespace {

class PathLerpSlide final : public Slide {
public:
    PathLerpSlide()
        : fTimeMapper({0.5f, 0}, {0.5f, 1}) {
        fName = "PathLerp";
    }

private:
    void load(SkScalar w, SkScalar h) override {
        fSize = {w, h};

        // Some hard-coded samples to start.
        // TODO: add more/selectable samples.
        SkPath p0, p1;
        SkAssertResult(SkParsePath::FromSVGString(
            "M0,0.5 Q0,0 0.5,0 Q1,0 1,0.5", &p0));
        SkAssertResult(SkParsePath::FromSVGString(
            "M0,0.5 Q0,1 0.5,1 Q1,1 1,0.5 L1.1,0.5 L1.2,0.5 L1.3,0.5 L1.5,0.5", &p1));

        this->updateAnimatingPaths(p0, p1);
    }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void draw(SkCanvas* canvas) override {
        SkPaint path_paint;
        path_paint.setColor(0xff424242);
        path_paint.setStyle(SkPaint::kStroke_Style);
        path_paint.setAntiAlias(true);
        path_paint.setStrokeCap(SkPaint::kRound_Cap);
        path_paint.setStrokeWidth(10 / fPathTransform.getScaleX());

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(fPathTransform);
        canvas->drawPath(fInterpolatedPath, path_paint);

        if (fShowVertices) {
            SkPaint vertex_paint, ctrl_paint;
            vertex_paint.setColor(0xffff0000);
            vertex_paint.setAntiAlias(true);
            ctrl_paint.setColor(0xff0000ff);
            ctrl_paint.setAntiAlias(true);

            const float vertex_radius = 10 / fPathTransform.getScaleX(),
                          ctrl_radius =  5 / fPathTransform.getScaleX();

            for (const auto [verb, pts, weights] : SkPathPriv::Iterate(fInterpolatedPath)) {
                switch (verb) {
                    case SkPathVerb::kMove: // pts: [ vertex ]
                        canvas->drawCircle(pts[0], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kLine: // pts: [ prev_vertex, vertex ]
                        canvas->drawCircle(pts[1], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kQuad: // pts: [ prev_vertex, ctrl, vertex ]
                        canvas->drawCircle(pts[1], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[2], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kCubic: // pts: [ prev_vertex, ctrl0, ctrl1, vertex ]
                        canvas->drawCircle(pts[1], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[2], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[3], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kConic: // pts: [ prev_vertex, ctrl, vertex ]
                        canvas->drawCircle(pts[1], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[2], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kClose: // pts: []
                        break;
                }
            }
        }
    }

    bool animate(double nanos) override {
        if (!fTimeBase) {
            fTimeBase = nanos;
        }

        // Oscillating between 0..1
        const float t =
                std::abs((std::fmod((nanos - fTimeBase)*0.000000001*fAnimationSpeed, 2) - 1));

        // Interpolate with easing
        return generalInterpolate(fPaths.first, fPaths.second, fTimeMapper.computeYFromX(t), &fInterpolatedPath);
    }

    bool onChar(SkUnichar c) override {
        switch (c) {
            case 'v':
                fShowVertices = !fShowVertices;
                return true;
            default:
                return false;
        }
    }

    void updateAnimatingPaths(const SkPath& p0, const SkPath& p1) {
        // TODO: preprocess arbitrary paths with mismatched verbs/controls.
        // SkASSERT(p0.isInterpolatable(p1));

        fPaths = {p0, p1};

        // Scale and center such that the path animation fills 90% of the view.
        SkRect bounds = p0.computeTightBounds();
        bounds.join(p1.computeTightBounds());

        const SkRect dst_rect = SkRect::MakeSize(fSize)
            .makeInset(fSize.width() * .05f, fSize.height() * .05f);
        fPathTransform = SkMatrix::MakeRectToRect(bounds, dst_rect, SkMatrix::kCenter_ScaleToFit);
    }

    SkSize                    fSize = {0,0};
    std::pair<SkPath, SkPath> fPaths;        // currently morphing paths
    SkPath                    fInterpolatedPath;
    SkMatrix                  fPathTransform = SkMatrix::I();

    float                     fAnimationSpeed = 1.f;
    double                    fTimeBase       = 0;
    const SkCubicMap          fTimeMapper;   // for animation easing

    bool                      fShowVertices   = false;
};

}  // namespace

DEF_SLIDE(return new PathLerpSlide();)


