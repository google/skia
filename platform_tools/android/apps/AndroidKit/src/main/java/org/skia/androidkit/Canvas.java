package org.skia.androidkit;

import android.graphics.Paint;
import org.skia.androidkit.Surface;

public class Canvas {
    private Surface mSurface;
    private long mNativeInstance;

    public Canvas(Surface surface) {
        mSurface = surface;
        mNativeInstance = surface.getNativeCanvas();
    }

    public void drawRect(float left, float right, float top, float bottom, Paint paint) {
        nDrawRect(mNativeInstance, left, right, top, bottom, paint);
    }

    private static native void nDrawRect(long nativeInstance,
                                         float left, float right, float top, float bottom, Paint p);
}
