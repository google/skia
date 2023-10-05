/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ToolUtils_DEFINED
#define ToolUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"  // IWYU pragma: keep
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkRandom.h"
#include "src/base/SkTInternalLList.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <functional>

class SkBitmap;
class SkCanvas;
class SkFont;
class SkImage;
class SkMatrix;
class SkMetaData;
class SkPaint;
class SkPath;
class SkShader;
class SkSurfaceProps;
class SkTextBlobBuilder;
enum SkAlphaType : int;
enum SkColorType : int;
enum class SkTextEncoding;
enum class SkTileMode;
struct SkImageInfo;

namespace ToolUtils {

const char* alphatype_name (SkAlphaType);
const char* colortype_name (SkColorType);
const char* colortype_depth(SkColorType);  // like colortype_name, but channel order agnostic
const char* tilemode_name(SkTileMode);

/**
 * Map opaque colors from 8888 to 565.
 */
SkColor color_to_565(SkColor color);

/* Return a color emoji typeface with planets to scale if available. */
sk_sp<SkTypeface> planet_typeface();

/** Return a color emoji typeface if available. */
sk_sp<SkTypeface> emoji_typeface();

/** Sample text for the emoji_typeface font. */
constexpr const char* emoji_sample_text() {
    return "\xF0\x9F\x98\x80"
           " "
           "\xE2\x99\xA2";  // 😀 ♢
}

/** A simple SkUserTypeface for testing. */
sk_sp<SkTypeface> sample_user_typeface();

/**
 * Returns a platform-independent text renderer.
 */
sk_sp<SkTypeface> create_portable_typeface(const char* name, SkFontStyle style);

static inline sk_sp<SkTypeface> create_portable_typeface() {
    return create_portable_typeface(nullptr, SkFontStyle());
}

void get_text_path(const SkFont&,
                   const void* text,
                   size_t      length,
                   SkTextEncoding,
                   SkPath*,
                   const SkPoint* positions = nullptr);

/**
 *  Returns true iff all of the pixels between the two images are identical.
 *
 *  If the configs differ, return false.
 */
bool equal_pixels(const SkPixmap&, const SkPixmap&);
bool equal_pixels(const SkBitmap&, const SkBitmap&);
bool equal_pixels(const SkImage* a, const SkImage* b);

/** Returns a newly created CheckerboardShader. */
sk_sp<SkShader> create_checkerboard_shader(SkColor c1, SkColor c2, int size);

/** Draw a checkerboard pattern in the current canvas, restricted to
    the current clip, using SkBlendMode::kSrc. */
void draw_checkerboard(SkCanvas* canvas, SkColor color1, SkColor color2, int checkSize);

/** Make it easier to create a bitmap-based checkerboard */
SkBitmap create_checkerboard_bitmap(int w, int h, SkColor c1, SkColor c2, int checkSize);

sk_sp<SkImage> create_checkerboard_image(int w, int h, SkColor c1, SkColor c2, int checkSize);

/** A default checkerboard. */
inline void draw_checkerboard(SkCanvas* canvas) {
    ToolUtils::draw_checkerboard(canvas, 0xFF999999, 0xFF666666, 8);
}

class HilbertGenerator {
public:
    HilbertGenerator(float desiredSize, float desiredLineWidth, int desiredDepth);

    // Draw a Hilbert curve into the canvas w/ a gradient along its length
    void draw(SkCanvas* canvas);

private:
    void turn90(bool turnLeft);
    void line(SkCanvas* canvas);
    void recursiveDraw(SkCanvas* canvas, int curDepth, bool turnLeft);
    SkColor4f getColor(float curLen);

    const float fDesiredSize;
    const int fDesiredDepth;
    const float fSegmentLength;            // length of a line segment
    const float fDesiredLineWidth;

    SkRect fActualBounds;

    // The "turtle" state
    SkPoint fCurPos;
    int fCurDir;

