/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentStage_DEFINED
#define GrFragmentStage_DEFINED

#include "GrFragmentProcessor.h"

/**
 * Wraps a GrFragmentProcessor, basically a copyable SkAutoTUnref
 */
class GrFragmentStage {
public:
    explicit GrFragmentStage(const GrFragmentProcessor* proc) : fProc(SkRef(proc)) {}

    GrFragmentStage(const GrFragmentStage& other) { fProc.reset(SkRef(other.fProc.get())); }

    const GrFragmentProcessor* processor() const { return fProc.get(); }

    bool operator==(const GrFragmentStage& that) const {
        return this->processor() == that.processor();
    }

    bool operator!=(const GrFragmentStage& that) const { return !(*this == that); }

protected:
    SkAutoTUnref<const GrFragmentProcessor> fProc;
};

#endif
