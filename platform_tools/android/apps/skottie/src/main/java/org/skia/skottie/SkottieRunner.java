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
import android.view.Choreographer;
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

public class SkottieRunner {
    private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    private static final int EGL_OPENGL_ES2_BIT = 4;
    private static final int STENCIL_BUFFER_SIZE = 8;
    private static final long TIME_OUT_MS = 10000;
    private static final String LOG_TAG = "SkottiePlayer";

    private static SkottieRunner sInstance;

    private HandlerThread mGLThreadLooper;
    private Handler mGLThread;
    private EGL10 mEgl;
    private EGLDisplay mEglDisplay;
    private EGLConfig mEglConfig;
    private EGLContext mEglContext;
    private EGLSurface mPBufferSurface;
    private long mNativeProxy;

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
    public SkottieAnimation createAnimation(TextureView view, InputStream is) {
        return new SkottieAnimationImpl(view, is);
    }

    /**
     * Create a new animation by feeding data from "is" and replaying in a SurfaceTexture.
     * SurfaceTexture is possibly taken from a TextureView and can be updated with
     * updateAnimationSurface.
     */
    public SkottieAnimation createAnimation(SurfaceTexture surfaceTexture, InputStream is) {
        return new SkottieAnimationImpl(surfaceTexture, is);
    }

