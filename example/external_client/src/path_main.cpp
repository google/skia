/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/pathops/SkPathOps.h"

#include <cstdio>

int main(int argc, char** argv) {
    SkPathBuilder pb;
    pb.moveTo(10, 10);
    pb.lineTo(15, 5);
    pb.lineTo(20, 10);
    pb.close();
    SkPath path1 = pb.detach();

    pb.moveTo(12, 12);
    pb.lineTo(18, 6);
    pb.lineTo(24, 12);
    pb.close();
    SkPath path2 = pb.detach();

    SkPath combined;
    if (Op(path1, path2, kIntersect_SkPathOp, &combined)) {
        printf("Success: \n");
        combined.dump();
        printf("\n");
    } else {
        printf("Operation failed\n");
    }
}
