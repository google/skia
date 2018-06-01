/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.viewer;

import android.app.Application;
import android.content.res.AssetManager;

public class ViewerApplication extends Application {
    private long mNativeHandle = 0;
    private ViewerActivity mViewerActivity;
    private String mStateJsonStr, mTitle;

    static {
        System.loadLibrary("viewer");
    }

    private native long createNativeApp(AssetManager assetManager);
    private native void destroyNativeApp(long handle);

    @Override
    public void onCreate() {
        super.onCreate();
        mNativeHandle = createNativeApp(this.getResources().getAssets());
    }

    @Override
    public void onTerminate() {
        if (mNativeHandle != 0) {
            destroyNativeApp(mNativeHandle);
            mNativeHandle = 0;
        }
        super.onTerminate();
    }

    public long getNativeHandle() {
        return mNativeHandle;
    }

    public void setViewerActivity(ViewerActivity viewerActivity) {
        mViewerActivity = viewerActivity;
        // Note that viewerActivity might be null (called by onDestroy)
        if (mViewerActivity != null) {
            // A new ViewerActivity is created; initialize its state and title
            if (mStateJsonStr != null) {
                mViewerActivity.setState(mStateJsonStr);
            }
            if (mTitle != null) {
                mViewerActivity.setTitle(mTitle);
            }
        }
    }

    public void setTitle(String title) {
        mTitle = title; // Similar to mStateJsonStr, we have to store this.
        if (mTitle.startsWith("Viewer: ")) { // Quick hack to shorten the title
            mTitle = mTitle.replaceFirst("Viewer: ", "");
        }
        if (mViewerActivity != null) {
            mViewerActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mViewerActivity.setTitle(mTitle);
                }
            });
        }
    }

    public void setState(String stateJsonStr) {
        // We have to store this state because ViewerActivity may be destroyed while the native app
        // is still running. When a new ViewerActivity is created, we'll pass the state to it.
        mStateJsonStr = stateJsonStr;
        if (mViewerActivity != null) {
            mViewerActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mViewerActivity.setState(mStateJsonStr);
                }
            });
        }
    }
}
