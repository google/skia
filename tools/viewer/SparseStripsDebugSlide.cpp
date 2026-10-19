/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_GRAPHITE) && defined(SK_ENABLE_SPARSE_STRIPS) && defined(GPU_TEST_UTILS)

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/core/SkFloatBits.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/StripGenerator.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"
#include "tests/graphite/sparse_strips/Oracle.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <utility>
#include <vector>

/*
 * SparseStripsDebug: an interactive oracle-diff overlay for the CPU sparse strips pipeline.
 *
 * Every analyzed draw is pushed through the production pipeline (Flatten -> Tiler(clip,
 * WindingHistogram) -> MakeStrips) and the resulting EndCaps + WideTiles are reconstructed into
 * per-pixel, per-subsample coverage over the *entire* clip. That coverage is diffed against
 * ScanlineOracle8x evaluated on the exact device-space path. Diffing the full clip (not just the
 * EndCaps) is what exposes WideTile / coarse-winding / left-edge-culling errors.
 *
 * Analysis sessions:
 *  - Baseline: emulated draws of the SKP at 1x (identity view matrix).
 *  - Capture ('C'): captures the *current* viewer transform. If the viewer is rendering with
 *    `--pr sparse8` on Graphite, the exact inputs Device hands to StripGenerator::processGeometry
 *    (path, ctm, clip, target size) are recorded via a GPU_TEST_UTILS hook ("device hook").
 *    Otherwise draws are emulated from the SKP under the captured transform ("emulated").
 *
 * All session data lives in the device space of the capture; the overlay is re-mapped onto the
 * current view so it stays registered with the content while panning/zooming.
 *
 * 'D' writes a bit-exact repro file (hex-float matrices/paths + clip + target size) of every
 * failing draw in the session.
 */

namespace {

namespace sg = skgpu::graphite;

constexpr uint16_t kTileW = sg::SparseStripConfig::kTileWidth;
constexpr uint16_t kTileH = sg::SparseStripConfig::kTileHeight;
constexpr int kTilePixels = kTileW * kTileH;
constexpr int kNumSamples = 8;
static_assert(sg::SparseStripConfig::kNumSubSamples == kNumSamples);

constexpr float kToleranceSq = static_cast<float>(sg::Flatten::kQuadTolerance2);
constexpr size_t kMaxStoredDiagnostics = 250000;

constexpr const char* kDefaultSkp = "skps/desk_mapsvg.skp";

enum Metric : int { kWindingMetric = 0, kCoverageMetric = 1 };
enum Strictness : int { kStrict = 0, kNonStrict = 1 };
enum Region : uint8_t { kGapRegion = 0, kWideRegion = 1, kCapRegion = 2 };

enum class InputSource { kEmulated, kDeviceHook };

float sample_x(int k) {
    return (static_cast<float>(sg::kMsaaPattern<uint8_t>[k]) + 0.5f) / kNumSamples;
}
float sample_y(int k) { return (static_cast<float>(k) + 0.5f) / kNumSamples; }

int floor_div(int v, int d) { return (v >= 0) ? v / d : -((-v + d - 1) / d); }

const char* fill_type_name(SkPathFillType ft) {
    switch (ft) {
        case SkPathFillType::kWinding:        return "kWinding";
        case SkPathFillType::kEvenOdd:        return "kEvenOdd";
        case SkPathFillType::kInverseWinding: return "kInverseWinding";
        case SkPathFillType::kInverseEvenOdd: return "kInverseEvenOdd";
    }
    return "?";
}

const char* region_name(uint8_t r) {
    return r == kCapRegion ? "EndCap" : (r == kWideRegion ? "WideTile" : "Gap (no geometry)");
}

uint8_t coverage_mask(const skvx::int8& w, bool evenOdd, bool inverse) {
    uint8_t mask = 0;
    for (int k = 0; k < kNumSamples; ++k) {
        bool inside = evenOdd ? ((w[k] & 1) != 0) : (w[k] != 0);
        if (inside != inverse) {
            mask |= (1 << k);
        }
    }
    return mask;
}

float dist_sq_to_segment(SkPoint p, SkPoint a, SkPoint b) {
    SkVector ab = b - a;
    SkVector ap = p - a;
    float lenSq = SkPoint::DotProduct(ab, ab);
    float t = lenSq > 0.0f ? std::clamp(SkPoint::DotProduct(ap, ab) / lenSq, 0.0f, 1.0f) : 0.0f;
    SkPoint proj = a + ab * t;
    SkVector d = p - proj;
    return SkPoint::DotProduct(d, d);
}

// ---- Bit-exact C++ emission helpers ---------------------------------------------------------

void append_float(SkString* out, float v) {
    out->appendf("SkBits2Float(0x%08x)", SkFloat2Bits(v));
}

void append_point(SkString* out, SkPoint p) {
    append_float(out, p.fX);
    out->append(", ");
    append_float(out, p.fY);
}

void append_matrix(SkString* out, const char* name, const SkMatrix& m) {
    out->appendf("    // [%g %g %g | %g %g %g | %g %g %g]\n",
                 m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
    out->appendf("    SkMatrix %s = SkMatrix::MakeAll(\n", name);
    for (int r = 0; r < 3; ++r) {
        out->append("            ");
        for (int c = 0; c < 3; ++c) {
            append_float(out, m[r * 3 + c]);
            out->append((r == 2 && c == 2) ? ");\n" : ", ");
        }
        if (r < 2) {
            out->append("\n");
        }
    }
}

void append_path(SkString* out, const char* name, const SkPath& path) {
    out->appendf("    SkPathBuilder %sBuilder(SkPathFillType::%s);\n",
                 name, fill_type_name(path.getFillType()));
    SkPath::RawIter iter(path);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        out->appendf("    %sBuilder.", name);
        switch (verb) {
            case SkPath::kMove_Verb:
                out->append("moveTo(");
                append_point(out, pts[0]);
                out->appendf(");  // %g, %g\n", pts[0].fX, pts[0].fY);
                break;
            case SkPath::kLine_Verb:
                out->append("lineTo(");
                append_point(out, pts[1]);
                out->appendf(");  // %g, %g\n", pts[1].fX, pts[1].fY);
                break;
            case SkPath::kQuad_Verb:
                out->append("quadTo(");
                append_point(out, pts[1]);
                out->append(", ");
                append_point(out, pts[2]);
                out->append(");\n");
                break;
            case SkPath::kConic_Verb:
                out->append("conicTo(");
                append_point(out, pts[1]);
                out->append(", ");
                append_point(out, pts[2]);
                out->append(", ");
                append_float(out, iter.conicWeight());
                out->append(");\n");
                break;
            case SkPath::kCubic_Verb:
                out->append("cubicTo(");
                append_point(out, pts[1]);
                out->append(", ");
                append_point(out, pts[2]);
                out->append(", ");
                append_point(out, pts[3]);
                out->append(");\n");
                break;
            case SkPath::kClose_Verb:
                out->append("close();\n");
                break;
            default:
                out->append("/* unknown verb */;\n");
                break;
        }
    }
    out->appendf("    SkPath %s = %sBuilder.detach();\n", name, name);
}

// ---- Data model -----------------------------------------------------------------------------

// The exact inputs to StripGenerator::processGeometry().
struct DrawInput {
    SkPath path;       // Local-space fill path (fill type includes inverse-ness).
    SkMatrix ctm;      // Local -> device.
    SkIRect clip;      // Clip handed to the pipeline.
    SkISize target;    // StripGenerator target size (drives WindingHistogram row count).
    SkColor color;
};

struct DrawRecord {
    DrawInput input;
    SkIRect usedClip;  // Clip after intersecting with target and tile-aligning left/top.
    SkPath devPath;
    SkPath flattened;  // Closed polygon built from the polyline.
    std::vector<sg::Line> chords;
    bool isCulled = false;
    bool pipelineOk = true;
    SkString streamError;
    int numTiles = 0;
    int numCaps = 0;
    int numWides = 0;
    int errorPixels[2][2] = {{0, 0}, {0, 0}};

