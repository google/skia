#include "SkGLDevice_FBO.h"
#include "SkRegion.h"

SkGLDevice_FBO::SkGLDevice_FBO(const SkBitmap& bitmap, bool offscreen)
        : SkGLDevice(bitmap, offscreen) {
    fFBO = 0;
    fTextureID = 0;

    if (offscreen) {
        int nw = SkNextPow2(bitmap.rowBytesAsPixels());
        int nh = SkNextPow2(bitmap.height());
        
        glGenFramebuffersEXT(1, &fFBO);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fFBO);

        glGenTextures(1, &fTextureID);
        glBindTexture(GL_TEXTURE_2D, fTextureID);
        SkGL::SetTexParamsClamp(false);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nw, nh, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                  GL_TEXTURE_2D, fTextureID, 0);
        GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
            SkDebugf("-- glCheckFramebufferStatusEXT %x\n", status);
        }

        // now reset back to "normal" drawing target
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
}

SkGLDevice_FBO::~SkGLDevice_FBO() {
    if (fTextureID) {
        glDeleteTextures(1, &fTextureID);
    }
    if (fFBO) {
        glDeleteFramebuffersEXT(1, &fFBO);
    }
}

SkGLDevice::TexOrientation SkGLDevice_FBO::bindDeviceAsTexture() {
    if (fTextureID) {
        glBindTexture(GL_TEXTURE_2D, fTextureID);
        return kBottomToTop_TexOrientation;
    }
    return kNo_TexOrientation;
}

void SkGLDevice_FBO::gainFocus(SkCanvas* canvas) {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fFBO);

    // now we're ready for the viewport and projection matrix
    this->INHERITED::gainFocus(canvas);
}

