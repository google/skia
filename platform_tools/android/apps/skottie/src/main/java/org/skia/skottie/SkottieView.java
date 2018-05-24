/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.graphics.drawable.Animatable;
import android.view.TextureView;
import android.view.ViewGroup;

import java.io.InputStream;

import javax.microedition.khronos.egl.EGLSurface;

public class SkottieView extends ViewGroup implements TextureView.SurfaceTextureListener, Animatable {

    private final static long TIME_OUT_MS = 10000;
    private final Object mLock = new Object();
    private TextureView mTextureView;
    private SurfaceTexture mSurface;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    private int mSurfaceUpdatedCount;
    boolean mIsRunning = false;
    private long mNativeProxy;
    private InputStream mInputStream;
    private byte[]  mTempStorage;

    private DrawFrame mDrawFrame = () -> {
        nDrawFrame(mNativeProxy, mSurfaceWidth, mSurfaceHeight, false);
    };

    public SkottieView(Context context, InputStream is) {
        super(context);
        mTempStorage = new byte[16 * 1024];
        mInputStream = is;
        long proxy = SkottieRunner.getInstance().getNativeProxy();
        mNativeProxy = nCreateProxy(proxy, mInputStream,
                mTempStorage);
        mTextureView = new TextureView(context);
        mTextureView.setSurfaceTextureListener(this);
        addView(mTextureView, LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            nDeleteProxy(mNativeProxy);
            mNativeProxy = 0;
        } finally {
            super.finalize();
        }
    }

    class SkottieThread extends Thread {
        public void run() {
            SkottieRunner runner = null;
            try {
                waitForSurface();
                runner = SkottieRunner.getInstance();
                EGLSurface eglSurface = runner.createEGLSurface(mSurface);
                while (mIsRunning) {
                    runner.drawFrame(eglSurface, mDrawFrame);
                }
                runner.destroyEGLSurface(eglSurface);
            }
            catch (Throwable ex) {
                throw new RuntimeException();
            }
        }
    };

    private SkottieThread mThread;

    /**
     * Starts the animation.
     */
    @Override
    public void start() {
        if (!mIsRunning) {
            mIsRunning = true;
            mThread = new SkottieThread();
            mThread.start();
        }
    }

    /**
     * Stops the animation.
     */
    @Override
    public void stop() {
        mIsRunning = false;
        mThread = null;
    }

    @Override
    public boolean isRunning() {
        return mIsRunning;
    }

    private void waitForSurface() throws InterruptedException {
        synchronized (mLock) {
            while (mSurface == null) {
                mLock.wait(TIME_OUT_MS);
            }
        }
    }

    private int waitForSurfaceUpdateCount(int updateCount) throws InterruptedException {
        synchronized (mLock) {
            while (updateCount > mSurfaceUpdatedCount) {
                mLock.wait(TIME_OUT_MS);
            }
            return mSurfaceUpdatedCount;
        }
    }

    /**
     * Ask all children to measure themselves and compute the measurement of this
     * layout based on the children.
     */
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        mTextureView.measure(widthMeasureSpec, heightMeasureSpec);
        int width = mTextureView.getMeasuredWidth();
        int height = mTextureView.getMeasuredHeight();
        setMeasuredDimension(width, height);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        if (changed) { // This is a new size or position for this view
            mTextureView.layout(0, 0, right - left, bottom - top);
        }
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        synchronized (mLock) {
            mSurface = surface;
            mSurfaceWidth = width;
            mSurfaceHeight = height;
            mLock.notifyAll();
        }
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        //TODO
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        synchronized (mLock) {
            mSurface = null;
            mLock.notifyAll();
        }
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {
        synchronized (mLock) {
            mSurfaceUpdatedCount++;
            mLock.notifyAll();
        }
    }

    private static native long nCreateProxy(long runner, InputStream is, byte[] storage);
    private static native void nDeleteProxy(long nativeProxy);
    private static native void nDrawFrame(long nativeProxy, int width, int height, boolean wideColorGamut);
}
