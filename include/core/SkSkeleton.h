/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSkeleton_DEFINED
#define SkSkeleton_DEFINED

#include "SkMatrix.h"
#include "SkData.h"
#include "SkRefCnt.h"
#include <vector>

class SkSkeleton : public SkNVRefCnt<SkSkeleton> {
public:
    // Data for bone attachment weight.
    struct Attachment {
        int   index;
        float weight;
    };

    /**
     * Returns the number of bones in the animation system.
     *
     * @return  size of fBones
     */
    int getNumBones() const { return fBones.size(); }

    /**
     * Returns a pointer to the bone data in the animation system.
     *
     * @return  data pointer of fBones
     */
    const SkMatrix* getBones() const { return fBones.data(); }

    /**
     * Set the bone data. Only takes up to 100 bones due to GPU memory limitations.
     *
     * @param bones     pointer to the bone data
     * @param numBones  number of matrices in the data
     */
    void setBones(const SkMatrix* bones, int numBones);

    /**
     *  Pack the animation object into a byte buffer. This can be used to recreate the vertices
     *  by calling Decode() with the buffer.
     */
    sk_sp<SkData> encode() const;

    /**
     *  Recreate an animation from a buffer previously created by calling encode().
     *  Returns null if the data is corrupt or the length is incorrect for the contents.
     */
    static sk_sp<SkSkeleton> Decode(const void* buffer, size_t length);

    static sk_sp<SkSkeleton> Make(const SkMatrix* bones, int numBones);
    static sk_sp<SkSkeleton> Make();

private:
    SkSkeleton() {}

    std::vector<SkMatrix> fBones;
};

#endif