    bool hasErrors(int metric, int strictness) const {
        return errorPixels[metric][strictness] > 0;
    }
    bool hasAnyError() const { return hasErrors(kWindingMetric, kStrict) || !pipelineOk ||
                                      !streamError.isEmpty(); }
};

struct PixelDiag {
    int drawIdx;
    int px, py;
    uint8_t region;
    uint8_t actualMask;
    uint8_t oracleMask;
    uint8_t windingDiff;  // Per-sample winding diff (parity for even-odd) outside caps.
    uint8_t tolMask;      // Per-sample: within flattening tolerance of a chord.
    skvx::int8 actualW;   // Only meaningful for EndCap pixels.
    skvx::int8 oracleW;

    uint8_t diff(int metric) const {
        return metric == kWindingMetric ? windingDiff
                                        : static_cast<uint8_t>(actualMask ^ oracleMask);
    }
    uint8_t activeDiff(int metric, int strictness) const {
        uint8_t d = this->diff(metric);
        return strictness == kStrict ? d : static_cast<uint8_t>(d & ~tolMask);
    }
};

struct Session {
    InputSource source = InputSource::kEmulated;
    bool isBaseline = true;
    int captureId = 0;
    SkMatrix captureMatrix = SkMatrix::I();
    SkMatrix inverseCapture = SkMatrix::I();
    SkISize targetSize = {0, 0};
    SkIRect deviceClip = SkIRect::MakeEmpty();

    std::vector<DrawInput> inputs;
    std::vector<DrawRecord> draws;
    std::vector<PixelDiag> diags;
    bool diagsTruncated = false;
    SkPath wireframe;

    std::vector<SkIPoint> errorTiles[2][2];
    size_t errorPixels[2][2] = {{0, 0}, {0, 0}};
    size_t errorSamples[2][2] = {{0, 0}, {0, 0}};
    size_t toleratedSamples[2] = {0, 0};
    int culledDraws = 0;
    int failedDraws = 0;
    SkString note;

    void resetResults() {
        draws.clear();
        diags.clear();
        diagsTruncated = false;
        wireframe.reset();
        for (int m = 0; m < 2; ++m) {
            toleratedSamples[m] = 0;
            for (int s = 0; s < 2; ++s) {
                errorTiles[m][s].clear();
                errorPixels[m][s] = 0;
                errorSamples[m][s] = 0;
            }
        }
        culledDraws = 0;
        failedDraws = 0;
    }
};

// Replays an SKP and emulates the inputs Device::drawGeometry would hand to the sparse strips
// pipeline. This is an approximation (only drawPath is sniffed, and Device's clip/bounds logic is
// mirrored rather than reused); prefer the device hook capture when available.
class EmulatingCanvas final : public SkNoDrawCanvas {
public:
    EmulatingCanvas(SkISize target, std::vector<DrawInput>* out)
            : SkNoDrawCanvas(target.width(), target.height()), fTarget(target), fOut(out) {}

protected:
    void onDrawPicture(const SkPicture* picture,
                       const SkMatrix* matrix,
                       const SkPaint* paint) override {
        this->SkCanvas::onDrawPicture(picture, matrix, paint);
    }

    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        const SkMatrix ctm = this->getTotalMatrix();
        if (ctm.hasPerspective()) {
            return;  // Device transforms perspective paths itself; out of scope here.
        }
        SkPathBuilder builder;
        if (!skpathutils::FillPathWithPaint(path, paint, &builder, nullptr, ctm)) {
            return;  // Hairline; never rendered via sparse strips.
        }
        SkPath fillPath = builder.detach();
        if (fillPath.isEmpty() || !fillPath.isFinite()) {
            return;
        }
        const SkIRect scissor = this->getDeviceClipBounds();
        if (scissor.isEmpty()) {
            return;
        }
        // Mirrors Device::drawGeometry(): inverse fills use scissor; regular fills outset
        // transformedShapeBounds by 1px for AA, intersect with scissor, and roundOut().
        SkIRect clip = scissor;
        if (!fillPath.isInverseFillType()) {
            SkRect devBounds = ctm.mapRect(fillPath.getBounds());
            devBounds.outset(1.0f, 1.0f);
            SkRect inter;
            if (!inter.intersect(devBounds, SkRect::Make(scissor))) {
                return;
            }
            clip = inter.roundOut();
        }
        fOut->push_back({fillPath, ctm, clip, fTarget, paint.getColor()});
    }

private:
    SkISize fTarget;
    std::vector<DrawInput>* fOut;
};

SkColor debug_color_for_index(int i) {
    static constexpr SkColor kPalette[] = {0xFF4E79A7, 0xFFF28E2B, 0xFFE15759, 0xFF76B7B2,
                                           0xFF59A14F, 0xFFEDC948, 0xFFB07AA1, 0xFFFF9DA7,
                                           0xFF9C755F, 0xFFBAB0AC};
    return kPalette[i % std::size(kPalette)];
}

}  // namespace

class SparseStripsDebugSlide final : public Slide {
public:
    SparseStripsDebugSlide() : fLut(sg::GenerateMSAALUT()) { fName = "SparseStripsDebug"; }

    void load(SkScalar, SkScalar) override {
        const char* envSkp = std::getenv("SPARSE_STRIPS_SKP");
        fSkpPath = envSkp ? envSkp : kDefaultSkp;
        fPicture = LoadPicture(fSkpPath, &fLoadError);
        fSession = Session();
        fBaselineDone = false;
        fCaptureRequested = false;
        fHasSelectedTile = false;
    }

