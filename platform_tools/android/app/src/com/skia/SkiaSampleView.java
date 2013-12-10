/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package com.skia;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.EGL14;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.view.MotionEvent;

public class SkiaSampleView extends GLSurfaceView {

    private final SkiaSampleRenderer mSampleRenderer;
    private boolean mRequestedOpenGLAPI; // true == use (desktop) OpenGL. false == use OpenGL ES.
    private int mRequestedMSAASampleCount;

    public SkiaSampleView(Context ctx, boolean useOpenGL, int msaaSampleCount) {
        super(ctx);

        mSampleRenderer = new SkiaSampleRenderer(this);
        mRequestedMSAASampleCount = msaaSampleCount;

        setEGLContextClientVersion(2);
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1) {
            setEGLConfigChooser(8, 8, 8, 8, 0, 8);
        } else {
            mRequestedOpenGLAPI = useOpenGL;
            setEGLConfigChooser(new SampleViewEGLConfigChooser());
        }
        setRenderer(mSampleRenderer);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int count = event.getPointerCount();
        for (int i = 0; i < count; i++) {
            final float x = event.getX(i);
            final float y = event.getY(i);
            final int owner = event.getPointerId(i);
            int action = event.getAction() & MotionEvent.ACTION_MASK;
            switch (action) {
            case MotionEvent.ACTION_POINTER_UP:
                action = MotionEvent.ACTION_UP;
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                action = MotionEvent.ACTION_DOWN;
                break;
            default:
                break;
            }
            final int finalAction = action;
            queueEvent(new Runnable() {
                @Override
                public void run() {
                    mSampleRenderer.handleClick(owner, x, y, finalAction);
                }
            });
        }
        return true;
    }

    public void inval() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.postInval();
            }
        });
    }

    public void terminate() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.term();
            }
        });
    }

    public void showOverview() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.showOverview();
            }
        });
    }

    public void nextSample() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.nextSample();
            }
        });
    }

    public void previousSample() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.previousSample();
            }
        });
    }

    public void goToSample(final int position) {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.goToSample(position);
            }
        });
    }

    public void toggleRenderingMode() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.toggleRenderingMode();
            }
        });
    }

    public void toggleSlideshow() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.toggleSlideshow();
            }
        });
    }

    public void toggleFPS() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.toggleFPS();
            }
        });
    }

    public void toggleTiling() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.toggleTiling();
            }
        });
    }

    public void toggleBBox() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.toggleBBox();
            }
        });
    }
    
    public void saveToPDF() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mSampleRenderer.saveToPDF();
            }
        });
    }

    public boolean getUsesOpenGLAPI() {
        return mRequestedOpenGLAPI;
    }

    public int getMSAASampleCount() {
        return mSampleRenderer.getMSAASampleCount();
    }

    private class SampleViewEGLConfigChooser implements GLSurfaceView.EGLConfigChooser {
        private int[] mValue;

        @Override
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
            mValue = new int[1];

            int glAPIToTry;

            if (mRequestedOpenGLAPI) {
                glAPIToTry = EGL14.EGL_OPENGL_API;
            } else {
                glAPIToTry = EGL14.EGL_OPENGL_ES_API;
            }

            int numConfigs = 0;
            int[] configSpec = null;

            do {
                EGL14.eglBindAPI(glAPIToTry);

                int renderableType;
                if (glAPIToTry == EGL14.EGL_OPENGL_API) {
                    renderableType = EGL14.EGL_OPENGL_ES2_BIT;

                    // If this API does not work, try ES next.
                    glAPIToTry = EGL14.EGL_OPENGL_ES_API;
                } else {
                    renderableType = EGL14.EGL_OPENGL_BIT;
                }


                if (mRequestedMSAASampleCount > 0) {
                    configSpec = new int[] {
                        EGL10.EGL_RED_SIZE, 8,
                        EGL10.EGL_GREEN_SIZE, 8,
                        EGL10.EGL_BLUE_SIZE, 8,
                        EGL10.EGL_ALPHA_SIZE, 8,
                        EGL10.EGL_DEPTH_SIZE, 0,
                        EGL10.EGL_STENCIL_SIZE, 8,
                        EGL10.EGL_RENDERABLE_TYPE, renderableType,
                        EGL10.EGL_SAMPLE_BUFFERS, 1,
                        EGL10.EGL_SAMPLES, mRequestedMSAASampleCount,
                        EGL10.EGL_NONE
                    };

                    if (!egl.eglChooseConfig(display, configSpec, null, 0, mValue)) {
                        throw new IllegalArgumentException("Could not get MSAA context count");
                    }

                    numConfigs = mValue[0];
                }

                if (numConfigs <= 0) {
                    // Try without multisampling.
                    configSpec = new int[] {
                        EGL10.EGL_RED_SIZE, 8,
                        EGL10.EGL_GREEN_SIZE, 8,
                        EGL10.EGL_BLUE_SIZE, 8,
                        EGL10.EGL_ALPHA_SIZE, 8,
                        EGL10.EGL_DEPTH_SIZE, 0,
                        EGL10.EGL_STENCIL_SIZE, 8,
                        EGL10.EGL_RENDERABLE_TYPE, renderableType,
                        EGL10.EGL_NONE
                    };

                    if (!egl.eglChooseConfig(display, configSpec, null, 0, mValue)) {
                        throw new IllegalArgumentException("Could not get non-MSAA context count");
                    }
                    numConfigs = mValue[0];
                }

            } while (glAPIToTry != EGL14.EGL_OPENGL_ES_API && numConfigs == 0);

            if (numConfigs <= 0) {
                throw new IllegalArgumentException("No configs match configSpec");
            }

            // Get all matching configurations.
            EGLConfig[] configs = new EGLConfig[numConfigs];
            if (!egl.eglChooseConfig(display, configSpec, configs, numConfigs, mValue)) {
                throw new IllegalArgumentException("Could not get config data");
            }

            for (int i = 0; i < configs.length; ++i) {
                EGLConfig config = configs[i];
                if (findConfigAttrib(egl, display, config , EGL10.EGL_RED_SIZE, 0) == 8 &&
                        findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE, 0) == 8 &&
                        findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE, 0) == 8 &&
                        findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE, 0) == 8 &&
                        findConfigAttrib(egl, display, config, EGL10.EGL_STENCIL_SIZE, 0) == 8) {
                    return config;
                }
            }

            throw new IllegalArgumentException("Could not find suitable EGL config");
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display,
                EGLConfig config, int attribute, int defaultValue) {
            if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
                return mValue[0];
            }
            return defaultValue;
        }

    }
}