    const float fExpectedLen;
    float fCurLen;
};

/** Create pixmaps to initialize a 32x32 image w/ or w/o mipmaps.
 *  Returns the number of levels (either 1 or 6). The mipmap levels will be colored as
 *  specified in 'colors'
 */
int make_pixmaps(SkColorType,
                 SkAlphaType,
                 bool withMips,
                 const SkColor4f colors[6],
                 SkPixmap pixmaps[6],
                 std::unique_ptr<char[]>* mem);

SkBitmap create_string_bitmap(int w, int h, SkColor c, int x, int y, int textSize, const char* str);
sk_sp<SkImage> create_string_image(int w, int h, SkColor c, int x, int y, int textSize, const char* str);

// If the canvas doesn't make a surface (e.g. recording), make a raster surface
sk_sp<SkSurface> makeSurface(SkCanvas*, const SkImageInfo&, const SkSurfaceProps* = nullptr);

// A helper for inserting a drawtext call into a SkTextBlobBuilder
void add_to_text_blob_w_len(SkTextBlobBuilder*,
                            const char* text,
                            size_t      len,
                            SkTextEncoding,
                            const SkFont&,
                            SkScalar x,
                            SkScalar y);

void add_to_text_blob(SkTextBlobBuilder*, const char* text, const SkFont&, SkScalar x, SkScalar y);

// Constructs a star by walking a 'numPts'-sided regular polygon with even/odd fill:
//
//   moveTo(pts[0]);
//   lineTo(pts[step % numPts]);
//   ...
//   lineTo(pts[(step * (N - 1)) % numPts]);
//
// numPts=5, step=2 will produce a classic five-point star.
//
// numPts and step must be co-prime.
SkPath make_star(const SkRect& bounds, int numPts = 5, int step = 2);

void create_hemi_normal_map(SkBitmap* bm, const SkIRect& dst);

void create_frustum_normal_map(SkBitmap* bm, const SkIRect& dst);

void create_tetra_normal_map(SkBitmap* bm, const SkIRect& dst);

// A helper object to test the topological sorting code (TopoSortBench.cpp & TopoSortTest.cpp)
class TopoTestNode : public SkRefCnt {
public:
    TopoTestNode(int id) : fID(id) {}

    void dependsOn(TopoTestNode* src) { *fDependencies.append() = src; }
    void targets(uint32_t target) { *fTargets.append() = target; }

    int  id() const { return fID; }
    void reset() {
        fOutputPos = 0;
        fTempMark = false;
        fWasOutput = false;
    }

    uint32_t outputPos() const {
        SkASSERT(fWasOutput);
        return fOutputPos;
    }

    // check that the topological sort is valid for this node
    bool check() {
        if (!fWasOutput) {
            return false;
        }

        for (int i = 0; i < fDependencies.size(); ++i) {
            if (!fDependencies[i]->fWasOutput) {
                return false;
            }
            // This node should've been output after all the nodes on which it depends
            if (fOutputPos < fDependencies[i]->outputPos()) {
                return false;
            }
        }

        return true;
    }

    // The following 7 methods are needed by the topological sort
    static void SetTempMark(TopoTestNode* node) { node->fTempMark = true; }
    static void ResetTempMark(TopoTestNode* node) { node->fTempMark = false; }
    static bool IsTempMarked(TopoTestNode* node) { return node->fTempMark; }
    static void Output(TopoTestNode* node, uint32_t outputPos) {
        SkASSERT(!node->fWasOutput);
        node->fOutputPos = outputPos;
        node->fWasOutput = true;
    }
    static bool          WasOutput(TopoTestNode* node) { return node->fWasOutput; }
    static uint32_t      GetIndex(TopoTestNode* node) { return node->outputPos(); }
    static int           NumDependencies(TopoTestNode* node) { return node->fDependencies.size(); }
    static TopoTestNode* Dependency(TopoTestNode* node, int index) {
        return node->fDependencies[index];
    }
    static int           NumTargets(TopoTestNode* node) { return node->fTargets.size(); }
    static uint32_t      GetTarget(TopoTestNode* node, int i) { return node->fTargets[i]; }
    static uint32_t      GetID(TopoTestNode* node) { return node->id(); }

