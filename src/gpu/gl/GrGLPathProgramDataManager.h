/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPathProgramDataManager_DEFINED
#define GrGLPathProgramDataManager_DEFINED

#include "gl/GrGLProgramDataManager.h"

class GrGLPathProgram;
class GrGLPathProgramBuilder;

/** Manages the resources used by a shader program for NVPR rendering.
 */
class GrGLPathProgramDataManager : SkNoncopyable {
public:
    class SeparableVaryingHandle : public GrGLProgramDataManager::ShaderResourceHandle {
    public:
        /*
         * Creates a reference to a separable varying of a GrGLShaderBuilder.  The ref can be used
         * to set the varying with the corresponding GrGLPathProgramDataManager.
         */
        static SeparableVaryingHandle CreateFromSeparableVaryingIndex(int i) {
            return GrGLPathProgramDataManager::SeparableVaryingHandle(i);
        }
        SeparableVaryingHandle() { }
        bool operator==(const SeparableVaryingHandle& other) {
            return other.fValue == fValue;
        }
    private:
        SeparableVaryingHandle(int value) : ShaderResourceHandle(value) { }
        int toProgramDataIndex() const { SkASSERT(isValid()); return fValue; }
        int toShaderBuilderIndex() const { return toProgramDataIndex(); }

        friend class GrGLPathProgramDataManager; // For accessing toProgramDataIndex().
        friend class GrGLPathProcessor; // For accessing toShaderBuilderIndex().
    };

    struct SeparableVaryingInfo {
        GrGLShaderVar fVariable;
        GrGLint       fLocation;
    };

    // This uses an allocator rather than array so that the GrGLShaderVars don't move in memory
    // after they are inserted. Users of GrGLShaderBuilder get refs to the vars and ptrs to their
    // name strings. Otherwise, we'd have to hand out copies.
    typedef GrTAllocator<SeparableVaryingInfo> SeparableVaryingInfoArray;

    GrGLPathProgramDataManager(GrGLGpu*, GrGLuint programID, const SeparableVaryingInfoArray&);

    /** Functions for uploading the varying values.
     */
    void setPathFragmentInputTransform(SeparableVaryingHandle u,
                                       int components,
                                       const SkMatrix& matrix) const;
private:
    enum {
        kUnusedSeparableVarying = -1,
    };
    struct SeparableVarying {
        GrGLint     fLocation;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
        );
    };
    SkTArray<SeparableVarying, true> fSeparableVaryings;
    GrGLGpu* fGpu;
    GrGLuint fProgramID;
    typedef SkNoncopyable INHERITED;
};
#endif
