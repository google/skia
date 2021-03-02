package org.skia.skottie;

import android.animation.Animator;
import android.animation.TimeInterpolator;
import android.graphics.SurfaceTexture;
import android.opengl.GLUtils;
import android.util.Log;
import android.view.Choreographer;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;

import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLSurface;

public class SkottieAnimation extends Animator implements Choreographer.FrameCallback,
        TextureView.SurfaceTextureListener, SurfaceHolder.Callback {
    class Config {
        int mSurfaceWidth;
        int mSurfaceHeight;
        boolean mValidSurface;
    }

    private final SkottieRunner mRunner = SkottieRunner.getInstance();
    private static final String LOG_TAG = "SkottiePlayer";

    private boolean mIsRunning = false;
    private SurfaceTexture mSurfaceTexture;
    private EGLSurface mEglSurface;
    private boolean mNewSurface = false;
    private SurfaceHolder mSurfaceHolder;
    private int mRepeatCount;
    private int mRepeatCounter;
    private Config config = new Config();
    private int mBackgroundColor;
    private long mNativeProxy;
    private long mDuration;  // duration in ms of the animation
    private float mProgress; // animation progress in the range of 0.0f to 1.0f
    private long mAnimationStartTime; // time in System.nanoTime units, when started

    SkottieAnimation(SurfaceTexture surfaceTexture, InputStream is) {
        if (init(is)) {
            mSurfaceTexture = surfaceTexture;
        }
    }
    SkottieAnimation(TextureView view, InputStream is, int backgroundColor, int repeatCount) {
        if (init(is)) {
            mSurfaceTexture = view.getSurfaceTexture();
        }
        view.setSurfaceTextureListener(this);
        mBackgroundColor = backgroundColor;
        mRepeatCount = repeatCount;
        mRepeatCounter = mRepeatCount;
    }

    SkottieAnimation(SurfaceView view, InputStream is, int backgroundColor, int repeatCount) {
        if (init(is)) {
            mSurfaceHolder = view.getHolder();
        }
        mSurfaceHolder.addCallback(this);
        mBackgroundColor = backgroundColor;
        mRepeatCount = repeatCount;
        mRepeatCounter = mRepeatCount;
    }

    void setSurfaceTexture(SurfaceTexture s) {
        mSurfaceTexture = s;
    }

    private ByteBuffer convertToByteBuffer(InputStream is) throws IOException {
        if (is instanceof FileInputStream) {
            FileChannel fileChannel = ((FileInputStream)is).getChannel();
            return fileChannel.map(FileChannel.MapMode.READ_ONLY,
                                   fileChannel.position(), fileChannel.size());
        }

        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        byte[] tmpStorage = new byte[4096];
        int bytesRead;
        while ((bytesRead = is.read(tmpStorage, 0, tmpStorage.length)) != -1) {
            byteStream.write(tmpStorage, 0, bytesRead);
        }

        byteStream.flush();
        tmpStorage = byteStream.toByteArray();

        ByteBuffer buffer = ByteBuffer.allocateDirect(tmpStorage.length);
        buffer.order(ByteOrder.nativeOrder());
        buffer.put(tmpStorage, 0, tmpStorage.length);
        return buffer.asReadOnlyBuffer();
    }

    private boolean init(InputStream is) {

        ByteBuffer byteBuffer;
        try {
            byteBuffer = convertToByteBuffer(is);
        } catch (IOException e) {
            Log.e(LOG_TAG, "failed to read input stream", e);
            return false;
        }

        long proxy = mRunner.getNativeProxy();
        mNativeProxy = nCreateProxy(proxy, byteBuffer);
        mDuration = nGetDuration(mNativeProxy);
        mProgress = 0f;
        return true;
    }

    private void notifyAnimationEnd() {
        if (this.getListeners() != null) {
            for (AnimatorListener l : this.getListeners()) {
                l.onAnimationEnd(this);
            }
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            end();
            nDeleteProxy(mNativeProxy);
            mNativeProxy = 0;
        } finally {
            super.finalize();
        }
    }

    // Always call this on GL thread
    public void updateSurface(int width, int height) {
        config.mSurfaceWidth = width;
        config.mSurfaceHeight = height;
        mNewSurface = true;
        drawFrame();
    }

    @Override
    public void start() {
        try {
            mRunner.runOnGLThread(() -> {
                if (!mIsRunning) {
                    long currentTime = System.nanoTime();
                    mAnimationStartTime = currentTime - (long)(1000000 * mDuration * mProgress);
                    mIsRunning = true;
                    mNewSurface = true;
                    mRepeatCounter = mRepeatCount;
                    doFrame(currentTime);
                }
            });
        }
        catch (Throwable t) {
            Log.e(LOG_TAG, "start failed", t);
            throw new RuntimeException(t);
        }
        if (this.getListeners() != null) {
            for (AnimatorListener l : this.getListeners()) {
                l.onAnimationStart(this);
            }
        }
    }

    @Override
    public void end() {
        try {
            mRunner.runOnGLThread(() -> {
                mIsRunning = false;
                if (mEglSurface != null) {
                    // Ensure we always have a valid surface & context.
                    mRunner.mEgl.eglMakeCurrent(mRunner.mEglDisplay, mRunner.mPBufferSurface,
                            mRunner.mPBufferSurface, mRunner.mEglContext);
                    mRunner.mEgl.eglDestroySurface(mRunner.mEglDisplay, mEglSurface);
                    mEglSurface = null;
                }
            });
        }
        catch (Throwable t) {
            Log.e(LOG_TAG, "stop failed", t);
            throw new RuntimeException(t);
        }
        notifyAnimationEnd();
    }

    @Override
    public void pause() {
        try {
            mRunner.runOnGLThread(() -> {
                mIsRunning = false;
            });
        }
        catch (Throwable t) {
            Log.e(LOG_TAG, "pause failed", t);
            throw new RuntimeException(t);
        }
    }

    @Override
    public void resume() {
        try {
            mRunner.runOnGLThread(() -> {
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
            Log.e(LOG_TAG, "resume failed", t);
            throw new RuntimeException(t);
        }
    }

    // TODO: add support for start delay
    @Override
    public long getStartDelay() {
        return 0;
    }

    // TODO: add support for start delay
    @Override
    public void setStartDelay(long startDelay) {

    }

    @Override
    public Animator setDuration(long duration) {
        return null;
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
    public long getTotalDuration() {
        if (mRepeatCount == -1) {
            return DURATION_INFINITE;
        }
        // TODO: add start delay when implemented
        return mDuration * (1 + mRepeatCount);
    }

    // TODO: support TimeInterpolators
    @Override
    public void setInterpolator(TimeInterpolator value) {

    }

    public void setProgress(float progress) {
        try {
            mRunner.runOnGLThread(() -> {
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

    public float getProgress() {
        return mProgress;
    }

    private void drawFrame() {
        try {
            boolean forceDraw = false;
            if (mNewSurface) {
                forceDraw = true;
                // if there is a new SurfaceTexture, we need to recreate the EGL surface.
                if (mEglSurface != null) {
                    mRunner.mEgl.eglDestroySurface(mRunner.mEglDisplay, mEglSurface);
                    mEglSurface = null;
                }
                mNewSurface = false;
            }

            if (mEglSurface == null) {
                // block for Texture Views
                if (mSurfaceTexture != null) {
                    mEglSurface = mRunner.mEgl.eglCreateWindowSurface(mRunner.mEglDisplay,
                            mRunner.mEglConfig, mSurfaceTexture, null);
                    checkSurface();
                // block for Surface Views
                } else if (mSurfaceHolder != null) {
                    mEglSurface = mRunner.mEgl.eglCreateWindowSurface(mRunner.mEglDisplay,
                            mRunner.mEglConfig, mSurfaceHolder, null);
                    checkSurface();
                }
            }

            if (mEglSurface != null) {
                if (!mRunner.mEgl.eglMakeCurrent(mRunner.mEglDisplay, mEglSurface, mEglSurface,
                        mRunner.mEglContext)) {
                    // If eglMakeCurrent failed, recreate EGL surface on next frame.
                    Log.w(LOG_TAG, "eglMakeCurrent failed "
                            + GLUtils.getEGLErrorString(mRunner.mEgl.eglGetError()));
                    mNewSurface = true;
                    return;
                }
                // only if nDrawFrames() returns true do we need to swap buffers
                if(nDrawFrame(mNativeProxy, config.mSurfaceWidth, config.mSurfaceHeight, false,
                        mProgress, mBackgroundColor, forceDraw)) {
                    if (!mRunner.mEgl.eglSwapBuffers(mRunner.mEglDisplay, mEglSurface)) {
                        int error = mRunner.mEgl.eglGetError();
                        if (error == EGL10.EGL_BAD_SURFACE
                            || error == EGL10.EGL_BAD_NATIVE_WINDOW) {
                            // For some reason our surface was destroyed. Recreate EGL surface
                            // on next frame.
                            mNewSurface = true;
                            // This really shouldn't happen, but if it does we can recover
                            // easily by just not trying to use the surface anymore
                            Log.w(LOG_TAG, "swapBuffers failed "
                                + GLUtils.getEGLErrorString(error));
                            return;
                        }

                        // Some other fatal EGL error happened, log an error and stop the
                        // animation.
                        throw new RuntimeException("Cannot swap buffers "
                            + GLUtils.getEGLErrorString(error));
                    }
                }


                // If animation stopped, release EGL surface.
                if (!mIsRunning) {
                    // Ensure we always have a valid surface & context.
                    mRunner.mEgl.eglMakeCurrent(mRunner.mEglDisplay, mRunner.mPBufferSurface,
                            mRunner.mPBufferSurface, mRunner.mEglContext);
                    mRunner.mEgl.eglDestroySurface(mRunner.mEglDisplay, mEglSurface);
                    mEglSurface = null;
                }
            }
        } catch (Throwable t) {
            Log.e(LOG_TAG, "drawFrame failed", t);
            mIsRunning = false;
        }
    }

    private void checkSurface() throws RuntimeException {
        // ensure eglSurface was created
        if (mEglSurface == null || mEglSurface == EGL10.EGL_NO_SURFACE) {
            // If failed to create a surface, log an error and stop the animation
            int error = mRunner.mEgl.eglGetError();
            throw new RuntimeException("createWindowSurface failed "
                + GLUtils.getEGLErrorString(error));
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
            if (timeSinceAnimationStartNS > durationNS) {
                if (mRepeatCounter > 0) {
                    mRepeatCounter--;
                } else if (mRepeatCounter == 0) {
                    mIsRunning = false;
                    mProgress = 1;
                    notifyAnimationEnd();
                }
            }
        }
        if (config.mValidSurface) {
            drawFrame();
        }
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        // will be called on UI thread
        try {
            mRunner.runOnGLThread(() -> {
                mSurfaceTexture = surface;
                updateSurface(width, height);
                config.mValidSurface = true;
            });
        }
        catch (Throwable t) {
            Log.e(LOG_TAG, "updateSurface failed", t);
            throw new RuntimeException(t);
        }
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
        config.mValidSurface = false;
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {

    }

    // Inherited from SurfaceHolder
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        try {
            mRunner.runOnGLThread(() -> {
                mSurfaceHolder = holder;
                updateSurface(width, height);
                config.mValidSurface = true;
            });
        }
        catch (Throwable t) {
            Log.e(LOG_TAG, "updateSurface failed", t);
            throw new RuntimeException(t);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        config.mValidSurface = false;
        surfaceChanged(null, 0, 0, 0);
    }

    Config getBackingViewConfig() {
        return config;
    }

    void setBackingViewConfig(Config config) {
        this.config.mSurfaceHeight = config.mSurfaceHeight;
        this.config.mSurfaceWidth = config.mSurfaceWidth;
        this.config.mValidSurface = config.mValidSurface;
    }

    private native long nCreateProxy(long runner, ByteBuffer data);
    private native void nDeleteProxy(long nativeProxy);
    private native boolean nDrawFrame(long nativeProxy, int width, int height,
                                      boolean wideColorGamut, float progress,
                                      int backgroundColor, boolean forceDraw);
    private native long nGetDuration(long nativeProxy);
}
