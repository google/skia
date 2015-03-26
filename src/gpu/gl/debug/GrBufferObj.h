
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBufferObj_DEFINED
#define GrBufferObj_DEFINED

#include "GrFakeRefObj.h"
#include "../GrGLDefines.h"

////////////////////////////////////////////////////////////////////////////////
class GrBufferObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrBufferObj);

public:
    GrBufferObj()
        : GrFakeRefObj()
        , fDataPtr(NULL)
        , fMapped(false)
        , fBound(false)
        , fSize(0)
        , fUsage(GR_GL_STATIC_DRAW) {
    }
    virtual ~GrBufferObj() {
        delete[] fDataPtr;
    }

    void access() {
        // cannot access the buffer if it is currently mapped
        GrAlwaysAssert(!fMapped);
    }

    void setMapped(GrGLintptr offset, GrGLsizeiptr length) {
        fMapped = true;
        fMappedOffset = offset;
        fMappedLength = length;
    }
    void resetMapped()           { fMapped = false; }
    bool getMapped() const       { return fMapped; }
    GrGLintptr getMappedOffset() const { return fMappedOffset; }
    GrGLsizeiptr getMappedLength() const { return fMappedLength; }

    void setBound()              { fBound = true; }
    void resetBound()            { fBound = false; }
    bool getBound() const        { return fBound; }

    void allocate(GrGLsizeiptr size, const GrGLchar *dataPtr);
    GrGLsizeiptr getSize() const { return fSize; }
    GrGLchar *getDataPtr()       { return fDataPtr; }

    void setUsage(GrGLint usage) { fUsage = usage; }
    GrGLint getUsage() const     { return fUsage; }

    void deleteAction() override;

protected:
private:

    GrGLchar*    fDataPtr;
    bool         fMapped;       // is the buffer object mapped via "glMapBuffer[Range]"?
    GrGLintptr   fMappedOffset; // the offset of the buffer range that is mapped
    GrGLsizeiptr fMappedLength; // the size of the buffer range that is mapped
    bool         fBound;        // is the buffer object bound via "glBindBuffer"?
    GrGLsizeiptr fSize;         // size in bytes
    GrGLint      fUsage;        // one of: GL_STREAM_DRAW,
                                //         GL_STATIC_DRAW,
                                //         GL_DYNAMIC_DRAW

    typedef GrFakeRefObj INHERITED;
};

#endif // GrBufferObj_DEFINED
