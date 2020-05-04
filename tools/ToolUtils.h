/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ToolUtils_DEFINED
#define ToolUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "include/utils/SkRandom.h"

class SkBitmap;
class SkCanvas;
class SkFontStyle;
class SkImage;
class SkPath;
class SkPixmap;
class SkRRect;
class SkShader;
class SkSurface;
class SkSurfaceProps;
class SkTextBlobBuilder;
class SkTypeface;

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
const char* emoji_sample_text();

/**
 * Returns a platform-independent text renderer.
 */
sk_sp<SkTypeface> create_portable_typeface(const char* name, SkFontStyle style);

static inline sk_sp<SkTypeface> create_portable_typeface() {
    return create_portable_typeface(nullptr, SkFontStyle());
}

/**
 *  Turn on portable (--nonativeFonts) or GDI font rendering (--gdi).
 */
void SetDefaultFontMgr();


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
    the current clip, using SkXfermode::kSrc_Mode. */
void draw_checkerboard(SkCanvas* canvas, SkColor color1, SkColor color2, int checkSize);

/** Make it easier to create a bitmap-based checkerboard */
SkBitmap create_checkerboard_bitmap(int w, int h, SkColor c1, SkColor c2, int checkSize);

/** A default checkerboard. */
inline void draw_checkerboard(SkCanvas* canvas) {
    ToolUtils::draw_checkerboard(canvas, 0xFF999999, 0xFF666666, 8);
}

SkBitmap create_string_bitmap(int w, int h, SkColor c, int x, int y, int textSize, const char* str);

// If the canvas does't make a surface (e.g. recording), make a raster surface
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

void make_big_path(SkPath& path);

// A helper object to test the topological sorting code (TopoSortBench.cpp & TopoSortTest.cpp)
class TopoTestNode : public SkRefCnt {
public:
    TopoTestNode(int id) : fID(id), fOutputPos(-1), fTempMark(false) {}

    void dependsOn(TopoTestNode* src) { *fDependencies.append() = src; }

    int  id() const { return fID; }
    void reset() { fOutputPos = -1; }

    int outputPos() const { return fOutputPos; }

    // check that the topological sort is valid for this node
    bool check() {
        if (-1 == fOutputPos) {
            return false;
        }

        for (int i = 0; i < fDependencies.count(); ++i) {
            if (-1 == fDependencies[i]->outputPos()) {
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
    static void Output(TopoTestNode* node, int outputPos) {
        SkASSERT(-1 != outputPos);
        node->fOutputPos = outputPos;
    }
    static bool          WasOutput(TopoTestNode* node) { return (-1 != node->fOutputPos); }
    static int           NumDependencies(TopoTestNode* node) { return node->fDependencies.count(); }
    static TopoTestNode* Dependency(TopoTestNode* node, int index) {
        return node->fDependencies[index];
    }

    // Helper functions for TopoSortBench & TopoSortTest
    static void AllocNodes(SkTArray<sk_sp<ToolUtils::TopoTestNode>>* graph, int num) {
        graph->reserve(num);

        for (int i = 0; i < num; ++i) {
            graph->push_back(sk_sp<TopoTestNode>(new TopoTestNode(i)));
        }
    }

#ifdef SK_DEBUG
    static void Print(const SkTArray<TopoTestNode*>& graph) {
        for (int i = 0; i < graph.count(); ++i) {
            SkDebugf("%d, ", graph[i]->id());
        }
        SkDebugf("\n");
    }
#endif

    // randomize the array
    static void Shuffle(SkTArray<sk_sp<TopoTestNode>>* graph, SkRandom* rand) {
        for (int i = graph->count() - 1; i > 0; --i) {
            int swap = rand->nextU() % (i + 1);

            (*graph)[i].swap((*graph)[swap]);
        }
    }

private:
    int  fID;
    int  fOutputPos;
    bool fTempMark;

    SkTDArray<TopoTestNode*> fDependencies;
};

template <typename T>
inline bool EncodeImageToFile(const char* path, const T& src, SkEncodedImageFormat f, int q) {
    SkFILEWStream file(path);
    return file.isValid() && SkEncodeImage(&file, src, f, q);
}

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

}  // namespace ToolUtils

#endif  // ToolUtils_DEFINED