    void unload() override {
        sg::StripGenerator::SetDebugCaptureCallback(nullptr);
        fPicture.reset();
        fSession = Session();
    }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'c': case 'C': fCaptureRequested = true;                    return true;
            case 'x': case 'X': fBaselineDone = false;                       return true;
            case 'd': case 'D': this->writeSessionReport();                  return true;
            case 'p': case 'P': this->dumpSelectedTileDraws();               return true;
            case 'e': case 'E': this->dumpSelectedTileErrors();              return true;
            case 's': case 'S':
                fStrictness = fStrictness == kStrict ? kNonStrict : kStrict;
                fErrorTileIndex = 0;
                return true;
            case 'm': case 'M':
                fMetric = fMetric == kWindingMetric ? kCoverageMetric : kWindingMetric;
                fErrorTileIndex = 0;
                return true;
            case ']': case '}': this->stepErrorTile(+1);                     return true;
            case '[': case '{': this->stepErrorTile(-1);                     return true;
            case 'n': case 'N': fShowSubsampleDeltas = !fShowSubsampleDeltas; return true;
            case 'w': case 'W': fShowWireframe = !fShowWireframe;            return true;
            case 'f': case 'F': fShowFlattenedFill = !fShowFlattenedFill;    return true;
            case 'b': case 'B': fBackgroundMode = (fBackgroundMode + 1) % 3; return true;
            case 'g': case 'G': fShowPixelGrid = !fShowPixelGrid;            return true;
            case 't': case 'T': fShowTileGrid = !fShowTileGrid;              return true;
            case 'h': case 'H': fShowHUD = !fShowHUD;                        return true;
            default: break;
        }
        return false;
    }

    bool onMouse(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey) override {
        if (state == skui::InputState::kDown) {
            fMouseDown = true;
            fMouseDownSlidePt = {x, y};
            fMouseDragSlideDelta = {0, 0};
        } else if (state == skui::InputState::kMove) {
            if (fMouseDown) {
                // Viewer::onMouse calls Slide::onMouse before fGesture.touchMoved updates fLocalM,
                // so {x, y} - fMouseDownSlidePt is the incremental step delta of this move event.
                fMouseDragSlideDelta.fX += x - fMouseDownSlidePt.fX;
                fMouseDragSlideDelta.fY += y - fMouseDownSlidePt.fY;
            }
        } else if (state == skui::InputState::kUp) {
            fMouseDown = false;
            const SkVector winDelta = fCurrentViewMatrix.mapVector(fMouseDragSlideDelta.fX,
                                                                   fMouseDragSlideDelta.fY);
            if (winDelta.length() < 4.0f) {
                // Slide coordinates are content coordinates; map them into the capture's device
                // space (see draw() for why captureMatrix is the right mapping).
                SkPoint dev = fSession.captureMatrix.mapPoint({x, y});
                fSelectedTile = {floor_div(static_cast<int>(std::floor(dev.fX)), kTileW) * kTileW,
                                 floor_div(static_cast<int>(std::floor(dev.fY)), kTileH) * kTileH};
                fHasSelectedTile = true;
            }
        }
        return false;  // Let the viewer keep handling panning.
    }

    void draw(SkCanvas* canvas) override {
        fCurrentViewMatrix = canvas->getTotalMatrix();
        if (!fLoadError.isEmpty()) {
            this->drawMessage(canvas, fLoadError, SK_ColorRED);
            return;
        }
        sg::Recorder* recorder = canvas->recorder();
        if (!recorder) {
            canvas->drawPicture(fPicture);
            this->drawMessage(canvas,
                              SkString("SparseStripsDebug requires a Graphite backend (the "
                                       "AlphaAtlasManager needs a Recorder)."),
                              SK_ColorRED);
            return;
        }

        if (fCaptureRequested) {
            fCaptureRequested = false;
            this->capture(canvas, recorder);  // Draws the picture itself with the hook installed.
        } else {
            if (!fBaselineDone) {
                this->runBaseline(recorder);
            }
            this->drawBackground(canvas);
        }

        // Session data is in the capture's device space. A content point p is drawn by the
        // current view at T_now * p; at capture time it was at C * p. Concatenating C^-1 onto the
        // current canvas maps capture-device geometry onto the content as currently displayed.
        canvas->save();
        canvas->concat(fSession.inverseCapture);
        this->drawOverlay(canvas);
        canvas->restore();

        if (fShowHUD) {
            this->drawHUD(canvas);
        }
    }

