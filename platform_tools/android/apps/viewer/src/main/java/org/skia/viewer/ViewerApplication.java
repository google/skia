/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.viewer;

import android.app.Application;

public class ViewerApplication extends Application {
    private long mNativeHandle = 0;
    private ViewerActivity mViewerActivity;

    static {
        System.loadLibrary("skia_android");
        System.loadLibrary("viewer");
    }

    private native long createNativeApp();
    private native void destroyNativeApp(long handle);

    @Override
    public void onCreate() {
        super.onCreate();
        mNativeHandle = createNativeApp();
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
        this.mViewerActivity = viewerActivity;
    }

    public void setTitle(String title) {
        final String finalTitle = title;
        if (mViewerActivity != null) {
            mViewerActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mViewerActivity.setTitle(finalTitle);
                }
            });
        }
    }
}
