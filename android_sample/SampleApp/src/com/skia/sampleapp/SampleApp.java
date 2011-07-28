/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package com.skia.sampleapp;

import android.app.Activity;
import android.app.DownloadManager;
import android.content.Context;
import android.graphics.Canvas;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import java.io.File;

public class SampleApp extends Activity
{
    private TextView mTitle;
    private SampleView mView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.layout);
        mTitle = (TextView) findViewById(R.id.title_view);
        LinearLayout holder = (LinearLayout) findViewById(R.id.holder);
        mView = new SampleView(this);

        holder.addView(mView, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));

        mTitle.setVisibility(View.GONE);
        getActionBar().setDisplayShowHomeEnabled(false);
    }

    @Override
    protected void onResume () {
        super.onResume();
        int width = mView.getWidth();
        int height = mView.getHeight();
        if (width > 0 && height > 0) {
            mView.postInval();
        }
    }

    @Override
    public void onDestroy() {
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                term();
            }
        });
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.sample, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.overview:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        handleKeyDown(KeyEvent.KEYCODE_BACK, 0);
                    }
                });
                return true;
            case R.id.prev:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        nextSample(true);
                    }
                });
                return true;
            case R.id.next:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        nextSample(false);
                    }
                });
                return true;
            case R.id.toggle_rendering:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        toggleRendering();
                    }
                });
                return true;
            case R.id.slideshow:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        toggleSlideshow();
                    }
                });
                return true;
            case R.id.fps:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        toggleFps();
                    }
                });
                return true;
            case R.id.save_to_pdf:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        saveToPdf();
                    }
                });
                return true;

            default:
                return false;
        }
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        final int keycode = event.getKeyCode();
        if (keycode == KeyEvent.KEYCODE_BACK) {
            if (event.getAction() == KeyEvent.ACTION_UP) {
                finish();
            }
            return true;
        }
        switch (event.getAction()) {
            case KeyEvent.ACTION_DOWN:
                final int uni = event.getUnicodeChar(event.getMetaState());
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        handleKeyDown(keycode, uni);
                    }
                });
                return true;
            case KeyEvent.ACTION_UP:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        handleKeyUp(keycode);
                    }
                });
                return true;
            default:
                return false;
        }
    }

    private static final int SET_TITLE = 1;
    private static final int TOAST_DOWNLOAD = 2;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case SET_TITLE:
                    mTitle.setText((String) msg.obj);
                    SampleApp.this.getActionBar().setSubtitle((String) msg.obj);
                    break;
                case TOAST_DOWNLOAD:
                    Toast.makeText(SampleApp.this, (String) msg.obj,
                            Toast.LENGTH_SHORT).show();
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    public void setTitle(CharSequence title) {
        mHandler.obtainMessage(SET_TITLE, title).sendToTarget();
    }

    // Called by JNI
    @SuppressWarnings("unused")
    private void addToDownloads(final String title, final String desc,
            final String path) {
        File file = new File(path);
        final long length = file.exists() ? file.length() : 0;
        if (length == 0) {
            String failed = getString(R.string.failed);
            mHandler.obtainMessage(TOAST_DOWNLOAD, failed).sendToTarget();
            return;
        }
        String toast = getString(R.string.file_saved).replace("%s", title);
        mHandler.obtainMessage(TOAST_DOWNLOAD, toast).sendToTarget();
        final DownloadManager manager = (DownloadManager) getSystemService(
                Context.DOWNLOAD_SERVICE);
        new Thread("Add pdf to downloads") {
            @Override
            public void run() {
                manager.addCompletedDownload(title, desc, true,
                        "application/pdf", path, length, true);
            }
        }.start();
    }

    // Called by JNI
    @SuppressWarnings("unused")
    private void startTimer(int ms) {
        // After the delay, queue an event to the Renderer's thread
        // to handle the event on the timer queue
        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        serviceQueueTimer();
                    }
                });
            }
        }, ms);
    }

    native void draw();
    native void init();
    native void term();
    // Currently depends on init having already been called.
    native void createOSWindow(SampleView view);
    native void updateSize(int w, int h);
    native void handleClick(int owner, float x, float y, int state);
    native boolean handleKeyDown(int key, int uni);
    native boolean handleKeyUp(int key);
    native void nextSample(boolean previous);
    native void toggleRendering();
    native void toggleSlideshow();
    native void toggleFps();
    native void processSkEvent();
    native void serviceQueueTimer();
    native void saveToPdf();
    native void postInval();

    static {
        System.loadLibrary("skia-sample");
    }
}