    /**
     * Pass a new SurfaceTexture: use this method only if managing TextureView outside
     * SkottieRunner.
     */
    public void updateAnimationSurface(Animatable animation, SurfaceTexture surfaceTexture,
                                       int width, int height) {
        ((SkottieAnimationImpl) animation).updateSurface(surfaceTexture, width, height);
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

    private long getNativeProxy() { return mNativeProxy; }

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

    private class SkottieAnimationImpl implements SkottieAnimation, Choreographer.FrameCallback,
            TextureView.SurfaceTextureListener {
        boolean mIsRunning = false;
        SurfaceTexture mSurfaceTexture;
        EGLSurface mEglSurface;
        boolean mNewSurface = false;

        private int mSurfaceWidth = 0;
        private int mSurfaceHeight = 0;
        private long mNativeProxy;
        private InputStream mInputStream;
        private byte[]  mTempStorage;
        private long mDuration;  // duration in ms of the animation
        private float mProgress; // animation progress in the range of 0.0f to 1.0f
        private long mAnimationStartTime; // time in System.nanoTime units, when started

        SkottieAnimationImpl(SurfaceTexture surfaceTexture, InputStream is) {
            init(surfaceTexture, is);
        }

        SkottieAnimationImpl(TextureView view, InputStream is) {
            init(view.getSurfaceTexture(), is);
            view.setSurfaceTextureListener(this);
        }

        private void init(SurfaceTexture surfaceTexture, InputStream is) {
            mTempStorage = new byte[16 * 1024];
            mInputStream = is;
            long proxy = SkottieRunner.getInstance().getNativeProxy();
            mNativeProxy = nCreateProxy(proxy, mInputStream, mTempStorage);
            mSurfaceTexture = surfaceTexture;
            mDuration = nGetDuration(mNativeProxy);
            mProgress = 0f;
        }

        @Override
        protected void finalize() throws Throwable {
            try {
                stop();
                nDeleteProxy(mNativeProxy);
                mNativeProxy = 0;
            } finally {
                super.finalize();
            }
        }

        public void updateSurface(SurfaceTexture surfaceTexture, int width, int height) {
            try {
                runOnGLThread(() -> {
                    mSurfaceTexture = surfaceTexture;
                    mSurfaceWidth = width;
                    mSurfaceHeight = height;
                    mNewSurface = true;

                    drawFrame();
                });
            }
            catch (Throwable t) {
                Log.e(LOG_TAG, "updateSurface failed", t);
                throw new RuntimeException(t);
            }
        }

        @Override
        public void start() {
            try {
                runOnGLThread(() -> {
                    if (!mIsRunning) {
                        long currentTime = System.nanoTime();
                        mAnimationStartTime = currentTime - (long)(1000000 * mDuration * mProgress);
                        mIsRunning = true;
                        mNewSurface = true;
                        doFrame(currentTime);
                    }
                });
            }
            catch (Throwable t) {
                Log.e(LOG_TAG, "start failed", t);
                throw new RuntimeException(t);
            }
        }

        @Override
        public void stop() {
            try {
                runOnGLThread(() -> {
                    mIsRunning = false;
                    if (mEglSurface != null) {
                        // Ensure we always have a valid surface & context.
                        mEgl.eglMakeCurrent(mEglDisplay, mPBufferSurface, mPBufferSurface,
                                mEglContext);
                        mEgl.eglDestroySurface(mEglDisplay, mEglSurface);
                        mEglSurface = null;
                    }
                });
            }
            catch (Throwable t) {
                Log.e(LOG_TAG, "stop failed", t);
                throw new RuntimeException(t);
            }
        }

        @Override
        public boolean isRunning() {
            return mIsRunning;
        }

        @Override
        public long getDuration() {
            return mDuration;
        }

        @Override
        public void setProgress(float progress) {
            try {
                runOnGLThread(() -> {
                    mProgress = progress;
                    if (mIsRunning) {
                        mAnimationStartTime = System.nanoTime()
                                - (long)(1000000 * mDuration * mProgress);
                    }
                    drawFrame();
                });
            }
            catch (Throwable t) {
                Log.e(LOG_TAG, "setProgress failed", t);
                throw new RuntimeException(t);
            }
        }

        @Override
        public float getProgress() {
            return mProgress;
        }

        private void drawFrame() {
            try {
                if (mNewSurface) {
                    // if there is a new SurfaceTexture, we need to recreate the EGL surface.
                    if (mEglSurface != null) {
                        mEgl.eglDestroySurface(mEglDisplay, mEglSurface);
                        mEglSurface = null;
                    }
                    mNewSurface = false;
                }

                if (mEglSurface == null) {
                    if (mSurfaceTexture != null) {
                        mEglSurface = mEgl.eglCreateWindowSurface(mEglDisplay, mEglConfig,
                                mSurfaceTexture, null);
                        if (mEglSurface == null || mEglSurface == EGL10.EGL_NO_SURFACE) {
                            // If failed to create a surface, log an error and stop the animation
                            int error = mEgl.eglGetError();
                            throw new RuntimeException("createWindowSurface failed "
                                    + GLUtils.getEGLErrorString(error));
                        }
                    }
                }

                if (mEglSurface != null) {
                    if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
                        // If eglMakeCurrent failed, recreate EGL surface on next frame.
                        Log.w(LOG_TAG, "eglMakeCurrent failed "
                                + GLUtils.getEGLErrorString(mEgl.eglGetError()));
                        mNewSurface = true;
                        return;
                    }

                    nDrawFrame(mNativeProxy, mSurfaceWidth, mSurfaceHeight, false,
                            mProgress);
                    if (!mEgl.eglSwapBuffers(mEglDisplay, mEglSurface)) {
                        int error = mEgl.eglGetError();
                        if (error == EGL10.EGL_BAD_SURFACE
                                || error == EGL10.EGL_BAD_NATIVE_WINDOW) {
                            // For some reason our surface was destroyed. Recreate EGL surface
                            // on next frame.
                            mNewSurface = true;
                            // This really shouldn't happen, but if it does we can recover easily
                            // by just not trying to use the surface anymore
                            Log.w(LOG_TAG, "swapBuffers failed "
                                    + GLUtils.getEGLErrorString(error));
                            return;
                        }

                        // Some other fatal EGL error happened, log an error and stop the animation.
                        throw new RuntimeException("Cannot swap buffers "
                                + GLUtils.getEGLErrorString(error));
                    }

                    // If animation stopped, release EGL surface.
                    if (!mIsRunning) {
                        // Ensure we always have a valid surface & context.
                        mEgl.eglMakeCurrent(mEglDisplay, mPBufferSurface, mPBufferSurface,
                                mEglContext);
                        mEgl.eglDestroySurface(mEglDisplay, mEglSurface);
                        mEglSurface = null;
                    }
                }
            } catch (Throwable t) {
                Log.e(LOG_TAG, "drawFrame failed", t);
                mIsRunning = false;
            }
        }

        @Override
        public void doFrame(long frameTimeNanos) {
            if (mIsRunning) {
                // Schedule next frame.
                Choreographer.getInstance().postFrameCallback(this);

                // Advance animation.
                long durationNS = mDuration * 1000000;
                long timeSinceAnimationStartNS = frameTimeNanos - mAnimationStartTime;
                long animationProgressNS = timeSinceAnimationStartNS % durationNS;
                mProgress = animationProgressNS / (float)durationNS;
                if (timeSinceAnimationStartNS > durationNS) {
                    mAnimationStartTime += durationNS;  // prevents overflow
                }
            }

            drawFrame();
        }

        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            // will be called on UI thread
            updateSurface(surface, width, height);
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
            // will be called on UI thread
            onSurfaceTextureAvailable(surface, width, height);
        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
            // will be called on UI thread
            onSurfaceTextureAvailable(null, 0, 0);
            return true;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {

        }

        private native long nCreateProxy(long runner, InputStream is, byte[] storage);
        private native void nDeleteProxy(long nativeProxy);
        private native void nDrawFrame(long nativeProxy, int width, int height,
                                       boolean wideColorGamut, float progress);
        private native long nGetDuration(long nativeProxy);
    }

    private static native long nCreateProxy();
    private static native void nDeleteProxy(long nativeProxy);
}

