/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLFW/glfw3.h"
#include <stdlib.h>
#include <stdio.h>

#include "GrContext.h"

#include "SkCanvas.h"
#include "SkImage.h"
#include "SkRSXform.h"
#include "SkSurface.h"
#include "Timer.h"

GrContext* sContext = nullptr;
SkSurface* sSurface = nullptr;

static void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}


static void init_skia(int w, int h) {
    sContext = GrContext::Create(kOpenGL_GrBackend, 0);
    
    GrBackendRenderTargetDesc desc;
    desc.fWidth = w;
    desc.fHeight = h;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fSampleCnt = 1;
    desc.fStencilBits = 0;
    desc.fRenderTargetHandle = 0;  // assume default framebuffer
 
    sSurface = SkSurface::NewFromBackendRenderTarget(sContext, desc, NULL);
}

static void cleanup_skia() {
    delete sSurface;
    delete sContext;
}

const int kGrid = 100;
const int kWidth = 960;
const int kHeight = 640;

int main(void) {
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
    
    window = glfwCreateWindow(kWidth, kHeight, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    
    init_skia(kWidth, kHeight);
    
    SkAutoTUnref<SkImage> atlas;
    SkRSXform   xform[kGrid*kGrid+1];
    SkRect      tex[kGrid*kGrid+1];
    WallTimer   timer;
    float       times[32];
    int         currentTime;

    SkAutoTUnref<SkData> imageData(SkData::NewFromFileName("ship.png"));
    atlas.reset(SkImage::NewFromEncoded(imageData));
    if (!atlas) {
        SkDebugf("\nCould not decode file ship.png\n");
        
        cleanup_skia();
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    SkScalar anchorX = atlas->width()*0.5f;
    SkScalar anchorY = atlas->height()*0.5f;
    int currIndex = 0;
    for (int x = 0; x < kGrid; x++) {
        for (int y = 0; y < kGrid; y++) {
            float xPos = (x / (kGrid - 1.0)) * kWidth;
            float yPos = (y / (kGrid - 1.0)) * kWidth;
            
            tex[currIndex] = SkRect::MakeLTRB(0.0f, 0.0f, atlas->width(), atlas->height());
            xform[currIndex] = SkRSXform::MakeFromRadians(2.0f, SK_ScalarPI*0.5f,
                                                          xPos, yPos, anchorX, anchorY);
            currIndex++;
        }
    }
    tex[currIndex] = SkRect::MakeLTRB(0.0f, 0.0f, atlas->width(), atlas->height());
    xform[currIndex] = SkRSXform::MakeFromRadians(2.0f, SK_ScalarPI*0.5f,
                                                  kWidth*0.5f, kHeight*0.5f, anchorX, anchorY);
    
    currentTime = 0;
    
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
    
    // Draw to the surface via its SkCanvas.
    SkCanvas* canvas = sSurface->getCanvas();   // We don't manage this pointer's lifetime.
    SkPaint paint;
    paint.setFilterQuality(kLow_SkFilterQuality);
    paint.setColor(SK_ColorWHITE);
    paint.setTextSize(15.0f);

    while (!glfwWindowShouldClose(window)) {
        const float kCosDiff = 0.99984769515f;
        const float kSinDiff = 0.01745240643f;

        timer.start();
        
        glfwPollEvents();
        
        float meanTime = 0.0f;
        for (int i = 0; i < 32; ++i) {
            meanTime += times[i];
        }
        meanTime /= 32.f;
        char outString[64];
        float fps = 1000.f/meanTime;
        sprintf(outString, "fps: %f ms: %f", fps, meanTime);
        
        for (int i = 0; i < kGrid*kGrid+1; ++i) {
            SkScalar c = xform[i].fSCos;
            SkScalar s = xform[i].fSSin;
        
            SkScalar dx = c*anchorX - s*anchorY;
            SkScalar dy = s*anchorX + c*anchorY;

            xform[i].fSCos = kCosDiff*c - kSinDiff*s;
            xform[i].fSSin = kSinDiff*c + kCosDiff*s;
            
            dx -= xform[i].fSCos*anchorX - xform[i].fSSin*anchorY;
            dy -= xform[i].fSSin*anchorX + xform[i].fSCos*anchorY;
            xform[i].fTx += dx;
            xform[i].fTy += dy;
        }
        
        canvas->clear(SK_ColorBLACK);
        canvas->drawAtlas(atlas, xform, tex, nullptr, kGrid*kGrid+1, SkXfermode::kSrcOver_Mode,
                          nullptr, &paint);
        canvas->drawText(outString, strlen(outString), 100.f, 100.f, paint);
        
        canvas->flush();
        
        timer.end();
        times[currentTime] = (float)(timer.fWall);
        currentTime = (currentTime + 1) & 0x1f;
        
        glfwSwapBuffers(window);
    }
  
    cleanup_skia();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
