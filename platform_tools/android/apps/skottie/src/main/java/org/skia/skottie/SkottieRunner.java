/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.graphics.SurfaceTexture;
import android.opengl.GLUtils;
import android.os.Handler;
import android.os.HandlerThread;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

interface DrawFrame {
    void drawFrame();
}

public class SkottieRunner {
    private final static long TIME_OUT_MS = 10000;
    private HandlerThread mGLThreadLooper;
    private Handler mGLThread;
    private EGL10 mEgl;
    private EGLDisplay mEglDisplay;
    private EGLConfig mEglConfig;
    private EGLContext mEglContext;
    private EGLSurface mPBufferSurface;
    private long mNativeProxy;

    static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    static final int EGL_OPENGL_ES2_BIT = 4;
    static final int STENCIL_BUFFER_SIZE = 8;

    private static SkottieRunner sInstance;

    public static synchronized SkottieRunner getInstance() {
        if (sInstance == null) {
            sInstance = new SkottieRunner();
        }
        return sInstance;
    }

    private SkottieRunner()
    {
        mGLThreadLooper = new HandlerThread("GLThread");
        mGLThreadLooper.start();
        mGLThread = new Handler(mGLThreadLooper.getLooper());
        initGl();
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            runOnGLThread(this::doFinishGL);
        } finally {
            super.finalize();
        }
    }

    public long getNativeProxy() { return mNativeProxy; }

    class EGLSurfaceCreate implements Runnable {
        public EGLSurface mEglSurface;
        SurfaceTexture mSurface;

        public EGLSurfaceCreate(SurfaceTexture surface) {
            mSurface = surface;
        }

        @Override
        public void run() {
            mEglSurface = mEgl.eglCreateWindowSurface(mEglDisplay, mEglConfig,
                    mSurface, null);
            if (mEglSurface == null || mEglSurface == EGL10.EGL_NO_SURFACE) {
                int error = mEgl.eglGetError();
                throw new RuntimeException("createWindowSurface failed "
                        + GLUtils.getEGLErrorString(error));
            }
        }
    }

    public EGLSurface createEGLSurface(SurfaceTexture surface) throws Throwable {
        EGLSurfaceCreate createSurface = new EGLSurfaceCreate(surface);
        runOnGLThread(createSurface);
        return createSurface.mEglSurface;
    }

    public void destroyEGLSurface(EGLSurface eglSurface) throws Throwable {
        runOnGLThread(()-> {
            if (eglSurface != null) {
                mEgl.eglDestroySurface(mEglDisplay, eglSurface);
            }
        });
    }

    public void drawFrame(EGLSurface surface, DrawFrame frame) throws Throwable {
        runOnGLThread(() -> {
            if (!mEgl.eglMakeCurrent(mEglDisplay, surface, surface, mEglContext)) {
                throw new RuntimeException("eglMakeCurrent failed "
                        + GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }

            frame.drawFrame();
            if (!mEgl.eglSwapBuffers(mEglDisplay, surface)) {
                throw new RuntimeException("Cannot swap buffers");
            }
        });
    }

    private class RunSignalAndCatch implements Runnable {
        public Throwable error;
        private Runnable mRunnable;
        private CountDownLatch mFence;

        RunSignalAndCatch(Runnable run, CountDownLatch fence) {
            mRunnable = run;
            mFence = fence;
        }

        @Override
        public void run() {
            try {
                mRunnable.run();
            } catch (Throwable t) {
                error = t;
            } finally {
                mFence.countDown();
            }
        }
    }

    private void runOnGLThread(Runnable r) throws Throwable {
        CountDownLatch fence = new CountDownLatch(1);
        RunSignalAndCatch wrapper = new RunSignalAndCatch(r, fence);
        mGLThread.post(wrapper);
        if (!fence.await(TIME_OUT_MS, TimeUnit.MILLISECONDS)) {
            throw new TimeoutException();
        }
        if (wrapper.error != null) {
            throw wrapper.error;
        }
    }

    private void initGl()
    {
        try {
            runOnGLThread(mDoInitGL);
        }
        catch (Throwable t) {
            throw new RuntimeException(t);
        }
    }

    private Runnable mDoInitGL = new Runnable() {
        @Override
        public void run() {
            mEgl = (EGL10) EGLContext.getEGL();

            mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
            if (mEglDisplay == EGL10.EGL_NO_DISPLAY) {
                throw new RuntimeException("eglGetDisplay failed "
                        + GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }

            int[] version = new int[2];
            if (!mEgl.eglInitialize(mEglDisplay, version)) {
                throw new RuntimeException("eglInitialize failed " +
                        GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }

            mEglConfig = chooseEglConfig();
            if (mEglConfig == null) {
                throw new RuntimeException("eglConfig not initialized");
            }

            mEglContext = createContext(mEgl, mEglDisplay, mEglConfig);

            int[] attribs = new int[] {
                    EGL10.EGL_WIDTH, 1,
                    EGL10.EGL_HEIGHT, 1,
                    EGL10.EGL_NONE
            };

            mPBufferSurface = mEgl.eglCreatePbufferSurface(mEglDisplay, mEglConfig, attribs);
            if (mPBufferSurface == null || mPBufferSurface == EGL10.EGL_NO_SURFACE) {
                int error = mEgl.eglGetError();
                throw new RuntimeException("createPbufferSurface failed "
                        + GLUtils.getEGLErrorString(error));
            }

            if (!mEgl.eglMakeCurrent(mEglDisplay, mPBufferSurface, mPBufferSurface, mEglContext)) {
                throw new RuntimeException("eglMakeCurrent failed "
                        + GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }

            mNativeProxy = nCreateProxy();
        }
    };

    EGLContext createContext(EGL10 egl, EGLDisplay eglDisplay, EGLConfig eglConfig) {
        int[] attrib_list = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
        return egl.eglCreateContext(eglDisplay, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
    }

    private EGLConfig chooseEglConfig() {
        int[] configsCount = new int[1];
        EGLConfig[] configs = new EGLConfig[1];
        int[] configSpec = getConfig();
        if (!mEgl.eglChooseConfig(mEglDisplay, configSpec, configs, 1, configsCount)) {
            throw new IllegalArgumentException("eglChooseConfig failed " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        } else if (configsCount[0] > 0) {
            return configs[0];
        }
        return null;
    }

    private int[] getConfig() {
        return new int[] {
                EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL10.EGL_RED_SIZE, 8,
                EGL10.EGL_GREEN_SIZE, 8,
                EGL10.EGL_BLUE_SIZE, 8,
                EGL10.EGL_ALPHA_SIZE, 8,
                EGL10.EGL_DEPTH_SIZE, 0,
                EGL10.EGL_STENCIL_SIZE, STENCIL_BUFFER_SIZE,
                EGL10.EGL_NONE
        };
    }

    private void doFinishGL() {
        nDeleteProxy(mNativeProxy);
        mNativeProxy = 0;
        if (mEglDisplay != null) {
            if (mEglContext != null) {
                mEgl.eglDestroyContext(mEglDisplay, mEglContext);
                mEglContext = null;
            }
            if (mPBufferSurface != null) {
                mEgl.eglDestroySurface(mEglDisplay, mPBufferSurface);
                mPBufferSurface = null;
            }

            mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE,  EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);

            mEgl.eglTerminate(mEglDisplay);
            mEglDisplay = null;
        }
    }

    private static native long nCreateProxy();
    private static native void nDeleteProxy(long nativeProxy);
}

