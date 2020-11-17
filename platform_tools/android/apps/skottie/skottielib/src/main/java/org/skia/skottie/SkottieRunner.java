/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.graphics.SurfaceTexture;
import android.graphics.drawable.Animatable;
import android.opengl.GLUtils;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.SurfaceView;
import android.view.TextureView;

import java.io.InputStream;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

class SkottieRunner {
    private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    private static final int EGL_OPENGL_ES2_BIT = 4;
    private static final int STENCIL_BUFFER_SIZE = 8;
    private static final long TIME_OUT_MS = 10000;
    private static final String LOG_TAG = "SkottiePlayer";

    private static SkottieRunner sInstance;

    private HandlerThread mGLThreadLooper;
    private Handler mGLThread;
    EGL10 mEgl;
    EGLDisplay mEglDisplay;
    EGLConfig mEglConfig;
    EGLContext mEglContext;
    EGLSurface mPBufferSurface;
    private long mNativeProxy;

    static {
        System.loadLibrary("skottie_android");
    }
    /**
     * Gets SkottieRunner singleton instance.
     */
    public static synchronized SkottieRunner getInstance() {
        if (sInstance == null) {
            sInstance = new SkottieRunner();
        }
        return sInstance;
    }

    /**
     * Create a new animation by feeding data from "is" and replaying in a TextureView.
     * TextureView is tracked internally for SurfaceTexture state.
     */
    public SkottieAnimation createAnimation(TextureView view, InputStream is, int backgroundColor, int repeatCount) {
        return new SkottieAnimation(view, is, backgroundColor, repeatCount);
    }

    /**
     * Create a new animation by feeding data from "is" and replaying in a SurfaceTexture.
     * SurfaceTexture is possibly taken from a TextureView and can be updated with
     * updateAnimationSurface.
     */
    public SkottieAnimation createAnimation(SurfaceTexture surfaceTexture, InputStream is) {
        return new SkottieAnimation(surfaceTexture, is);
    }

    /**
     * Create a new animation by feeding data from "is" and replaying in a SurfaceView.
     * State is controlled internally by SurfaceHolder.
     */
    public SkottieAnimation createAnimation(SurfaceView view, InputStream is, int backgroundColor, int repeatCount) {
        return new SkottieAnimation(view, is, backgroundColor, repeatCount);
    }

    /**
     * Pass a new SurfaceTexture: use this method only if managing TextureView outside
     * SkottieRunner.
     */
    public void updateAnimationSurface(Animatable animation, SurfaceTexture surfaceTexture,
                                       int width, int height) {
        try {
            runOnGLThread(() -> {
                ((SkottieAnimation) animation).setSurfaceTexture(surfaceTexture);
                ((SkottieAnimation) animation).updateSurface(width, height);
            });
        }
        catch (Throwable t) {
            Log.e(LOG_TAG, "update failed", t);
            throw new RuntimeException(t);
        }
    }

    private SkottieRunner()
    {
        mGLThreadLooper = new HandlerThread("SkottieAnimator");
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

    long getNativeProxy() { return mNativeProxy; }

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

    void runOnGLThread(Runnable r) throws Throwable {
        runOnGLThread(r, false);
    }

    private void runOnGLThread(Runnable r, boolean postAtFront) throws Throwable {

        CountDownLatch fence = new CountDownLatch(1);
        RunSignalAndCatch wrapper = new RunSignalAndCatch(r, fence);
        if (postAtFront) {
            mGLThread.postAtFrontOfQueue(wrapper);
        } else {
            mGLThread.post(wrapper);
        }
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
            Log.e(LOG_TAG, "initGl failed", t);
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

            mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE,  EGL10.EGL_NO_SURFACE,
                    EGL10.EGL_NO_CONTEXT);

            mEgl.eglTerminate(mEglDisplay);
            mEglDisplay = null;
        }
    }

    private static native long nCreateProxy();
    private static native void nDeleteProxy(long nativeProxy);
}