    // Helper functions for TopoSortBench & TopoSortTest
    static void AllocNodes(skia_private::TArray<sk_sp<ToolUtils::TopoTestNode>>* graph, int num) {
        graph->reserve_exact(graph->size() + num);

        for (int i = 0; i < num; ++i) {
            graph->push_back(sk_sp<TopoTestNode>(new TopoTestNode(i)));
        }
    }

#ifdef SK_DEBUG
    static void Print(const skia_private::TArray<TopoTestNode*>& graph) {
        for (int i = 0; i < graph.size(); ++i) {
            SkDebugf("%d, ", graph[i]->id());
        }
        SkDebugf("\n");
    }
#endif

    // randomize the array
    static void Shuffle(SkSpan<sk_sp<TopoTestNode>> graph, SkRandom* rand) {
        for (size_t i = graph.size() - 1; i > 0; --i) {
            int swap = rand->nextU() % (i + 1);

            graph[i].swap(graph[swap]);
        }
    }

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(TopoTestNode);

private:
    int      fID;
    uint32_t fOutputPos = 0;
    bool     fTempMark = false;
    bool     fWasOutput = false;

    SkTDArray<TopoTestNode*> fDependencies;
    SkTDArray<uint32_t>      fTargets;
};

bool copy_to(SkBitmap* dst, SkColorType dstCT, const SkBitmap& src);
void copy_to_g8(SkBitmap* dst, const SkBitmap& src);

class PixelIter {
public:
    PixelIter();
    PixelIter(SkSurface* surf) {
        SkPixmap pm;
        if (!surf->peekPixels(&pm)) {
            pm.reset();
        }
        this->reset(pm);
    }

    void reset(const SkPixmap& pm) {
        fPM  = pm;
        fLoc = {-1, 0};
    }

    void* next(SkIPoint* loc = nullptr) {
        if (!fPM.addr()) {
            return nullptr;
        }
        fLoc.fX += 1;
        if (fLoc.fX >= fPM.width()) {
            fLoc.fX = 0;
            if (++fLoc.fY >= fPM.height()) {
                this->setDone();
                return nullptr;
            }
        }
        if (loc) {
            *loc = fLoc;
        }
        return fPM.writable_addr(fLoc.fX, fLoc.fY);
    }

    void setDone() { fPM.reset(); }

private:
    SkPixmap fPM;
    SkIPoint fLoc;
};

using PathSniffCallback = void(const SkMatrix&, const SkPath&, const SkPaint&);

// Calls the provided PathSniffCallback for each path in the given file.
// Supported file formats .skp. (See SvgPathExtractor for .svg)
void ExtractPathsFromSKP(const char filepath[], std::function<PathSniffCallback>);

// Initialised with a font, this class can be called to setup GM UI with sliders for font
// variations, and returns a set of variation coordinates that matches what the sliders in the UI
// are set to. Useful for testing variable font properties, see colrv1.cpp.
class VariationSliders {
public:
    VariationSliders() {}

    VariationSliders(SkTypeface*,
                     SkFontArguments::VariationPosition variationPosition = {nullptr, 0});

    bool writeControls(SkMetaData* controls);

    /* Scans controls for information about the variation axes that the user may have configured.
     * Optionally pass in a boolean to receive information on whether the axes were updated. */
    void readControls(const SkMetaData& controls, bool* changed = nullptr);

    SkSpan<const SkFontArguments::VariationPosition::Coordinate> getCoordinates();

    static SkString tagToString(SkFourByteTag tag);

private:
    struct AxisSlider {
        SkScalar current;
        SkFontParameters::Variation::Axis axis;
        SkString name;
    };

    std::vector<AxisSlider> fAxisSliders;
    std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> fCoords;
    static constexpr size_t kAxisVarsSize = 3;
};

}  // namespace ToolUtils

#endif  // ToolUtils_DEFINED
