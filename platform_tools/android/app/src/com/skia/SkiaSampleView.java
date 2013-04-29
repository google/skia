/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package com.skia;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;

public class SkiaSampleView extends GLSurfaceView {

    private final SkiaSampleRenderer mSampleRenderer;

    public SkiaSampleView(Context ctx) {
        super(ctx);

        mSampleRenderer = new SkiaSampleRenderer(this);

        setEGLContextClientVersion(2);
        setEGLConfigChooser(8,8,8,8,0,8);
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
}
