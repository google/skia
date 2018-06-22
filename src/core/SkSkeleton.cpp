/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSkeleton.h"
#include "SkData.h"
#include "SkReader32.h"
#include "SkWriter32.h"

void SkSkeleton::setBones(const SkMatrix* bones, int numBones) {
    // Don't do anything if numBones is invalid.
    if (numBones < 0) {
        return;
    }

    // Limit the number of bones to 100.
    if (numBones > 100) {
        numBones = 100;
    }

    // Set the data.
    fBones.assign(bones, bones + numBones);
}

sk_sp<SkData> SkSkeleton::encode() const {
    // Store the matrices.
    const size_t size = sizeof(SkMatrix) * fBones.size();
    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    SkWriter32 writer(data->writable_data(), data->size());

    // Write the matrices.
    for (auto matrix : fBones) {
        writer.writeMatrix(matrix);
    }

    return data;
}

sk_sp<SkSkeleton> SkSkeleton::Decode(const void* data, size_t length) {
    // Determine how many matrices there are.
    const size_t numBones = length / sizeof(SkMatrix);

    // Allocate space.
    sk_sp<SkSkeleton> animation(new SkSkeleton());
    animation->fBones.reserve(numBones);

    // Read the matrices.
    SkReader32 reader(data, length);
    SkMatrix matrix;
    for (uint32_t i = 0; i < numBones; i ++) {
        reader.readMatrix(&matrix);
        animation->fBones.push_back(matrix);
    }

    return animation;
}

sk_sp<SkSkeleton> SkSkeleton::Make() {
    return sk_sp<SkSkeleton>(new SkSkeleton());
}

sk_sp<SkSkeleton> SkSkeleton::Make(const SkMatrix* bones, int numBones) {
    SkSkeleton* animation = new SkSkeleton();
    animation->setBones(bones, numBones);
    return sk_sp<SkSkeleton>(animation);
}
