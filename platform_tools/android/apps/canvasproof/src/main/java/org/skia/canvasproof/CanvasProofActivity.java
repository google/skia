/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.canvasproof;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Picture;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.LinearLayout.LayoutParams;
import android.widget.LinearLayout;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;

public class CanvasProofActivity extends Activity {
    private static final String TAG = "CanvasProofActivity";
    private GaneshPictureRenderer ganeshPictureRenderer;
    private HwuiPictureView hwuiPictureView;
    private GLSurfaceView ganeshPictureView;
    private LinearLayout splitPaneView;
    private View currentView;
    private float x, y;
    private int resourcesIndex;
    private class PictureAsset {
        public String path;
        public long ptr;
        public Picture picture;
    };
    private PictureAsset[] assets;

    @SuppressWarnings("deprecation")  // purposely using this
    private static Picture ReadPicture(InputStream inputStream)
        throws IOException {
        Picture p = null;
        try {
            p = Picture.createFromStream(inputStream);
        } catch (java.lang.Exception e) {
            Log.e(TAG, "Exception in Picture.createFromStream", e);
        }
        inputStream.close();
        return p;
    }

    private void getAssetPaths() {
        String directory = "skps";
        AssetManager mgr = this.getAssets();
        assert (mgr != null);
        String[] resources;
        try {
            resources = mgr.list(directory);
        } catch (IOException e) {
            Log.e(TAG, "IOException in getAssetPaths", e);
            return;
        }
        if (resources == null || resources.length == 0) {
            Log.e(TAG, "SKP assets should be packaged in " +
                  ".../apps/canvasproof/src/main/assets/skps/" +
                  ", but none were found.");
            return;
        }
        CreateSkiaPicture.init();
        this.assets = new PictureAsset[resources.length];
        for (int i = 0; i < resources.length; ++i) {
            String path = directory + File.separator + resources[i];
            this.assets[i] = new PictureAsset();
            this.assets[i].path = path;
            try {
                this.assets[i].ptr = CreateSkiaPicture.create(mgr.open(path));
                if (0 == this.assets[i].ptr) {
                    Log.e(TAG, "CreateSkiaPicture.create returned 0 " + path);
                }
                Picture p = CanvasProofActivity.ReadPicture(mgr.open(path));
                if (null == p) {
                    Log.e(TAG, "CanvasProofActivity.ReadPicture.create " +
                          "returned null " + path);
                } else if (0 == p.getHeight() || 0 == p.getWidth()) {
                    Log.e(TAG, "CanvasProofActivity.ReadPicture.create " +
                          "empty picture" + path);
                    p = null;
                }
                this.assets[i].picture = p;
            } catch (IOException e) {
                Log.e(TAG, "IOException in getAssetPaths " + path + e);
                return;
            }
        }
    }

    private void nextView() {
        if (this.currentView == this.hwuiPictureView) {
            this.currentView = this.ganeshPictureView;
            this.ganeshPictureView.setRenderMode(
                    GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        } else if (this.currentView == null ||
                   this.currentView == this.ganeshPictureView) {
            this.setContentView(new View(this));
            LayoutParams layoutParams =
                new LayoutParams(
                        LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT, 0.5f);
            this.ganeshPictureView.setLayoutParams(layoutParams);
            this.ganeshPictureView.setRenderMode(
                    GLSurfaceView.RENDERMODE_WHEN_DIRTY);
            this.splitPaneView.addView(this.ganeshPictureView);
            this.hwuiPictureView.setLayoutParams(layoutParams);
            this.splitPaneView.addView(this.hwuiPictureView);
            this.currentView = this.splitPaneView;
            this.hwuiPictureView.fullTime = false;
        } else if (this.currentView == this.splitPaneView) {
            this.splitPaneView.removeAllViews();
            this.currentView = this.hwuiPictureView;
            this.hwuiPictureView.fullTime = true;
        } else {
            Log.e(TAG, "unexpected value");
            this.setContentView(null);
            return;
        }
        this.setContentView(this.currentView);
        this.currentView.invalidate();
    }

    private void nextPicture(int d) {
        if (this.assets == null) {
            Log.w(TAG, "this.assets == null");
            return;
        }
        assert (this.assets.length > 0);
        resourcesIndex = (resourcesIndex + d) % this.assets.length;
        while (resourcesIndex < 0) {
            resourcesIndex += this.assets.length;
        }
        while (resourcesIndex >= this.assets.length) {
            resourcesIndex -= this.assets.length;
        }
        this.ganeshPictureRenderer.setPicture(assets[resourcesIndex].ptr);
        this.hwuiPictureView.setPicture(assets[resourcesIndex].picture);
        this.currentView.invalidate();
    }

    @Override
    protected void onStop() {
        this.ganeshPictureRenderer.releaseResources();
        super.onStop();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        this.getAssetPaths();
        this.ganeshPictureRenderer = new GaneshPictureRenderer();
        this.hwuiPictureView = new HwuiPictureView(this);

        this.ganeshPictureRenderer.setScale(2.0f);
        this.hwuiPictureView.setScale(2.0f);
        this.ganeshPictureView = ganeshPictureRenderer.makeView(this);
        this.splitPaneView = new LinearLayout(this);
        this.splitPaneView.setOrientation(LinearLayout.VERTICAL);
        this.splitPaneView.setGravity(Gravity.FILL);

        LayoutParams layoutParams =
            new LayoutParams(
                    LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT, 0.5f);
        this.ganeshPictureView.setLayoutParams(layoutParams);
        this.hwuiPictureView.setLayoutParams(layoutParams);

        this.nextView();
        this.nextPicture(0);
    }

    // TODO: replace this funtion with onTouchEvent().
    // @Override public boolean onTouchEvent(MotionEvent event)...
    @Override
    public boolean dispatchTouchEvent (MotionEvent event) {
        switch(event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                this.x = event.getX();
                this.y = event.getY();
                break;
            case MotionEvent.ACTION_UP:
                float dx = event.getX() - this.x;
                float dy = event.getY() - this.y;
                float dx2 = dx * dx;
                float dy2 = dy * dy;
                if (dx2 + dy2 > 22500.0) {
                    if (dy2 < dx2) {
                        this.nextPicture(dx > 0 ? 1 : -1);
                    } else if (dy > 0) {
                        this.nextView();
                    }
                }
                break;
        }
        return super.onTouchEvent(event);
    }
}
