/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_tool_utils_DEFINED
#define sk_tool_utils_DEFINED

#include "SkColor.h"
#include "SkImageEncoder.h"
#include "SkImageInfo.h"
#include "SkRandom.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTypeface.h"

class SkBitmap;
class SkCanvas;
class SkColorFilter;
class SkImage;
class SkPaint;
class SkPath;
class SkRRect;
class SkShader;
class SkSurface;
class SkSurfaceProps;
class SkTestFont;
class SkTextBlobBuilder;

namespace sk_tool_utils {

    const char* alphatype_name(SkAlphaType);
    const char* colortype_name(SkColorType);

    /**
     * Map opaque colors from 8888 to 565.
     */
    SkColor color_to_565(SkColor color);

    /**
     * Return a color emoji typeface if available.
     */
    sk_sp<SkTypeface> emoji_typeface();

    /**
     * If the platform supports color emoji, return sample text the emoji can render.
     */
    const char* emoji_sample_text();

    /**
     * Returns a string describing the platform font manager, if we're using one, otherwise "".
     */
    const char* platform_font_manager();

    /**
     * Sets the paint to use a platform-independent text renderer
     */
    void set_portable_typeface(SkPaint* paint, const char* name = nullptr,
                               SkFontStyle style = SkFontStyle());

    /**
     * Returns a platform-independent text renderer.
     */
    sk_sp<SkTypeface> create_portable_typeface(const char* name, SkFontStyle style);

    /** Call to clean up portable font references. */
    void release_portable_typefaces();

    /**
     *  Call writePixels() by using the pixels from bitmap, but with an info that claims
     *  the pixels are colorType + alphaType
     */
    void write_pixels(SkCanvas*, const SkBitmap&, int x, int y, SkColorType, SkAlphaType);
    void write_pixels(SkSurface*, const SkBitmap&, int x, int y, SkColorType, SkAlphaType);

    /**
     *  Returns true iff all of the pixels between the two images differ by <= the maxDiff value
     *  per component.
     *
     *  If the configs differ, return false.
     *
     *  If the colorType is half-float, then maxDiff is interpreted as 0..255 --> 0..1
     */
    bool equal_pixels(const SkPixmap&, const SkPixmap&, unsigned maxDiff = 0,
                      bool respectColorSpaces = false);
    bool equal_pixels(const SkBitmap&, const SkBitmap&, unsigned maxDiff = 0,
                      bool respectColorSpaces = false);
    bool equal_pixels(const SkImage* a, const SkImage* b, unsigned maxDiff = 0,
                      bool respectColorSpaces = false);

    // private to sk_tool_utils
    sk_sp<SkTypeface> create_font(const char* name, SkFontStyle);

    /** Returns a newly created CheckerboardShader. */
    sk_sp<SkShader> create_checkerboard_shader(SkColor c1, SkColor c2, int size);

    /** Draw a checkerboard pattern in the current canvas, restricted to
        the current clip, using SkXfermode::kSrc_Mode. */
    void draw_checkerboard(SkCanvas* canvas,
                           SkColor color1,
                           SkColor color2,
                           int checkSize);

    /** Make it easier to create a bitmap-based checkerboard */
    SkBitmap create_checkerboard_bitmap(int w, int h,
                                        SkColor c1, SkColor c2,
                                        int checkSize);

    /** A default checkerboard. */
    inline void draw_checkerboard(SkCanvas* canvas) {
        sk_tool_utils::draw_checkerboard(canvas, 0xFF999999, 0xFF666666, 8);
    }

    SkBitmap create_string_bitmap(int w, int h, SkColor c, int x, int y,
                                  int textSize, const char* str);

    // If the canvas does't make a surface (e.g. recording), make a raster surface
    sk_sp<SkSurface> makeSurface(SkCanvas*, const SkImageInfo&, const SkSurfaceProps* = nullptr);

    // A helper for inserting a drawtext call into a SkTextBlobBuilder
    void add_to_text_blob_w_len(SkTextBlobBuilder* builder, const char* text, size_t len,
                                const SkPaint& origPaint, SkScalar x, SkScalar y);

    void add_to_text_blob(SkTextBlobBuilder* builder, const char* text,
                          const SkPaint& origPaint, SkScalar x, SkScalar y);

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

    void make_big_path(SkPath& path);

    // Return a blurred version of 'src'. This doesn't use a separable filter
    // so it is slow!
    SkBitmap slow_blur(const SkBitmap& src, float sigma);

    SkRect compute_central_occluder(const SkRRect& rr);
    SkRect compute_widest_occluder(const SkRRect& rr);
    SkRect compute_tallest_occluder(const SkRRect& rr);

    // A helper object to test the topological sorting code (TopoSortBench.cpp & TopoSortTest.cpp)
    class TopoTestNode : public SkRefCnt {
    public:
        TopoTestNode(int id) : fID(id), fOutputPos(-1), fTempMark(false) { }

        void dependsOn(TopoTestNode* src) {
            *fDependencies.append() = src;
        }

        int id() const { return fID; }
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
        static bool WasOutput(TopoTestNode* node) { return (-1 != node->fOutputPos); }
        static int NumDependencies(TopoTestNode* node) { return node->fDependencies.count(); }
        static TopoTestNode* Dependency(TopoTestNode* node, int index) {
            return node->fDependencies[index];
        }

        // Helper functions for TopoSortBench & TopoSortTest
        static void AllocNodes(SkTArray<sk_sp<sk_tool_utils::TopoTestNode>>* graph, int num) {
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
            for (int i = graph->count()-1; i > 0; --i) {
                int swap = rand->nextU() % (i+1);

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

    template <typename T>
    inline sk_sp<SkData> EncodeImageToData(const T& src, SkEncodedImageFormat f, int q) {
        SkDynamicMemoryWStream buf;
        return SkEncodeImage(&buf, src , f, q) ? buf.detachAsData() : nullptr;
    }

    bool copy_to(SkBitmap* dst, SkColorType dstCT, const SkBitmap& src);
    void copy_to_g8(SkBitmap* dst, const SkBitmap& src);
}  // namespace sk_tool_utils

#endif  // sk_tool_utils_DEFINED
