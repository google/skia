/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorCubeFilter_DEFINED
#define SkColorCubeFilter_DEFINED

#include "SkColorFilter.h"
#include "SkData.h"
#include "../private/SkOnce.h"
#include "../private/SkTemplates.h"

class SK_API SkColorCubeFilter : public SkColorFilter {
public:
    /** cubeData must containt a 3D data in the form of cube of the size:
     *  cubeDimension * cubeDimension * cubeDimension * sizeof(SkColor)
     *  This cube contains a transform where (x,y,z) maps to the (r,g,b).
     *  The alpha components of the colors must be 0xFF.
     */
    static sk_sp<SkColorFilter> Make(sk_sp<SkData> cubeData, int cubeDimension);

#ifdef SK_SUPPORT_LEGACY_COLORFILTER_PTR
    static SkColorFilter* Create(SkData* cubeData, int cubeDimension);
#endif

    void filterSpan(const SkPMColor src[], int count, SkPMColor[]) const override;
    uint32_t getFlags() const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorCubeFilter)

protected:
    SkColorCubeFilter(sk_sp<SkData> cubeData, int cubeDimension);
    void flatten(SkWriteBuffer&) const override;

private:
    /** The cache is initialized on-demand when getProcessingLuts is called.
     */
    class ColorCubeProcesingCache {
    public:
        ColorCubeProcesingCache(int cubeDimension);

        void getProcessingLuts(const int* (*colorToIndex)[2],
                               const SkScalar* (*colorToFactors)[2],
                               const SkScalar** colorToScalar);

        int cubeDimension() const { return fCubeDimension; }

    private:
        // Working pointers. If any of these is NULL,
        // we need to recompute the corresponding cache values.
        int* fColorToIndex[2];
        SkScalar* fColorToFactors[2];
        SkScalar* fColorToScalar;

        SkAutoTMalloc<uint8_t> fLutStorage;

        const int fCubeDimension;

        // Make sure we only initialize the caches once.
        SkOnce fLutsInitOnce;

        static void initProcessingLuts(ColorCubeProcesingCache* cache);
    };

    sk_sp<SkData> fCubeData;
    int32_t fUniqueID;

    mutable ColorCubeProcesingCache fCache;

    typedef SkColorFilter INHERITED;
};

#endif
