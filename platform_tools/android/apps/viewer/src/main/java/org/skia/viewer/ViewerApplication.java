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
}
