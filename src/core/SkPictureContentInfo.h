/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureContentInfo_DEFINED
#define SkPictureContentInfo_DEFINED

class GrContext;

class SkPictureContentInfo {
public:
    SkPictureContentInfo() { this->reset(); }
    SkPictureContentInfo(const SkPictureContentInfo& src) { this->set(src); }

    int numOperations() const { return fNumOperations; }
    bool hasText() const { return fNumTexts > 0; }
    bool suitableForGpuRasterization(GrContext* context, const char **reason,
                                     int sampleCount) const;

    void addOperation() { ++fNumOperations; }

    void onDrawPoints(size_t count, const SkPaint& paint);
    void onDrawPath(const SkPath& path, const SkPaint& paint);
    void onAddPaintPtr(const SkPaint* paint);
    void onDrawText() { ++fNumTexts; }

    void set(const SkPictureContentInfo& src);
    void reset();
    void swap(SkPictureContentInfo* other);

private:
    // Raw count of operations in the picture
    int fNumOperations;
    // Count of all forms of drawText
    int fNumTexts;

    // This field is incremented every time a paint with a path effect is
    // used (i.e., it is not a de-duplicated count)
    int fNumPaintWithPathEffectUses;
    // This field is incremented every time a paint with a path effect that is
    // dashed, we are drawing a line, and we can use the gpu fast path
    int fNumFastPathDashEffects;
    // This field is incremented every time an anti-aliased drawPath call is
    // issued with a concave path
    int fNumAAConcavePaths;
    // This field is incremented every time a drawPath call is
    // issued for a hairline stroked concave path.
    int fNumAAHairlineConcavePaths;
};

#endif
