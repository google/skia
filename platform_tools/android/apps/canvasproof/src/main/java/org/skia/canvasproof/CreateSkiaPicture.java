/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/*
AJAR=$ANDROID_SDK_ROOT/platforms/android-19/android.jar
CLASS=CreateSkiaPicture
SRC=platform_tools/android/apps/canvasproof/src/main
javac -classpath $AJAR $SRC/java/org/skia/canvasproof/$CLASS.java
javah -classpath $AJAR:$SRC/java -d $SRC/jni org.skia.canvasproof.$CLASS
*/

package org.skia.canvasproof;

import android.util.Log;
import java.io.IOException;
import java.io.InputStream;
import java.lang.UnsatisfiedLinkError;

public class CreateSkiaPicture {
    private static final String TAG = "CreateSkiaPicture";

    public static void init() {
        try {
            System.loadLibrary("skia_android");
            System.loadLibrary("canvasproof");
        } catch (java.lang.Error e) {
            Log.v(TAG, "System.loadLibrary error", e);
        }
    }

    public static long create(InputStream inputStream) throws IOException {
        byte[] buffer = new byte[16 * (1 << 10)];  // 16 KByte
        long p = 0;
        try {
            p = CreateSkiaPicture.createImpl(inputStream, buffer);
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "UnsatisfiedLinkError createImpl");
        }
        inputStream.close();
        return p;
    }

    public static void delete(long ptr) {
        try {
            if (ptr != 0) {
                CreateSkiaPicture.deleteImpl(ptr);
            }
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "UnsatisfiedLinkError deleteImpl");
        }

    }
    private static native void deleteImpl(long ptr);
    private static native long createImpl(InputStream s, byte[] b);
}
