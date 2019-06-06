// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkRect;
import org.skia.SkPathDirection;

public class SkPath {
    public native void moveTo(float x, float y);

    public native void lineTo(float x, float y);

    public native void cubicTo(float x0, float y0, float x1, float y1, float x2, float y2);

    private native void nativeInit();

    public SkPath() {
        nativeInit();
    }

    public native void quadTo(float x0, float y0, float x1, float y1);

    public native void conicTo(float x0, float y0, float x1, float y1, float w);

    public native void close();

    public native void addRect(SkRect rect, SkPathDirection dir);

    public native void addOval(SkRect rect, SkPathDirection dir);

    public native SkRect getBounds();

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkRect.referenced();
        SkPathDirection.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkPath(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
