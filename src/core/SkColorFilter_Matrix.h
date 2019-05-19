/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilter_Matrix_DEFINED
#define SkColorFilter_Matrix_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"

class SkColorFilter_Matrix : public SkColorFilter {
public:
    SkColorFilter_Matrix() {}
    explicit SkColorFilter_Matrix(const float array[20]);

    uint32_t getFlags() const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo&) const override;
#endif

    static void RegisterFlattenables();

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMatrix(float matrix[20]) const override;

private:
    SK_FLATTENABLE_HOOKS(SkColorFilter_Matrix)

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    float       fMatrix[20];
    uint32_t    fFlags;

    void initState();

    typedef SkColorFilter INHERITED;
};

#endif
