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
#include "SkPixelSerializer.h"
#include "SkRandom.h"
#include "SkTDArray.h"
#include "SkTypeface.h"

class SkBitmap;
class SkCanvas;
class SkPaint;
class SkPath;
class SkShader;
class SkTestFont;
class SkTextBlobBuilder;

namespace sk_tool_utils {

    const char* colortype_name(SkColorType);

    /**
     * Map opaque colors from 8888 to 565.
     */
    SkColor color_to_565(SkColor color);

    /**
     * Return a color emoji typeface if available.
     */
    void emoji_typeface(SkAutoTUnref<SkTypeface>* );

    /**
     * If the platform supports color emoji, return sample text the emoji can render.
     */
    const char* emoji_sample_text();

    /**
     * If the platform supports color emoji, return the type (i.e. "CBDT", "SBIX", "").
     */
    const char* platform_os_emoji();

    /**
     * Return the platform name with the version number ("Mac10.9", "Win8", etc.) if available.
     */
    const char* platform_os_name();

    /**
     * Return the platform name without the version number ("Mac", "Win", etc.) if available.
     */
    SkString major_platform_os_name();

    /**
     * Return the platform extra config (e.g. "GDI") if available.
     */
    const char* platform_extra_config(const char* config);

    /**
     * Map serif, san-serif, and monospace to the platform-specific font name.
     */
    const char* platform_font_name(const char* name);

    /**
     * Sets the paint to use a platform-independent text renderer
     */
    void set_portable_typeface(SkPaint* paint, const char* name = nullptr,
                               SkTypeface::Style style = SkTypeface::kNormal);

    /**
     * Returns a platform-independent text renderer.
     */
    SkTypeface* create_portable_typeface(const char* name, SkTypeface::Style style);

    /** Call to clean up portable font references. */
    void release_portable_typefaces();

    /**
     *  Call canvas->writePixels() by using the pixels from bitmap, but with an info that claims
     *  the pixels are colorType + alphaType
     */
    void write_pixels(SkCanvas*, const SkBitmap&, int x, int y, SkColorType, SkAlphaType);

    // private to sk_tool_utils
    SkTypeface* create_font(const char* name, SkTypeface::Style);

    /** Returns a newly created CheckerboardShader. */
    SkShader* create_checkerboard_shader(SkColor c1, SkColor c2, int size);

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

    // A helper for inserting a drawtext call into a SkTextBlobBuilder
    void add_to_text_blob(SkTextBlobBuilder* builder, const char* text, const SkPaint& origPaint,
                          SkScalar x, SkScalar y);

    void create_hemi_normal_map(SkBitmap* bm, const SkIRect& dst);

    void create_frustum_normal_map(SkBitmap* bm, const SkIRect& dst);

    void create_tetra_normal_map(SkBitmap* bm, const SkIRect& dst);

    void make_big_path(SkPath& path);

    // Return a blurred version of 'src'. This doesn't use a separable filter
    // so it is slow!
    SkBitmap slow_blur(const SkBitmap& src, float sigma);

    // A helper object to test the topological sorting code (TopoSortBench.cpp & TopoSortTest.cpp)
    class TopoTestNode {
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
        static void AllocNodes(SkTDArray<TopoTestNode*>* graph, int num) {
            graph->setReserve(num);

            for (int i = 0; i < num; ++i) {
                *graph->append() = new TopoTestNode(i);
            }
        }

        static void DeallocNodes(SkTDArray<TopoTestNode*>* graph) {
            for (int i = 0; i < graph->count(); ++i) {
                delete (*graph)[i];
            }
        }

        #ifdef SK_DEBUG
        static void Print(const SkTDArray<TopoTestNode*>& graph) {
            for (int i = 0; i < graph.count(); ++i) {
                SkDebugf("%d, ", graph[i]->id());
            }
            SkDebugf("\n");
        }
        #endif

        // randomize the array
        static void Shuffle(SkTDArray<TopoTestNode*>* graph, SkRandom* rand) {
            for (int i = graph->count()-1; i > 0; --i) {
                int swap = rand->nextU() % (i+1);

                TopoTestNode* tmp = (*graph)[i];
                (*graph)[i] = (*graph)[swap];
                (*graph)[swap] = tmp;
            }
        }

    private:
        int  fID;
        int  fOutputPos;
        bool fTempMark;

        SkTDArray<TopoTestNode*> fDependencies;
    };

}  // namespace sk_tool_utils

#endif  // sk_tool_utils_DEFINED