private:
    // ---- Loading / capture ------------------------------------------------------------------

    static sk_sp<SkPicture> LoadPicture(const SkString& path, SkString* error) {
        std::unique_ptr<SkStreamAsset> stream;
        sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
        if (!data) {
            data = SkData::MakeFromFileName(SkStringPrintf("../%s", path.c_str()).c_str());
        }
        if (data) {
            stream = SkMemoryStream::Make(data);
        } else {
            stream = GetResourceAsStream(path.c_str());
        }
        if (!stream) {
            error->printf("Failed to load SKP '%s' (set SPARSE_STRIPS_SKP to override).",
                          path.c_str());
            return nullptr;
        }
        sk_sp<SkPicture> picture = SkPicture::MakeFromStream(stream.get());
        if (!picture) {
            error->printf("Failed to parse SKP '%s'.", path.c_str());
        }
        return picture;
    }

    SkISize pictureExtent() const {
        SkRect cull = fPicture->cullRect();
        return {std::max(1, static_cast<int>(std::ceil(cull.fRight))),
                std::max(1, static_cast<int>(std::ceil(cull.fBottom)))};
    }

    void emulateInputs(const SkMatrix& view, SkISize target, std::vector<DrawInput>* out) const {
        EmulatingCanvas emu(target, out);
        emu.concat(view);
        emu.drawPicture(fPicture);
    }

    void runBaseline(sg::Recorder* recorder) {
        fSession = Session();
        fSession.isBaseline = true;
        fSession.source = InputSource::kEmulated;
        fSession.targetSize = this->pictureExtent();
        fSession.deviceClip = SkIRect::MakeSize(fSession.targetSize);
        this->emulateInputs(SkMatrix::I(), fSession.targetSize, &fSession.inputs);
        this->analyzeSession(recorder);
        fBaselineDone = true;
        this->selectFirstErrorTile();

        SkDebugf("[SparseStripsDebug] Baseline: %zu draws (emulated), %zu erroneous pixels "
                 "(winding/strict), %zu (winding/non-strict).\n",
                 fSession.draws.size(),
                 fSession.errorPixels[kWindingMetric][kStrict],
                 fSession.errorPixels[kWindingMetric][kNonStrict]);
    }

    void capture(SkCanvas* canvas, sg::Recorder* recorder) {
        std::vector<DrawInput> hooked;
        sg::StripGenerator::SetDebugCaptureCallback(
                [&hooked](const SkPath& path, const SkMatrix& ctm, const SkIRect& clip,
                          SkISize target) {
                    hooked.push_back({path, ctm, clip, target,
                                      debug_color_for_index(static_cast<int>(hooked.size()))});
                });
        // Draw directly to the root device (no saveLayer) so captured coordinates are root-device
        // coordinates that line up with the canvas matrix captured below.
        canvas->drawPicture(fPicture);
        sg::StripGenerator::SetDebugCaptureCallback(nullptr);

        fSession = Session();
        fSession.isBaseline = false;
        fSession.captureId = ++fCaptureCounter;
        fSession.captureMatrix = canvas->getTotalMatrix();
        if (!fSession.captureMatrix.invert(&fSession.inverseCapture)) {
            fSession.inverseCapture = SkMatrix::I();
        }
        fSession.deviceClip = canvas->getDeviceClipBounds();
        fSession.targetSize = canvas->imageInfo().dimensions();

        if (!hooked.empty()) {
            fSession.source = InputSource::kDeviceHook;
            fSession.inputs = std::move(hooked);
        } else {
            fSession.source = InputSource::kEmulated;
            fSession.note = SkString("No sparse strips draws observed by the device hook (is "
                                     "the path renderer set to sparse8?); emulating draws.");
            this->emulateInputs(fSession.captureMatrix, fSession.targetSize, &fSession.inputs);
        }
        this->analyzeSession(recorder);
        fHasSelectedTile = false;
        this->selectFirstErrorTile();

        SkDebugf("[SparseStripsDebug] Capture #%d: %zu draws (%s), %zu erroneous pixels "
                 "(winding/strict). Press 'D' to write a repro file.\n",
                 fSession.captureId, fSession.draws.size(),
                 fSession.source == InputSource::kDeviceHook ? "device hook" : "emulated",
                 fSession.errorPixels[kWindingMetric][kStrict]);
    }

    // ---- Analysis -----------------------------------------------------------------------------

    void analyzeSession(sg::Recorder* recorder) {
        fSession.resetResults();
        SkPathBuilder wire;
        for (size_t i = 0; i < fSession.inputs.size(); ++i) {
            this->analyzeDraw(static_cast<int>(i), recorder, &wire);
        }
        fSession.wireframe = wire.detach();

        for (int m = 0; m < 2; ++m) {
            for (int s = 0; s < 2; ++s) {
                auto& tiles = fSession.errorTiles[m][s];
                std::sort(tiles.begin(), tiles.end(), [](SkIPoint a, SkIPoint b) {
                    return a.fY != b.fY ? a.fY < b.fY : a.fX < b.fX;
                });
                tiles.erase(std::unique(tiles.begin(), tiles.end()), tiles.end());
            }
        }
        fErrorTileIndex = 0;
    }

    void analyzeDraw(int drawIdx, sg::Recorder* recorder, SkPathBuilder* wire) {
        fSession.draws.emplace_back();
        DrawRecord& rec = fSession.draws.back();
        rec.input = fSession.inputs[drawIdx];
        const DrawInput& in = rec.input;

        rec.usedClip = in.clip;
        if (!rec.usedClip.intersect(SkIRect::MakeSize(in.target))) {
            rec.usedClip = SkIRect::MakeEmpty();
        } else {
            rec.usedClip.fLeft = (rec.usedClip.fLeft / kTileW) * kTileW;
            rec.usedClip.fTop = (rec.usedClip.fTop / kTileH) * kTileH;
        }
        const SkIRect clip = rec.usedClip;
        rec.devPath = in.path.makeTransform(in.ctm);
        rec.devPath.setFillType(in.path.getFillType());

        if (in.ctm.hasPerspective() || clip.isEmpty()) {
            return;
        }

        const SkPathFillType fillType = in.path.getFillType();
        const bool evenOdd = SkPathFillType_IsEvenOdd(fillType);
        const bool inverse = SkPathFillType_IsInverse(fillType);

        // --- Production pipeline, mirroring StripGenerator::processGeometry() ---
        sg::Flatten flattener;
        sg::Polyline polyline;
        flattener.processPaths<sg::FlattenMode::kSimd>(in.path, in.ctm, clip, &polyline);

        sg::WideTiles wides;
        sg::EndCaps ends;
        std::vector<uint8_t> masks;
        std::vector<skvx::int8> windings;
        sg::Tiles<kTileW, kTileH> tiler;

        if (polyline.count() > 0 || inverse) {
            int rows = (std::max(in.target.height(), clip.bottom()) + kTileH - 1) / kTileH;
            sg::WindingHistogram histogram(rows);
            histogram.resize(rows);
            histogram.clear();

            rec.isCulled = tiler.makeTilesMSAA(polyline, clip, &histogram);
            tiler.sortTiles();
            rec.numTiles = tiler.getTiles().size();

            sg::AlphaAtlasManager atlas(recorder);
            auto observer = [&](uint8_t exactMask, skvx::int8 winding) {
                masks.push_back(exactMask);
                windings.push_back(winding);
            };
            rec.pipelineOk = sg::MakeStrips::MsaaSimd<kTileW, kTileH>(tiler, &wides, &ends, &atlas,
                                                                      fillType, polyline, fLut,
                                                                      rec.isCulled, histogram,
                                                                      clip, observer);
        }
        rec.numCaps = ends.caps().size();
        rec.numWides = wides.tiles().size();
        fSession.culledDraws += rec.isCulled ? 1 : 0;
        fSession.failedDraws += rec.pipelineOk ? 0 : 1;

        // --- Visualization geometry ---
        {
            SkPathBuilder flat(fillType);
            bool inContour = false;
            for (const SkPoint& pt : polyline.points()) {
                if (std::isnan(pt.fX) || std::isnan(pt.fY)) {
                    if (inContour) {
                        flat.close();
                    }
                    inContour = false;
                } else if (!inContour) {
                    flat.moveTo(pt);
                    inContour = true;
                } else {
                    flat.lineTo(pt);
                }
            }
            if (inContour) {
                flat.close();
            }
            rec.flattened = flat.detach();
            for (auto it = polyline.begin(); it != polyline.end(); ++it) {
                auto [line, idx] = *it;
                rec.chords.push_back(line);
                wire->moveTo(line.p0);
                wire->lineTo(line.p1);
            }
        }

        // --- Stream reconstruction ---
        std::vector<int> capMaskStart(ends.caps().size() + 1, 0);
        for (int c = 0; c < ends.caps().size(); ++c) {
            capMaskStart[c + 1] = capMaskStart[c] + (ends.caps()[c].fWidth / kTileW) * kTilePixels;
        }
        if (static_cast<size_t>(capMaskStart.back()) != masks.size()) {
            rec.streamError.printf("Observer produced %zu masks but EndCaps span %d pixels.",
                                   masks.size(), capMaskStart.back());
        }

        const int x0 = std::max(clip.fLeft, 0);
        const int x1 = std::min(clip.fRight, in.target.width());
        const int y0 = std::max(clip.fTop, 0);
        const int y1 = std::min(clip.fBottom, in.target.height());
        if (x0 >= x1 || y0 >= y1) {
            return;
        }

        const int firstTileRow = y0 / kTileH;
        const int numTileRows = (y1 - 1) / kTileH - firstTileRow + 1;
        std::vector<std::vector<int>> capsByRow(numTileRows), widesByRow(numTileRows);
        for (int c = 0; c < ends.caps().size(); ++c) {
            int r = ends.caps()[c].fY / kTileH - firstTileRow;
            if (r >= 0 && r < numTileRows) {
                capsByRow[r].push_back(c);
            }
        }
        for (int w = 0; w < wides.tiles().size(); ++w) {
            int r = wides.tiles()[w].fY / kTileH - firstTileRow;
            if (r >= 0 && r < numTileRows) {
                widesByRow[r].push_back(w);
            }
        }

        // Chords bucketed by tile row (with tolerance margin) for the non-strict check.
        std::vector<std::vector<int>> chordsByRow(numTileRows);
        for (size_t i = 0; i < rec.chords.size(); ++i) {
            const auto& l = rec.chords[i];
            float minY = std::min(l.p0.fY, l.p1.fY) - 0.25f;
            float maxY = std::max(l.p0.fY, l.p1.fY) + 0.25f;
            int r0 = std::max(0, static_cast<int>(std::floor(minY / kTileH)) - firstTileRow);
            int r1 = std::min(numTileRows - 1,
                              static_cast<int>(std::floor(maxY / kTileH)) - firstTileRow);
            for (int r = r0; r <= r1; ++r) {
                chordsByRow[r].push_back(static_cast<int>(i));
            }
        }

        struct PixelSrc {
            uint8_t region;
            int maskIdx;
        };
        std::vector<PixelSrc> rowSrc(x1 - x0);
        sg::ScanlineOracle8x oracle(rec.devPath);

        for (int py = y0; py < y1; ++py) {
            const int r = py / kTileH - firstTileRow;
            std::fill(rowSrc.begin(), rowSrc.end(), PixelSrc{kGapRegion, -1});
            for (int w : widesByRow[r]) {
                const auto& wt = wides.tiles()[w];
                for (int x = std::max<int>(wt.fX, x0); x < std::min<int>(wt.fX + wt.fWidth, x1);
                     ++x) {
                    rowSrc[x - x0] = {kWideRegion, -1};
                }
            }
            for (int c : capsByRow[r]) {
                const auto& cap = ends.caps()[c];
                const int localY = py - cap.fY;
                for (int x = std::max<int>(cap.fX, x0); x < std::min<int>(cap.fX + cap.fWidth, x1);
                     ++x) {
                    if (rowSrc[x - x0].region == kWideRegion && rec.streamError.isEmpty()) {
                        rec.streamError.printf("EndCap overlaps WideTile at (%d, %d).", x, py);
                    }
                    const int s = (x - cap.fX) / kTileW;
                    const int localX = x - cap.fX - s * kTileW;
                    rowSrc[x - x0] = {kCapRegion, capMaskStart[c] + s * kTilePixels +
                                                          localY * kTileW + localX};
                }
            }

            oracle.buildRow(py);
            const auto& intervals = oracle.rowIntervals();

            for (int px = x0; px < x1; ++px) {
                const PixelSrc& src = rowSrc[px - x0];
                const skvx::int8 oracleW = sg::ScanlineOracle8x::GetOracleWinding(px, intervals);
                const uint8_t oracleMask = coverage_mask(oracleW, evenOdd, inverse);

                uint8_t actualMask = 0;
                skvx::int8 actualW(0);
                bool hasWinding = false;
                if (src.region == kWideRegion) {
                    actualMask = 0xFF;
                } else if (src.region == kCapRegion && src.maskIdx >= 0 &&
                           static_cast<size_t>(src.maskIdx) < masks.size()) {
                    actualMask = masks[src.maskIdx];
                    actualW = windings[src.maskIdx];
                    hasWinding = true;
                }

                const uint8_t covDiff = actualMask ^ oracleMask;
                uint8_t windDiff = covDiff;
                if (hasWinding) {
                    windDiff = 0;
                    for (int k = 0; k < kNumSamples; ++k) {
                        bool differs = evenOdd ? (((actualW[k] ^ oracleW[k]) & 1) != 0)
                                               : (actualW[k] != oracleW[k]);
                        windDiff |= differs ? (1 << k) : 0;
                    }
                }
                if ((covDiff | windDiff) == 0) {
                    continue;
                }

                PixelDiag diag;
                diag.drawIdx = drawIdx;
                diag.px = px;
                diag.py = py;
                diag.region = src.region;
                diag.actualMask = actualMask;
                diag.oracleMask = oracleMask;
                diag.windingDiff = windDiff;
                diag.actualW = actualW;
                diag.oracleW = oracleW;
                diag.tolMask = 0;
                for (int k = 0; k < kNumSamples; ++k) {
                    if (!((covDiff | windDiff) & (1 << k))) {
                        continue;
                    }
                    SkPoint sp = {px + sample_x(k), py + sample_y(k)};
                    for (int li : chordsByRow[r]) {
                        const auto& l = rec.chords[li];
                        if (dist_sq_to_segment(sp, l.p0, l.p1) <= kToleranceSq) {
                            diag.tolMask |= (1 << k);
                            break;
                        }
                    }
                }
                this->recordDiag(diag, &rec);
            }
        }
    }

    void recordDiag(const PixelDiag& diag, DrawRecord* rec) {
        const SkIPoint tile = {floor_div(diag.px, kTileW) * kTileW,
                               floor_div(diag.py, kTileH) * kTileH};
        for (int m = 0; m < 2; ++m) {
            fSession.toleratedSamples[m] +=
                    std::popcount(static_cast<unsigned>(diag.diff(m) & diag.tolMask));
            for (int s = 0; s < 2; ++s) {
                uint8_t active = diag.activeDiff(m, s);
                if (active) {
                    rec->errorPixels[m][s]++;
                    fSession.errorPixels[m][s]++;
                    fSession.errorSamples[m][s] += std::popcount(static_cast<unsigned>(active));
                    fSession.errorTiles[m][s].push_back(tile);
                }
            }
        }
        if (fSession.diags.size() < kMaxStoredDiagnostics) {
            fSession.diags.push_back(diag);
        } else {
            fSession.diagsTruncated = true;
        }
    }

    // ---- Selection ----------------------------------------------------------------------------

    const std::vector<SkIPoint>& activeErrorTiles() const {
        return fSession.errorTiles[fMetric][fStrictness];
    }

    void selectFirstErrorTile() {
        const auto& tiles = this->activeErrorTiles();
        if (!fHasSelectedTile && !tiles.empty()) {
            fSelectedTile = tiles[0];
            fHasSelectedTile = true;
            fErrorTileIndex = 0;
        }
    }

    void stepErrorTile(int dir) {
        const auto& tiles = this->activeErrorTiles();
        if (tiles.empty()) {
            return;
        }
        const size_t n = tiles.size();
        fErrorTileIndex = std::min(fErrorTileIndex, n - 1);
        fErrorTileIndex = dir > 0 ? (fErrorTileIndex + 1) % n : (fErrorTileIndex + n - 1) % n;
        fSelectedTile = tiles[fErrorTileIndex];
        fHasSelectedTile = true;
    }

    SkIRect selectedTileRect() const {
        return SkIRect::MakeXYWH(fSelectedTile.fX, fSelectedTile.fY, kTileW, kTileH);
    }

    // Draws whose (used) clip and device bounds touch the selected tile.
    std::vector<int> drawsTouchingSelectedTile() const {
        std::vector<int> result;
        const SkIRect tile = this->selectedTileRect();
        for (size_t i = 0; i < fSession.draws.size(); ++i) {
            const auto& rec = fSession.draws[i];
            SkIRect devBounds = rec.devPath.getBounds().roundOut();
            if (SkPathFillType_IsInverse(rec.devPath.getFillType())) {
                devBounds = rec.usedClip;
            }
            if (SkIRect::Intersects(devBounds, tile) && SkIRect::Intersects(rec.usedClip, tile)) {
                result.push_back(static_cast<int>(i));
            }
        }
        return result;
    }

    // ---- Reporting ----------------------------------------------------------------------------

    SkString sessionHeader() const {
        SkString out;
        out.appendf("// SparseStripsDebug capture\n");
        out.appendf("//   SKP:          %s\n", fSkpPath.c_str());
        out.appendf("//   Session:      %s (#%d), inputs via %s\n",
                    fSession.isBaseline ? "baseline 1x" : "capture", fSession.captureId,
                    fSession.source == InputSource::kDeviceHook
                            ? "StripGenerator device hook (exact)"
                            : "SKP emulation (approximate clips)");
        out.appendf("//   Target size:  %d x %d, device clip [%d %d %d %d]\n",
                    fSession.targetSize.width(), fSession.targetSize.height(),
                    fSession.deviceClip.fLeft, fSession.deviceClip.fTop,
                    fSession.deviceClip.fRight, fSession.deviceClip.fBottom);
        out.appendf("//   Draws:        %zu (%d culled, %d pipeline failures)\n",
                    fSession.draws.size(), fSession.culledDraws, fSession.failedDraws);
        out.appendf("//   Errors (pixels) winding strict/non-strict: %zu / %zu, coverage "
                    "strict/non-strict: %zu / %zu%s\n",
                    fSession.errorPixels[kWindingMetric][kStrict],
                    fSession.errorPixels[kWindingMetric][kNonStrict],
                    fSession.errorPixels[kCoverageMetric][kStrict],
                    fSession.errorPixels[kCoverageMetric][kNonStrict],
                    fSession.diagsTruncated ? " (diagnostic list truncated)" : "");
        out.append("// Canvas total matrix at capture (viewer transform):\n");
        append_matrix(&out, "viewMatrix", fSession.captureMatrix);
        out.append("\n");
        return out;
    }

    SkString drawRepro(int drawIdx) const {
        const DrawRecord& rec = fSession.draws[drawIdx];
        const DrawInput& in = rec.input;
        SkString out;
        const SkRect b = rec.devPath.getBounds();
        out.appendf("// ---- Draw #%d | %s | verbs=%d | culled=%s | tiles=%d caps=%d wides=%d%s\n",
                    drawIdx, fill_type_name(in.path.getFillType()), in.path.countVerbs(),
                    rec.isCulled ? "yes" : "no", rec.numTiles, rec.numCaps, rec.numWides,
                    rec.pipelineOk ? "" : " | MakeStrips FAILED");
        out.appendf("// Error pixels: winding %d/%d, coverage %d/%d (strict/non-strict)\n",
                    rec.errorPixels[kWindingMetric][kStrict],
                    rec.errorPixels[kWindingMetric][kNonStrict],
                    rec.errorPixels[kCoverageMetric][kStrict],
                    rec.errorPixels[kCoverageMetric][kNonStrict]);
        if (!rec.streamError.isEmpty()) {
            out.appendf("// Stream error: %s\n", rec.streamError.c_str());
        }
        out.appendf("// Device bounds: [%g %g %g %g]\n", b.fLeft, b.fTop, b.fRight, b.fBottom);
        out.append("{\n");
        out.appendf("    // Inputs to StripGenerator::processGeometry(path, ctm, clip)\n");
        out.appendf("    SkIRect clip = SkIRect::MakeLTRB(%d, %d, %d, %d);\n",
                    rec.usedClip.fLeft, rec.usedClip.fTop, rec.usedClip.fRight,
                    rec.usedClip.fBottom);
        if (rec.usedClip != in.clip) {
            out.appendf("    // (captured clip before tile alignment: [%d %d %d %d])\n",
                        in.clip.fLeft, in.clip.fTop, in.clip.fRight, in.clip.fBottom);
        }
        out.appendf("    SkISize target = SkISize::Make(%d, %d);  // StripGenerator size\n",
                    in.target.width(), in.target.height());
        append_matrix(&out, "ctm", in.ctm);
        append_path(&out, "path", in.path);
        out.append("}\n\n");
        return out;
    }

    void writeSessionReport() const {
        if (fSession.draws.empty()) {
            SkDebugf("[SparseStripsDebug] Nothing to dump; no session analyzed yet.\n");
            return;
        }
        SkString report = this->sessionHeader();
        int failing = 0;
        for (size_t i = 0; i < fSession.draws.size(); ++i) {
            if (fSession.draws[i].hasAnyError()) {
                report.append(this->drawRepro(static_cast<int>(i)));
                failing++;
            }
        }
        if (failing == 0) {
            report.append("// No failing draws in this session.\n");
        }
        SkString filename = SkStringPrintf("sparse_strips_capture_%d%s.txt", fSession.captureId,
                                           fSession.isBaseline ? "_baseline" : "");
        SkFILEWStream file(filename.c_str());
        if (file.isValid()) {
            file.write(report.c_str(), report.size());
            SkDebugf("[SparseStripsDebug] Wrote %d failing draw(s) to %s\n", failing,
                     filename.c_str());
        } else {
            SkDebugf("[SparseStripsDebug] Could not open %s; dumping to stdout.\n",
                     filename.c_str());
        }
        if (!file.isValid() || failing <= 3) {
            printf("%s", report.c_str());
            fflush(stdout);
        }
    }

    void dumpSelectedTileDraws() const {
        if (!fHasSelectedTile) {
            SkDebugf("[SparseStripsDebug] No tile selected (click a tile or use '[' / ']').\n");
            return;
        }
        SkString out = this->sessionHeader();
        out.appendf("// Selected tile (%d, %d) .. (%d, %d)\n\n", fSelectedTile.fX,
                    fSelectedTile.fY, fSelectedTile.fX + kTileW, fSelectedTile.fY + kTileH);
        std::vector<int> drawIdxs = this->drawsTouchingSelectedTile();
        if (drawIdxs.empty()) {
            out.append("// No draws touch this tile.\n");
        }
        for (int i : drawIdxs) {
            out.append(this->drawRepro(i));
        }
        printf("%s", out.c_str());
        fflush(stdout);
    }

    void dumpSelectedTileErrors() const {
        if (!fHasSelectedTile) {
            SkDebugf("[SparseStripsDebug] No tile selected (click a tile or use '[' / ']').\n");
            return;
        }
        const SkIRect tile = this->selectedTileRect();
        std::vector<const PixelDiag*> diags;
        for (const auto& d : fSession.diags) {
            if (tile.contains(d.px, d.py)) {
                diags.push_back(&d);
            }
        }
        std::sort(diags.begin(), diags.end(), [](const PixelDiag* a, const PixelDiag* b) {
            if (a->drawIdx != b->drawIdx) return a->drawIdx < b->drawIdx;
            if (a->py != b->py) return a->py < b->py;
            return a->px < b->px;
        });

        printf("\n=============================================================================\n");
        printf("[SparseStripsDebug] Tile (%d, %d)  metric=%s  mode=%s\n", tile.fLeft, tile.fTop,
               fMetric == kWindingMetric ? "WINDING" : "COVERAGE",
               fStrictness == kStrict ? "STRICT" : "NON-STRICT (0.25px)");
        printf("=============================================================================\n");
        if (diags.empty()) {
            printf("No discrepancies recorded in this tile.\n\n");
            fflush(stdout);
            return;
        }

        int lastDraw = -1;
        for (const PixelDiag* d : diags) {
            if (d->drawIdx != lastDraw) {
                lastDraw = d->drawIdx;
                const auto& rec = fSession.draws[lastDraw];
                printf("\n--- Draw #%d (%s, culled=%s, clip=[%d %d %d %d]) ---\n", lastDraw,
                       fill_type_name(rec.devPath.getFillType()), rec.isCulled ? "yes" : "no",
                       rec.usedClip.fLeft, rec.usedClip.fTop, rec.usedClip.fRight,
                       rec.usedClip.fBottom);
            }
            uint8_t active = d->activeDiff(fMetric, fStrictness);
            printf("Pixel (%d, %d) local (%d, %d) | %s | %s | actual mask 0x%02x, oracle mask "
                   "0x%02x\n",
                   d->px, d->py, d->px - tile.fLeft, d->py - tile.fTop, region_name(d->region),
                   active ? "ERROR" : "tolerated", d->actualMask, d->oracleMask);
            printf("  k | subsample (x, y)       | act W | orc W | act cov | orc cov | near chord "
                   "| status\n");
            for (int k = 0; k < kNumSamples; ++k) {
                bool bitDiff = (d->diff(fMetric) >> k) & 1;
                bool tol = (d->tolMask >> k) & 1;
                const char* status = !bitDiff ? "match"
                                              : (((active >> k) & 1) ? "ERROR" : "tolerated");
                SkString actW = d->region == kCapRegion ? SkStringPrintf("%+3d", d->actualW[k])
                                                        : SkString("  -");
                printf("  %d | (%10.4f, %10.4f) |  %s  |  %+3d  |    %d    |    %d    |    %s     "
                       "| %s\n",
                       k, d->px + sample_x(k), d->py + sample_y(k), actW.c_str(), d->oracleW[k],
                       (d->actualMask >> k) & 1, (d->oracleMask >> k) & 1, tol ? "yes" : " no",
                       status);
            }
        }

        // Chords near the tile, per draw with errors.
        lastDraw = -1;
        for (const PixelDiag* d : diags) {
            if (d->drawIdx == lastDraw) {
                continue;
            }
            lastDraw = d->drawIdx;
            const auto& rec = fSession.draws[lastDraw];
            printf("\nFlattened chords of draw #%d in/near tile:\n", lastDraw);
            for (const auto& l : rec.chords) {
                float minX = std::min(l.p0.fX, l.p1.fX), maxX = std::max(l.p0.fX, l.p1.fX);
                float minY = std::min(l.p0.fY, l.p1.fY), maxY = std::max(l.p0.fY, l.p1.fY);
                if (maxX >= tile.fLeft - 1 && minX <= tile.fRight + 1 &&
                    maxY >= tile.fTop - 1 && minY <= tile.fBottom + 1) {
                    printf("  (%a, %a) -> (%a, %a)   // (%.4f, %.4f) -> (%.4f, %.4f)\n",
                           l.p0.fX, l.p0.fY, l.p1.fX, l.p1.fY, l.p0.fX, l.p0.fY, l.p1.fX,
                           l.p1.fY);
                }
            }
        }
        printf("\n");
        fflush(stdout);
    }

    // ---- Drawing ------------------------------------------------------------------------------

    void drawBackground(SkCanvas* canvas) const {
        if (fBackgroundMode == 0) {
            return;
        }
        if (fBackgroundMode == 1) {
            SkPaint faint;
            faint.setAlphaf(0.30f);
            canvas->saveLayer(nullptr, &faint);
            canvas->drawPicture(fPicture);
            canvas->restore();
        } else {
            canvas->drawPicture(fPicture);
        }
    }

    void drawOverlay(SkCanvas* canvas) const {
        const SkRect vis = canvas->getLocalClipBounds();  // In capture device space.
        const float pixelsPerUnit = canvas->getTotalMatrix().mapRadius(1.0f);

        if (fShowFlattenedFill) {
            for (const auto& rec : fSession.draws) {
                if (!SkRect::Intersects(rec.flattened.getBounds(), vis)) {
                    continue;
                }
                SkPaint fill;
                fill.setColor(rec.input.color);
                fill.setAlphaf(fBackgroundMode > 0 ? 0.35f : 0.85f);
                canvas->drawPath(rec.flattened, fill);
            }
        }

        if (fShowWireframe) {
            SkPaint wire;
            wire.setAntiAlias(true);
            wire.setStyle(SkPaint::kStroke_Style);
            wire.setStrokeWidth(0.0f);
            wire.setColor(SkColorSetARGB(230, 0, 229, 255));
            canvas->drawPath(fSession.wireframe, wire);
        }

        auto drawGrid = [&](int step, SkColor color) {
            const int gx0 = floor_div(static_cast<int>(std::floor(vis.fLeft)), step) * step;
            const int gy0 = floor_div(static_cast<int>(std::floor(vis.fTop)), step) * step;
            const int gx1 = static_cast<int>(std::ceil(vis.fRight));
            const int gy1 = static_cast<int>(std::ceil(vis.fBottom));
            if ((gx1 - gx0) / step + (gy1 - gy0) / step > 4000) {
                return;  // Too dense to be useful at this zoom.
            }
            SkPaint p;
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(0.0f);
            p.setColor(color);
            SkPathBuilder grid;
            for (int x = gx0; x <= gx1; x += step) {
                grid.moveTo(x, gy0);
                grid.lineTo(x, gy1);
            }
            for (int y = gy0; y <= gy1; y += step) {
                grid.moveTo(gx0, y);
                grid.lineTo(gx1, y);
            }
            canvas->drawPath(grid.detach(), p);
        };
        if (fShowPixelGrid && pixelsPerUnit >= 4.0f) {
            drawGrid(1, SkColorSetARGB(38, 200, 220, 240));
        }
        if (fShowTileGrid) {
            drawGrid(kTileW, SkColorSetARGB(95, 255, 170, 0));
        }

        if (fHasSelectedTile) {
            const SkRect sel = SkRect::Make(this->selectedTileRect());
            SkPaint selFill;
            selFill.setColor(SkColorSetARGB(55, 255, 235, 0));
            canvas->drawRect(sel, selFill);
            SkPaint selBox;
            selBox.setStyle(SkPaint::kStroke_Style);
            selBox.setStrokeWidth(0.0f);
            selBox.setColor(SkColorSetARGB(255, 255, 235, 0));
            canvas->drawRect(sel, selBox);

            // Outline the clips of draws with errors in the selected tile (magenta).
            SkPaint clipBox;
            clipBox.setStyle(SkPaint::kStroke_Style);
            clipBox.setStrokeWidth(0.0f);
            clipBox.setColor(SkColorSetARGB(220, 255, 0, 220));
            std::vector<int> drawn;
            for (const auto& d : fSession.diags) {
                if (this->selectedTileRect().contains(d.px, d.py) &&
                    d.activeDiff(fMetric, fStrictness) &&
                    std::find(drawn.begin(), drawn.end(), d.drawIdx) == drawn.end()) {
                    drawn.push_back(d.drawIdx);
                    canvas->drawRect(SkRect::Make(fSession.draws[d.drawIdx].usedClip), clipBox);
                }
            }
        }

        SkPaint errBox;
        errBox.setStyle(SkPaint::kStroke_Style);
        errBox.setStrokeWidth(0.0f);
        errBox.setColor(SkColorSetARGB(200, 255, 60, 60));
        SkPaint green;
        green.setAntiAlias(true);
        green.setColor(SkColorSetARGB(250, 0, 230, 118));
        SkPaint red;
        red.setAntiAlias(true);
        red.setColor(SkColorSetARGB(255, 255, 23, 68));
        const bool drawSamples = pixelsPerUnit >= 12.0f;
        const bool drawNumbers = fShowSubsampleDeltas && pixelsPerUnit >= 40.0f;
        SkFont numFont(ToolUtils::DefaultTypeface(), 0.16f);
        SkPaint shadow;
        shadow.setAntiAlias(true);
        shadow.setColor(SkColorSetARGB(220, 0, 0, 0));
        SkPaint num;
        num.setAntiAlias(true);

        for (const auto& d : fSession.diags) {
            const uint8_t active = d.activeDiff(fMetric, fStrictness);
            if (!active) {
                continue;
            }
            const SkRect px = SkRect::MakeXYWH(d.px, d.py, 1, 1);
            if (!SkRect::Intersects(px, vis)) {
                continue;
            }
            canvas->drawRect(px, errBox);
            if (!drawSamples) {
                continue;
            }
            for (int k = 0; k < kNumSamples; ++k) {
                const float sx = d.px + sample_x(k);
                const float sy = d.py + sample_y(k);
                const bool isErr = (active >> k) & 1;
                canvas->drawCircle(sx, sy, 0.038f, isErr ? red : green);
                if (!drawNumbers || !isErr) {
                    continue;
                }
                SkString label;
                if (fMetric == kWindingMetric && d.region == kCapRegion) {
                    int delta = static_cast<int>(d.actualW[k]) - static_cast<int>(d.oracleW[k]);
                    label.printf("%+d", delta);
                    num.setColor(delta > 0 ? SkColorSetRGB(255, 80, 80)
                                           : SkColorSetRGB(80, 180, 255));
                } else {
                    bool cov = (d.actualMask >> k) & 1;
                    label = cov ? "+C" : "-C";
                    num.setColor(cov ? SkColorSetRGB(255, 100, 100)
                                     : SkColorSetRGB(100, 200, 255));
                }
                canvas->drawString(label, sx + 0.035f, sy + 0.055f, numFont, shadow);
                canvas->drawString(label, sx + 0.030f, sy + 0.050f, numFont, num);
            }
        }
    }

    void drawMessage(SkCanvas* canvas, const SkString& msg, SkColor color) const {
        canvas->save();
        canvas->resetMatrix();
        SkPaint p;
        p.setColor(color);
        SkFont font(ToolUtils::DefaultTypeface(), 18.0f);
        canvas->drawString(msg, 30, 50, font, p);
        canvas->restore();
    }

    void drawHUD(SkCanvas* canvas) const {
        canvas->save();
        canvas->resetMatrix();

        std::vector<std::pair<SkString, SkColor>> lines;
        auto add = [&](SkString s, SkColor c) { lines.emplace_back(std::move(s), c); };

        SkString title;
        if (fSession.isBaseline) {
            title.printf("SparseStrips Oracle Diff | Baseline (1x, emulated)");
        } else {
            title.printf("SparseStrips Oracle Diff | Capture #%d (%s)", fSession.captureId,
                         fSession.source == InputSource::kDeviceHook ? "device hook, exact"
                                                                     : "emulated");
        }
        add(title, SK_ColorWHITE);
        add(SkStringPrintf("SKP: %s | draws: %zu (%d culled, %d failed) | target %dx%d",
                           fSkpPath.c_str(), fSession.draws.size(), fSession.culledDraws,
                           fSession.failedDraws, fSession.targetSize.width(),
                           fSession.targetSize.height()),
            SkColorSetRGB(180, 205, 230));
        if (!fSession.isBaseline) {
            const SkMatrix& m = fSession.captureMatrix;
            add(SkStringPrintf("View: scale (%.4g, %.4g) skew (%.4g, %.4g) trans (%.4g, %.4g)",
                               m.getScaleX(), m.getScaleY(), m.getSkewX(), m.getSkewY(),
                               m.getTranslateX(), m.getTranslateY()),
                SkColorSetRGB(180, 205, 230));
        }
        if (!fSession.note.isEmpty()) {
            add(fSession.note, SkColorSetRGB(255, 200, 80));
        }

        const size_t errPx = fSession.errorPixels[fMetric][fStrictness];
        add(SkStringPrintf("Metric: %s | Mode: %s | %zu error pixels, %zu error samples%s%s",
                           fMetric == kWindingMetric ? "WINDING" : "COVERAGE",
                           fStrictness == kStrict ? "STRICT" : "NON-STRICT (0.25px)", errPx,
                           fSession.errorSamples[fMetric][fStrictness],
                           fStrictness == kNonStrict
                                   ? SkStringPrintf(" (%zu tolerated)",
                                                    fSession.toleratedSamples[fMetric]).c_str()
                                   : "",
                           fSession.diagsTruncated ? " [list truncated]" : ""),
            errPx == 0 ? SkColorSetRGB(100, 230, 120) : SkColorSetRGB(255, 120, 90));

        if (fHasSelectedTile) {
            std::vector<int> touching = this->drawsTouchingSelectedTile();
            int withErrors = 0;
            for (int i : touching) {
                withErrors += fSession.draws[i].hasErrors(fMetric, fStrictness) ? 1 : 0;
            }
            add(SkStringPrintf("Selected tile (%d, %d) | %zu draw(s), %d with errors | "
                               "error tile %zu/%zu",
                               fSelectedTile.fX, fSelectedTile.fY, touching.size(), withErrors,
                               std::min(fErrorTileIndex + 1, this->activeErrorTiles().size()),
                               this->activeErrorTiles().size()),
                SkColorSetRGB(255, 235, 50));
        } else {
            add(SkString("Selected tile: none (click a tile or use [ / ])"),
                SkColorSetRGB(180, 180, 180));
        }

        const char* bg = fBackgroundMode == 0 ? "OFF" : (fBackgroundMode == 1 ? "FAINT" : "FULL");
        add(SkStringPrintf("[S]trict  [M]etric  [N]umbers:%s  [B]g:%s  [W]ire:%s  [F]ill:%s  "
                           "[G]rid:%s  [T]iles:%s  [H]UD",
                           fShowSubsampleDeltas ? "on" : "off", bg, fShowWireframe ? "on" : "off",
                           fShowFlattenedFill ? "on" : "off", fShowPixelGrid ? "on" : "off",
                           fShowTileGrid ? "on" : "off"),
            SkColorSetRGB(0, 229, 255));
        add(SkString("[C]apture view  [X] baseline  [D]ump repro file  "
                     "[P] tile draws  [E] tile errors  [ / ] step"),
            SkColorSetRGB(255, 170, 80));

        SkFont titleFont(ToolUtils::DefaultTypeface(), 16.0f);
        SkFont bodyFont(ToolUtils::DefaultTypeface(), 13.0f);
        const float lineH = 22.0f;
        SkRect hud = SkRect::MakeXYWH(20, 20, 820, 24 + lineH * lines.size());
        SkPaint bgPaint;
        bgPaint.setColor(SkColorSetARGB(238, 12, 16, 22));
        canvas->drawRoundRect(hud, 10, 10, bgPaint);
        SkPaint border;
        border.setAntiAlias(true);
        border.setStyle(SkPaint::kStroke_Style);
        border.setStrokeWidth(1.5f);
        border.setColor(SkColorSetARGB(190, 80, 140, 220));
        canvas->drawRoundRect(hud, 10, 10, border);

        float y = 44.0f;
        for (size_t i = 0; i < lines.size(); ++i) {
            SkPaint text;
            text.setAntiAlias(true);
            text.setColor(lines[i].second);
            canvas->drawString(lines[i].first, 36, y, i == 0 ? titleFont : bodyFont, text);
            y += lineH;
        }
        canvas->restore();
    }

    // ---- State --------------------------------------------------------------------------------

    const sg::SparseStripConfig::LUTArray fLut;
    SkString fSkpPath;
    SkString fLoadError;
    sk_sp<SkPicture> fPicture;

    Session fSession;
    int fCaptureCounter = 0;
    bool fBaselineDone = false;
    bool fCaptureRequested = false;

    SkIPoint fSelectedTile = {0, 0};
    bool fHasSelectedTile = false;
    size_t fErrorTileIndex = 0;
    bool fMouseDown = false;
    SkPoint fMouseDownSlidePt = {0, 0};
    SkVector fMouseDragSlideDelta = {0, 0};
    SkMatrix fCurrentViewMatrix = SkMatrix::I();

    int fMetric = kWindingMetric;
    int fStrictness = kNonStrict;
    bool fShowSubsampleDeltas = false;
    bool fShowWireframe = true;
    bool fShowFlattenedFill = true;
    int fBackgroundMode = 1;  // 0: off, 1: faint, 2: full
    bool fShowPixelGrid = true;
    bool fShowTileGrid = true;
    bool fShowHUD = true;
};

DEF_SLIDE(return new SparseStripsDebugSlide();)

#endif  // SK_GRAPHITE && SK_ENABLE_SPARSE_STRIPS && GPU_TEST_UTILS
