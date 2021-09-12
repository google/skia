/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUniformAggregator_DEFINED
#define GrUniformAggregator_DEFINED

#include "include/core/SkString.h"
#include "include/private/SkChecksum.h"
#include "src/gpu/GrProcessor.h"

#include <vector>

/** Collects the uniforms from various processors comprising the shaders of a pipeline/program. */
class GrUniformAggregator {
public:
    class ProcessorUniforms {
    public:
        ProcessorUniforms(ProcessorUniforms&&) = default;
        ProcessorUniforms& operator=(ProcessorUniforms&&) = default;

        /**
         * Finds a uniform name by index. The uniform initially has a generic name. It can
         * optionally be given a descriptive name via the newBaseName param. However, the caller
         * must use the returned name because even if a name is passed the final uniform name will
         * be mangled to be unique.
         */
        const char* getUniformName(size_t index, const char* newBaseName = nullptr) const;

    private:
        ProcessorUniforms(const GrProcessor& p,
                          const SkString& mangleSuffix,
                          GrUniformAggregator* aggregator);

        ProcessorUniforms(const ProcessorUniforms&) = delete;
        ProcessorUniforms& operator=(const ProcessorUniforms&) = delete;

        GrUniformAggregator* fAgg;

        SkString fMangleSuffix;

        size_t fBegin = 0;
        size_t fEnd = 0;

        friend class GrUniformAggregator;
    };

    struct Record {
        SkString           name;
        const GrProcessor* processor        = nullptr;
        size_t             indexInProcessor = -1;

        const GrProcessor::Uniform& uniform() const {
            return processor->uniforms()[indexInProcessor];
        }
    };

    GrUniformAggregator() = default;

    /**
     * Aggregates the uniforms for a processor. This must be called for all processors in a
     * program and must be called in this order: GP, FP0-T, FP1-T, ... XP. FPi-T is a pre-order
     * traversal of the ith FP in the GrPipeline.
     */
    ProcessorUniforms addUniforms(const GrProcessor&, const SkString& mangleSuffix);

    /**
     * Iterable range of all uniform Records across all processors added.
     */
    SkSpan<const Record> records() const {
        return SkSpan<const Record>(fUniforms.data(), fUniforms.size());
    }

    /**
     * Iterable range of Records for a given processor index.
     */
    SkSpan<const Record> processorRecords(size_t processorIndex) const {
        SkASSERT(processorIndex < fProcessors.size());
        size_t size = fProcessors[processorIndex].end - fProcessors[processorIndex].begin;
        return SkSpan<const Record>(fUniforms.data() + fProcessors[processorIndex].begin, size);
    }

    int uniformCount() const { return fUniforms.size(); }

    /**
     * The number of processors whose uniforms have been added to the aggregator, including
     * processors that had no valid uniforms.
     */
    int numProcessors() const { return fProcessors.size(); }

private:
    struct Processor {
        const GrProcessor* processor;
        size_t             begin; // index of first uniform owned by processor in fUniforms.
        size_t             end;   // index of last uniform + 1 owned by processor in fUniforms.
    };
    std::vector<Processor> fProcessors;
    using Records = std::vector<Record>;

    Records fUniforms;
};

#endif
