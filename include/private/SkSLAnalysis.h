/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLAnalysis_DEFINED
#define SkSLAnalysis_DEFINED

#include "include/private/SkSLSampleMatrix.h"

namespace SkSL {

struct Program;
struct Variable;

/**
 * Provides utilities for analyzing SkSL statically before it's composed into a full program.
 */
struct Analysis {
    static SampleMatrix GetSampleMatrix(const Program& program, const Variable& fp);

    static bool IsExplicitlySampled(const Program& program, const Variable& fp);

    static bool ReferencesSampleCoords(const Program& program);
};

}

#endif
