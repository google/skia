package org.skia.androidkit;

import android.graphics.Paint;
import org.skia.androidkit.Surface;

public class Canvas {
    private long mNativeInstance;
    private Surface mSurface;

    public void drawRect(float left, float right, float top, float bottom, Paint paint) {
        nDrawRect(mNativeInstance, left, right, top, bottom, paint);
    }

    // package private
    Canvas(Surface surface, long native_instance) {
        mNativeInstance = native_instance;
        mSurface = surface;
    }

    private static native void nDrawRect(long nativeInstance,
                                         float left, float right, float top, float bottom, Paint p);
}
