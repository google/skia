/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTBackendProcessorFactory_DEFINED
#define GrTBackendProcessorFactory_DEFINED

#include "GrBackendProcessorFactory.h"

/**
 * Implements GrBackendProcessorFactory for a GrProcessor subclass as a singleton. This can be used
 * by most GrProcessor subclasses to implement the GrProcessor::getFactory() method:
 *
 * const GrBackendProcessorFactory& MyProcessor::getFactory() const {
 *     return GrTBackendProcessorFactory<MyProcessor>::getInstance();
 * }
 *
 * Using this class requires that the GrProcessor subclass always produces the same GrGLProcessor
 * subclass. Additionally, it adds the following requirements to the GrProcessor and GrGLProcessor
 * subclasses:
 *
 * 1. The GrGLProcessor used by GrProcessor subclass MyProcessor must be named or typedef'ed to
 *    MyProcessor::GLProcessor.
 * 2. MyProcessor::GLProcessor must have a static function:
        void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder* b)
 *    which generates a key that maps 1 to 1 with code variations emitted by
 *    MyProcessor::GLProcessor::emitCode().
 * 3. MyProcessor must have a static function:
 *      const char* Name()
 *    which returns a human-readable name for the processor.
 */
template <class ProcessorClass, class BackEnd, class ProcessorBase, class GLProcessorBase>
class GrTBackendProcessorFactory : public BackEnd {
public:
    typedef typename ProcessorClass::GLProcessor GLProcessor;

    /** Returns a human-readable name for the processor. Implemented using GLProcessor::Name as
     *  described in this class's comment. */
    virtual const char* name() const SK_OVERRIDE { return ProcessorClass::Name(); }


    /** Implemented using GLProcessor::GenKey as described in this class's comment. */
    virtual void getGLProcessorKey(const GrProcessor& processor,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE {
        GLProcessor::GenKey(processor, caps, b);
    }

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrProcessor; caller is responsible for deleting
        the object. */
    virtual GLProcessorBase* createGLInstance(const ProcessorBase& processor) const SK_OVERRIDE {
        return SkNEW_ARGS(GLProcessor, (*this, processor));
    }

    /** This class is a singleton. This function returns the single instance. */
    static const BackEnd& getInstance() {
        static SkAlignedSTStorage<1, GrTBackendProcessorFactory> gInstanceMem;
        static const GrTBackendProcessorFactory* gInstance;
        if (!gInstance) {
            gInstance = SkNEW_PLACEMENT(gInstanceMem.get(),
                                        GrTBackendProcessorFactory);
        }
        return *gInstance;
    }

protected:
    GrTBackendProcessorFactory() {}
};

/*
 * Every processor so far derives from one of the following subclasses of
 * GrTBackendProcessorFactory. All of this machinery is necessary to ensure that creatGLInstace is
 * typesafe and does not require any casting.
 */
template <class ProcessorClass>
class GrTBackendGeometryProcessorFactory
        : public GrTBackendProcessorFactory<ProcessorClass,
                                            GrBackendGeometryProcessorFactory,
                                            GrGeometryProcessor,
                                            GrGLGeometryProcessor> {
protected:
    GrTBackendGeometryProcessorFactory() {}
};

template <class ProcessorClass>
class GrTBackendFragmentProcessorFactory
        : public GrTBackendProcessorFactory<ProcessorClass,
                                           GrBackendFragmentProcessorFactory,
                                           GrFragmentProcessor,
                                           GrGLFragmentProcessor> {
protected:
    GrTBackendFragmentProcessorFactory() {}
};


#endif
