/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStagedProcessorStage_DEFINED
#define GrStagedProcessorStage_DEFINED

#include "GrFragmentProcessor.h"
#include "SkRefCnt.h"

/**
 * Wraps a GrFragmentProcessor, basically a copyable SkAutoTUnref
 * Templatized based on the ref type so backends can use the same wrapper
 */
template<template<typename> class T>
class GrStagedProcessor {
public:
    explicit GrStagedProcessor(const GrFragmentProcessor* proc) : fProc(SkRef(proc)) {}

    GrStagedProcessor(const GrStagedProcessor& other) { fProc.reset(SkRef(other.fProc.get())); }

    const GrFragmentProcessor* processor() const { return fProc.get(); }

    bool operator==(const GrStagedProcessor& that) const {
        return this->processor()->isEqual(*that.processor());
    }

    bool operator!=(const GrStagedProcessor& that) const { return !(*this == that); }

    const char* name() const { return fProc->name(); }

protected:
    GrStagedProcessor() {}

    T<const GrFragmentProcessor> fProc;
};

typedef GrStagedProcessor<SkAutoTUnref> GrFragmentStage;

#endif
