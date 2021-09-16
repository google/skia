/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrUniformAggregator.h"

using ProcessorUniforms = GrUniformAggregator::ProcessorUniforms;

ProcessorUniforms GrUniformAggregator::addUniforms(const GrProcessor& p,
                                                   const SkString& mangleSuffix) {
    Processor processor{
            &p,
            fUniforms.size(),
            fUniforms.size(),
    };
    for (size_t i = 0; i < p.uniforms().size(); ++i) {
        if (!p.uniforms()[i].isInitialized()) {
            continue;
        }
        // We give every uniform an initial name so it always can be validly declared. When code is
        // emitted the processor can give it a more meaningful name. The actual name doesn't matter,
        // other than for readability.
        SkString unusedName = SkStringPrintf("default_%zu%s", i, mangleSuffix.c_str());
        fUniforms.push_back(Record{std::move(unusedName), &p, i});
        ++processor.end;
    }
    fProcessors.push_back(processor);
    return ProcessorUniforms(p, mangleSuffix, this);
}

//////////////////////////////////////////////////////////////////////////////

ProcessorUniforms::ProcessorUniforms(const GrProcessor& p,
                                     const SkString& mangleSuffix,
                                     GrUniformAggregator* aggregator)
        : fAgg(aggregator), fMangleSuffix(mangleSuffix) {
    for (size_t i = 0; i < fAgg->fProcessors.size(); ++i) {
        if (fAgg->fProcessors[i].processor == &p) {
            fBegin = fAgg->fProcessors[i].begin;
            fEnd   = fAgg->fProcessors[i].end;
            return;
        }
    }
}

const char* ProcessorUniforms::getUniformName(size_t index, const char* newBaseName) const {
    for (size_t i = fBegin; i < fEnd; ++i) {
        if (fAgg->fUniforms[i].indexInProcessor == index) {
            GrUniformAggregator::Record& r = fAgg->fUniforms[i];
            if (newBaseName) {
                SkString mangledName = SkStringPrintf("%s%s", newBaseName, fMangleSuffix.c_str());
                r.name = mangledName;
            }
            return r.name.c_str();
        } else if (fAgg->fUniforms[i].indexInProcessor > index) {
            break;
        }
    }
    return nullptr;
}
