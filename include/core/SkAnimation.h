/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnimation_DEFINED
#define SkAnimation_DEFINED

#include "SkMatrix.h"
#include <vector>

class SkAnimation {
public:
    // Data for bone attachment weight.
    struct Attachment {
        int   index;
        float weight;
    };

    SkAnimation();
    ~SkAnimation();

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
    void setBones(SkMatrix* bones, int numBones);

private:
    std::vector<SkMatrix> fBones;
};

#endif
