
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkGLContext_DEFINED
#define SkGLContext_DEFINED

#include "GrGLInterface.h"
#include "SkString.h"

/**
 * Create an offscreen opengl context with an RGBA8 / 8bit stencil FBO.
 * Provides a GrGLInterface struct of function pointers for the context.
 */

class SkGLContext : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkGLContext)

    SkGLContext();
    virtual ~SkGLContext();

    /**
     * Initializes the context and makes it current.
     */
    bool init(const int width, const int height);

    int getFBOID() const { return fFBO; }

    const GrGLInterface* gl() const { return fGL; }

    virtual void makeCurrent() const = 0;

    bool hasExtension(const char* extensionName) const;

protected:
    /**
     * Subclass implements this to make a GL context. The returned GrGLInterface
     * should be populated with functions compatible with the context. The
     * format and size of backbuffers does not matter since an FBO will be
     * created.
     */
    virtual const GrGLInterface* createGLContext() = 0;

    /**
     * Subclass should destroy the underlying GL context.
     */
    virtual void destroyGLContext() = 0;

private:
    SkString fExtensionString;
    GrGLuint fFBO;
    GrGLuint fColorBufferID;
    GrGLuint fDepthStencilBufferID;
    const GrGLInterface* fGL;

    typedef SkRefCnt INHERITED;
};

/**
 * Helper macro for using the GL context through the GrGLInterface. Example:
 * SK_GL(glCtx, GenTextures(1, &texID));
 */
#define SK_GL(ctx, X) (ctx).gl()->f ## X

#endif
