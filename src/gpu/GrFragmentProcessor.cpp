
/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
#include "gl/GrGLFragmentProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

GrFragmentProcessor::~GrFragmentProcessor() {
    // If we got here then our ref count must have reached zero, so we will have converted refs
    // to pending executions for all children.
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        fChildProcessors[i]->completedExecution();
    }
}

bool GrFragmentProcessor::isEqual(const GrFragmentProcessor& that,
                                  bool ignoreCoordTransforms) const {
    if (this->classID() != that.classID() ||
        !this->hasSameTextureAccesses(that)) {
        return false;
    }
    if (ignoreCoordTransforms) {
        if (this->numTransforms() != that.numTransforms()) {
            return false;
        }
    } else if (!this->hasSameTransforms(that)) {
        return false;
    }
    if (!this->onIsEqual(that)) {
        return false;
    }
    if (this->numChildProcessors() != that.numChildProcessors()) {
        return false;
    }
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        if (!this->childProcessor(i).isEqual(that.childProcessor(i), ignoreCoordTransforms)) {
            return false;
        }
    }
    return true;
}

GrGLFragmentProcessor* GrFragmentProcessor::createGLInstance() const {
    GrGLFragmentProcessor* glFragProc = this->onCreateGLInstance();
    glFragProc->fChildProcessors.push_back_n(fChildProcessors.count());
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        glFragProc->fChildProcessors[i] = fChildProcessors[i]->createGLInstance();
    }
    return glFragProc;
}

void GrFragmentProcessor::addTextureAccess(const GrTextureAccess* textureAccess) {
    // Can't add texture accesses after registering any children since their texture accesses have
    // already been bubbled up into our fTextureAccesses array
    SkASSERT(fChildProcessors.empty());

    INHERITED::addTextureAccess(textureAccess);
    fNumTexturesExclChildren++;
}

void GrFragmentProcessor::addCoordTransform(const GrCoordTransform* transform) {
    // Can't add transforms after registering any children since their transforms have already been
    // bubbled up into our fCoordTransforms array
    SkASSERT(fChildProcessors.empty());

    fCoordTransforms.push_back(transform);
    fUsesLocalCoords = fUsesLocalCoords || transform->sourceCoords() == kLocal_GrCoordSet;
    SkDEBUGCODE(transform->setInProcessor();)
    fNumTransformsExclChildren++;
}

int GrFragmentProcessor::registerChildProcessor(const GrFragmentProcessor* child) {
    // Append the child's transforms to our transforms array and the child's textures array to our
    // textures array
    if (!child->fCoordTransforms.empty()) {
        fCoordTransforms.push_back_n(child->fCoordTransforms.count(),
                                     child->fCoordTransforms.begin());
    }
    if (!child->fTextureAccesses.empty()) {
        fTextureAccesses.push_back_n(child->fTextureAccesses.count(),
                                     child->fTextureAccesses.begin());
    }

    int index = fChildProcessors.count();
    fChildProcessors.push_back(SkRef(child));

    if (child->willReadFragmentPosition()) {
        this->setWillReadFragmentPosition();
    }

    if (child->usesLocalCoords()) {
        fUsesLocalCoords = true;
    }

    return index;
}

void GrFragmentProcessor::notifyRefCntIsZero() const {
    // See comment above GrProgramElement for a detailed explanation of why we do this.
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        fChildProcessors[i]->addPendingExecution();
        fChildProcessors[i]->unref();
    }
}

bool GrFragmentProcessor::hasSameTransforms(const GrFragmentProcessor& that) const {
    if (this->numTransforms() != that.numTransforms()) {
        return false;
    }
    int count = this->numTransforms();
    for (int i = 0; i < count; ++i) {
        if (this->coordTransform(i) != that.coordTransform(i)) {
            return false;
        }
    }
    return true;
}

const GrFragmentProcessor* GrFragmentProcessor::MulOuputByInputAlpha(
    const GrFragmentProcessor* fp) {
    return GrXfermodeFragmentProcessor::CreateFromDstProcessor(fp, SkXfermode::kDstIn_Mode);
}

